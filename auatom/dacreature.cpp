//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dacreature.cpp,v 1.1134 2022/05/19 15:08:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dacreature.h"
//
#include "auanimalia.h"
#include "auatomrenderinfo.h"
#include "aubox.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "audna.h"
#include "aueffect.h"
#include "augleffects.h"
#include "auglmodelmaker.h"
#include "auglobal.h"
#include "ausound.h"
#include "ausynccontroller.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesignbase.h"
#include "dadesigner.h"
#include "daitem.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "davehicle.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dpconsoleengineer.h"
#include "dpconsolegame.h"
#include "dpconsolehelm.h"
#include "dpconsolenav.h"
#include "dpconsolepower.h"
#include "dpconsolesensor.h"
#include "dpconsoleshield.h"
#include "dpconsolesurgery.h"
#include "dpconsoletransporter.h"
#include "dpconsoleweapon.h"
#include "dpsccommand.h"
#include "dpscengineer.h"
#include "dpschelm.h"
#include "dpscnav.h"
#include "dpscpower.h"
#include "dpscsensor.h"
#include "dpscshield.h"
#include "dpscsurgery.h"
#include "dpsctransporter.h"
#include "dpscweapon.h"

#define c_OriginAtHead					0						// 1=Body origin at head position, 0=Body origin at body pivot point
#define c_GunMaxZ	-0.0222989		// 3ds model max extent of the CID_HeavyWeapon model.
#define c_GunMinZ	-1.3777			// 3ds model min extent of the CID_HeavyWeapon model.

// Avatar
DaCreature::DaCreature(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, quint32 idEmpire,
		GROWTHSTAGE stage) : DaUnit(name, idEmpire)
{
	try
	{
		const AuDNA& dna4 = dynamic_cast<const AuDNA&>(dna);
		m_Animalia = new AuAnimalia(dna4, chroma, hand, stage);
		m_Body = 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		try
		{
			const DhDNA& dna3 = dynamic_cast<const DhDNA&>(dna);
			m_Animalia = 0;
			m_Body = new DpBody(chroma == AuDNABase::Alpha ? Male : Female, hand == LeftHanded, dna3);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			m_Animalia = 0;
			m_Body = 0;
			qWarning() << "warning: DaCreature::DaCreature bad avatar dna";
		}
	}
	m_AirInLungs = 3;
	m_BodyQ = 128;
	m_Console = 0;
	m_Haircut = AuHairStyle::Unkempt;
	m_Shave = AuHairStyle::Unshaven;
	m_Hunger = 0;
	m_ItemInHand = 0;
	m_Ordnance = 0;
	m_RenderedHeadTurn = 0;
	m_SeatIndex = 0;
	m_MinutesToFullGrown = 0;
	m_MouthAnimDur = 0;
	m_MouthFinish = AuVisage(AuVisage::Invalid);
	m_MouthStationary = AuPose::MouthClosed;
	m_Stamina = 0;
	m_StationId = 0;
	m_ControlId = 0;
	for(int cs=0; cs<CREATURE_SOUNDS; cs++)
	{
		m_AudioCreature[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}
}

// Citizen
DaCreature::DaCreature(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, GROWTHSTAGE stage) :
	DaUnit(name, 0)
{
	try
	{
		const AuDNA& dna4 = dynamic_cast<const AuDNA&>(dna);
		m_Animalia = new AuAnimalia(dna4, chroma, hand, stage);
		m_Body = 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		try
		{
			const DhDNA& dna3 = dynamic_cast<const DhDNA&>(dna);
			m_Animalia = 0;
			m_Body = new DpBody(chroma == AuDNABase::Alpha ? Male : Female, hand == LeftHanded, dna3);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			m_Animalia = 0;
			m_Body = 0;
			qWarning() << "warning: DaCreature::DaCreature bad citizen dna";
		}
	}
	m_AirInLungs = 3;
	m_BodyQ = 128;
	m_Console = 0;
	m_Haircut = AuHairStyle::Unkempt;
	m_Shave = AuHairStyle::Unshaven;
	m_Hunger = 0;
	m_ItemInHand = 0;
	m_Ordnance = 0;
	m_RenderedHeadTurn = 0;
	m_SeatIndex = 0;
	m_MinutesToFullGrown = 0;
	m_MouthAnimDur = 0;
	m_MouthFinish = AuVisage(AuVisage::Invalid);
	m_MouthStationary = AuPose::MouthClosed;
	m_Stamina = 0;
	m_StationId = 0;
	m_ControlId = 0;
	for(int cs=0; cs<CREATURE_SOUNDS; cs++)
	{
		m_AudioCreature[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}
}

DaCreature::DaCreature(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
		GROWTHSTAGE stage) : DaUnit(name, 0)
{
	AuDNABase* dna = species.CreateDNA();
	if(dna)
	{
		const AuDNA* dna4 = dynamic_cast<const AuDNA*>(dna);
		if(dna4)
		{
			m_Animalia = new AuAnimalia(*dna4, chroma, hand, stage);
			m_Body = 0;
		}
		else
		{
			const DhDNA* dna3 = dynamic_cast<const DhDNA*>(dna);
			if(dna3)
			{
				m_Animalia = 0;
				m_Body = new DpBody((chroma == AuDNABase::Alpha) ? Male : Female, hand == LeftHanded, *dna3);
			}
			else
			{
				m_Animalia = 0;
				m_Body = 0;
			}
		}
		delete dna;
	}
	else
	{
		qWarning() << "warning: DaCreature::DaCreature bad species" << (species.IsNull() ? "is null" : "is not null")
			<< "name" << name;
		m_Animalia = 0;
		m_Body = 0;
	}
	m_AirInLungs = 3;
	m_BodyQ = 128;
	m_Console = 0;
	m_Haircut = AuHairStyle::Unkempt;
	m_Shave = AuHairStyle::Unshaven;
	m_Hunger = 0;
	m_ItemInHand = 0;
	m_Ordnance = 0;
	m_RenderedHeadTurn = 0;
	m_SeatIndex = 0;
	m_MinutesToFullGrown = 0;
	m_MouthAnimDur = 0;
	m_MouthFinish = AuVisage(AuVisage::Invalid);
	m_MouthStationary = AuPose::MouthClosed;
	m_Stamina = 0;
	m_StationId = 0;
	m_ControlId = 0;
	for(int cs=0; cs<CREATURE_SOUNDS; cs++)
	{
		m_AudioCreature[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}
}

DaCreature::DaCreature(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, GROWTHSTAGE stage)
{
	AuDNABase* dna = species.CreateDNA();
	if(dna)
	{
		const AuDNA* dna4 = dynamic_cast<const AuDNA*>(dna);
		if(dna4)
		{
			m_Animalia = new AuAnimalia(*dna4, chroma, hand, stage);
			m_Body = 0;
		}
		else
		{
			const DhDNA* dna3 = dynamic_cast<const DhDNA*>(dna);
			if(dna3)
			{
				m_Animalia = 0;
				m_Body = new DpBody((chroma == AuDNABase::Alpha) ? Male : Female, hand == LeftHanded, *dna3);
			}
			else
			{
				m_Animalia = 0;
				m_Body = 0;
			}
		}
		delete dna;
	}
	else
	{
		qWarning() << "warning: DaCreature::DaCreature bad species" << (species.IsNull() ? "is null" : "is not null");
		m_Animalia = 0;
		m_Body = 0;
	}
	m_AirInLungs = 3;
	m_BodyQ = 128;
	m_Console = 0;
	m_Haircut = AuHairStyle::Unkempt;
	m_Shave = AuHairStyle::Unshaven;
	m_Hunger = 0;
	m_ItemInHand = 0;
	m_Ordnance = 0;
	m_RenderedHeadTurn = 0;
	m_SeatIndex = 0;
	m_MinutesToFullGrown = 0;
	m_MouthAnimDur = 0;
	m_MouthFinish = AuVisage(AuVisage::Invalid);
	m_MouthStationary = AuPose::MouthClosed;
	m_Stamina = 0;
	m_StationId = 0;
	m_ControlId = 0;
	for(int cs=0; cs<CREATURE_SOUNDS; cs++)
	{
		m_AudioCreature[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}
}

DaCreature::DaCreature()
{
	m_Animalia = 0;
	m_Body = new DpBody;
	m_AirInLungs = 3;
	m_BodyQ = 128;
	m_Console = 0;
	m_Haircut = AuHairStyle::Unkempt;
	m_Shave = AuHairStyle::Unshaven;
	m_Hunger = 0;
	m_ItemInHand = 0;
	m_Ordnance = 0;
	m_RenderedHeadTurn = 0;
	m_SeatIndex = 0;
	m_MinutesToFullGrown = 0;
	m_MouthAnimDur = 0;
	m_MouthFinish = AuVisage(AuVisage::Invalid);
	m_MouthStationary = AuPose::MouthClosed;
	m_Stamina = 0;
	m_StationId = 0;
	m_ControlId = 0;
	for(int cs=0; cs<CREATURE_SOUNDS; cs++)
	{
		m_AudioCreature[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}
}

DaCreature::~DaCreature()
{
	if(m_Console)
		delete m_Console;
	if(m_Ordnance)
		delete m_Ordnance;
	if(m_ItemInHand)
		delete m_ItemInHand;
	if(m_Body)
		delete m_Body;
	if(m_Animalia)
		delete m_Animalia;
}

DaCreature::DaCreature(const DaCreature &rhs) : DaUnit(rhs)
{
	rhs.m_BodyLock.lockForRead();
	if(rhs.m_Body)
		m_Body = new DpBody(*rhs.m_Body);
	else
		m_Body = 0;
	if(rhs.m_Animalia)
		m_Animalia = new AuAnimalia(*rhs.m_Animalia);
	else
		m_Animalia = 0;
	m_Haircut = rhs.m_Haircut;
	m_Shave = rhs.m_Shave;
	m_StationId = rhs.m_StationId;
	m_tmBodyChange = rhs.m_tmBodyChange;
	m_tmHeadChange = rhs.m_tmHeadChange;
	m_tmHPChange = rhs.m_tmHPChange;
	m_MouthAnimDur = rhs.m_MouthAnimDur;
	m_MouthAnimStart = rhs.m_MouthAnimStart;
	m_MouthFinish = rhs.m_MouthFinish;
	m_MouthStationary = rhs.m_MouthStationary;
	m_tmHairChange = rhs.m_tmHairChange;
	m_tmPoseChange = rhs.m_tmPoseChange;
	rhs.m_BodyLock.unlock();

	rhs.m_ConsoleLock.lockForRead();
	if(rhs.m_Console)
		m_Console = rhs.m_Console->Clone();
	else
		m_Console = 0;
	rhs.m_ConsoleLock.unlock();

	rhs.m_HungerLock.lockForRead();
	m_Hunger = rhs.m_Hunger;
	m_tmHungerChange = rhs.m_tmHungerChange;
	rhs.m_HungerLock.unlock();

	m_ItemInHand = 0;

	if(rhs.m_Ordnance)
		m_Ordnance = rhs.m_Ordnance->Clone();
	else
		m_Ordnance = 0;

	m_AirInLungs = rhs.m_AirInLungs;
	m_BodyQ = rhs.m_BodyQ;
	m_MinutesToFullGrown = rhs.m_MinutesToFullGrown;
	m_SeatIndex = rhs.m_SeatIndex;
	m_Stamina = rhs.m_Stamina;
	m_ControlId = rhs.m_ControlId;

	m_RenderedHeadTurn = 0;
	//DHH I'm not sure if it's ok to duplicate these sources like this.
	for(int cs=0; cs<CREATURE_SOUNDS; cs++)
	{
		m_AudioCreature[cs] = rhs.m_AudioCreature[cs];
		m_Sound[cs] = rhs.m_Sound[cs];
	}
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaCreature::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, 
		const DaPerson& sceneAtom)
{
	DaUnit::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
	quint16 hasWhat = 0;
	if(tmLastUpdate.IsNull())
	{
		hasWhat |= 0x2f;	// everything
	}
	else
	{
		m_BodyLock.lockForRead();
		if(m_tmBodyChange >= lastTx)
			hasWhat |= 0x01;
		if(m_tmHeadChange >= lastTx)
			hasWhat |= 0x02;
		if(m_tmHPChange >= lastTx)
			hasWhat |= 0x04;
		if(m_tmPoseChange >= lastTx)
			hasWhat |= 0x08;
		m_BodyLock.unlock();

		if(m_MinutesToFullGrown)
			hasWhat |= 0x20;
	}
	if(m_Animalia)
		hasWhat |= 0x0100;	// Signal that we are using new style DNA.
	if(lastTx.IsNull() || m_tmHairChange >= lastTx)
		hasWhat |= 0x0200;

	// Some data is only sent to the scene atom.
	if(sceneAtom.Id() == Id())
	{
		if(HasConsole() && IsConsolePose())
			hasWhat |= 0x10;

		// Readiness of special weapon.
		bool r;
		m_BodyLock.lockForRead();
		if(hasWhat & 0x0100)
			r = m_Animalia->SpecialWeaponReady();	// Global UTC is close enough for this test. ms accuracy not needed.
		else
			r = m_Body->SpecialWeaponReady();	// Global UTC is close enough for this test. ms accuracy not needed.
		m_BodyLock.unlock();
		if(r)
			hasWhat |= 0x40;

		if(IsTurretPose() || (IsFireControlStationPose() && HasConsole()))
			hasWhat |= 0x80;
	}

	toClient << hasWhat;

	if(hasWhat & 0x01)
	{
		quint8 bits = Chromatid();
		if(IsLeftHanded())
			bits |= 0x04;
		toClient << bits;
		m_BodyLock.lockForRead();
		if(hasWhat & 0x0100)
			toClient << m_Animalia->DNA();
		else
			toClient << m_Body->DNA();
		m_BodyLock.unlock();
	}

	if(hasWhat & 0x02)
		toClient << CurrentHeadTilt();

	if(hasWhat & 0x04)
		toClient << CurrentBodyHitPoints();

	if(hasWhat & 0x08)
	{
		m_BodyLock.lockForRead();
		if(hasWhat & 0x0100)
		{
			const AuPose* curposes = m_Animalia->CurrentPoses();
			for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
				toClient << curposes[ps];
		}
		else
		{
			const AuPose* curposes = m_Body->CurrentPoses();
			for(int ps=0; ps<DpBody::PS_Count; ps++)
				toClient << curposes[ps];
		}
		toClient << m_StationId;
		m_BodyLock.unlock();
	}

	if(hasWhat & 0x10)
	{
		m_ConsoleLock.lockForRead();
		if(m_Console)
		{
			toClient << (quint8)m_Console->Type();
			m_Console->Tx(toClient);		// Console sends the subsystem which identifier when applicable.
		}
		else
			toClient << (quint8)0;
		m_ConsoleLock.unlock();
	}

	toClient << m_Stamina;

	if(hasWhat & 0x20)
		toClient << m_MinutesToFullGrown;

	if(hasWhat & 0x80)
	{
		// Send the weapon's current ammo load to the client.
		const AuItem* ammo = 0;
		if(IsFireControlStationPose())
		{
			m_ConsoleLock.lockForRead();
			if(m_Console && AuStation::IsFireControl(m_Console->Type()))
			{
				ammo = ((DpStationWeapon*)m_Console)->LockOrdnance();
				if(ammo)
				{
					toClient << ammo->Type();
					toClient << *ammo;
				}
				else
					toClient << (quint8)0;
				((DpStationWeapon*)m_Console)->UnlockOrdnance();
			}
			else
				toClient << (quint8)0;
			m_ConsoleLock.unlock();
		}
		else
		{
			DaCapsule* cap = dynamic_cast<DaCapsule*>(Parent());
			if(cap)
				ammo = cap->TurretAmmunitionClone(StationId());

			if(ammo)
			{
				toClient << ammo->Type();
				toClient << *ammo;

				// Cleanup ammo clone.
				delete ammo;
			}
			else
				toClient << (quint8)0;

		}
	}

	if(hasWhat & 0x0200)
	{
		// Clothing, haircut and shave.
		toClient << (quint8)m_Haircut;
		toClient << (quint8)m_Shave;
	}
}

// Receive updates to this atom from a server.
void DaCreature::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaUnit::RxUpdate(fromServer, isFirstUpdate);
	quint16 hasWhat;
	fromServer >> hasWhat;

	// Insure we have the proper kind of body.
	if(hasWhat & 0x0100)
	{
		if(!m_Animalia)
			m_Animalia = new AuAnimalia;
		if(m_Body)
		{
			delete m_Body;
			m_Body = 0;
		}
	}
	else
	{
		if(!m_Body)
			m_Body = new DpBody;
		if(m_Animalia)
		{
			delete m_Animalia;
			m_Animalia = 0;
		}
	}

	if(hasWhat & 0x01)
	{
		quint8 bits;
		fromServer >> bits;
		AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)(bits & 0x01);
		if(hasWhat & 0x0100)
		{
			AuDNA dna;
			fromServer >> dna;
			SetDNA(dna);
			m_Animalia->SetDNA(dna, chroma);
			m_Animalia->SetLeftHanded((bits&0x04) ? true : false);
		}
		else
		{
			m_Body->SetSex((chroma == AuDNABase::Alpha) ? Male : Female);
			m_Body->SetLeftHanded((bits&0x04) ? true : false);

			DhDNA dna;
			fromServer >> dna;
			SetDNA(dna);
		}
	}

	if(hasWhat & 0x02)
	{
		float ht;
		fromServer >> ht;
		//qDebug("DaCreature::RxUpdate ht %g", ht*RAD2DEGf);
		TiltHeadTo(ht);
	}

	if(hasWhat & 0x04)
	{
		short int hitpoints;
		fromServer >> hitpoints;
		//qDebug("DaCreature::RxUpdate hp %d", hitpoints);
		if(hasWhat & 0x0100)
			m_Animalia->SetHitPoints(hitpoints);
		else
			m_Body->SetHitPoints(hitpoints);
	}

	AuStation::TYPE oldcontype = AuStation::TYPE_Count;
	if(m_Console && IsConsolePose())
		oldcontype = m_Console->Type();

	if(hasWhat & 0x08)
	{
		if(hasWhat & 0x0100)
		{
			AuPose poses[AuAnimalPart::PS_Count];
			AuPose pose;
			for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
			{
				fromServer >> pose;
				//if(ps == AuAnimalPart::PS_Mouth)
					//qDebug() << "DaCreature::RxUpdate" << *this << "pose for animalia segment" << ps << "is" << pose;
				poses[ps] = pose;
			}
			quint16 stationId;
			fromServer >> stationId;
			SetPoses4(poses, stationId, stationId);	// This could change my console type. Console may change subsytem control id.
		}
		else
		{
			AuPose poses[DpBody::PS_Count];
			AuPose pose;
			for(int ps=0; ps<DpBody::PS_Count; ps++)
			{
				fromServer >> pose;
				//qDebug() << "DaCreature::RxUpdate" << *this << "pose for body segment" << ps << "is" << pose;
				poses[ps] = pose;
			}
			quint16 stationId;
			fromServer >> stationId;
			SetPoses3(poses, stationId, stationId);	// This could change my console type. Console may change subsytem control id.
		}
		//if(Id() == AuId::Haxus())
			//qDebug() << "DaCreature::RxUpdate" << Id() << Name() << "pose" << CurrentGeneralPose();

		//DHH Pose Testing
		//m_Body.Pose(AuPose::DeadFaceRight);
		//m_Body.Pose(AuPose::WritheFaceRight, HasItemInHand());
	}

	if(hasWhat & 0x10)
	{
		quint8 consoleType;
		fromServer >> consoleType;
		bool readConsole = false;
		if(m_Console && IsConsolePose() && m_Console->Type() == consoleType)
		{
			// Receive console data for the appropriate kind of console.
			DaCapsule* capsule = FindParentCapsuleInEnv();
			if(capsule)
			{
				m_Console->Rx(fromServer, *capsule);
				readConsole = true;
			}

			// Get my console subsystem which identifier from the console.
			m_ControlId = m_Console->SystemWhich();

			// If my console is nu or not the same as before, load this person's last settings and send them to the server.
			if(m_Console->Type() != oldcontype)
			{
				DpStation::STATION_TYPE stationType = m_Console->ClientRestoreType();
				if(stationType != DpStation::STATION_Count)
				{
					// This console knows how to restore its settings.
					LoadClientConsoleSettings();

					// Emit a signal so the client application can send a packet to the server.
					QByteArray ba;
					QDataStream out(&ba, QIODevice::WriteOnly);
					out.setVersion(QDataStream::Qt_4_1);
					m_Console->TxSettings(out);
					emit RestoreConsoleSettings((quint8)stationType, ba);
				}
			}
		}

		if(!readConsole)
		{
			// We must read the bad console data to keep the data stream intact.
			DaCapsule* ship = FindParentCapsuleInEnv();
			if(ship)
			{
				DpStation* console;
				switch((AuStation::TYPE)consoleType)
				{
				default:
					qDebug() << "DaCreature::RxUpdate bad console" << (m_Console?"at console":"not at console")
						<< "isConPose" << IsConsolePose()
						<< "conType" << m_Console->Type()
						<< "rxType" << consoleType;
					console = 0;
					break;
				case AuStation::Integrated_Command:
					console = new DpScCommand();
					break;
				case AuStation::Integrated_Engine:
					console = new DpScEngineer(m_ControlId);
					break;
				case AuStation::Integrated_FireControl:
					console = new DpScWeapon(m_ControlId);
					break;
				case AuStation::Integrated_Helm:
					console = new DpScHelm();
					break;
				case AuStation::Integrated_Medic:
					console = new DpScSurgery(m_ControlId);
					break;
				case AuStation::Integrated_Navigation:
					console = new DpScNav();
					break;
				case AuStation::Integrated_Patient:
					console = 0;
					break;
				case AuStation::Integrated_Power:
					console = new DpScPower();
					break;
				case AuStation::Integrated_Sensor:
					console = new DpScSensor();
					break;
				case AuStation::Integrated_Shield:
					console = new DpScShield();
					break;
				case AuStation::Integrated_Transporter:
					console = new DpScTransporter(m_ControlId);
					break;
				case AuStation::Console_Engine:
					console = new DpConsoleEngineer(m_ControlId);
					break;
				case AuStation::Console_FireControl:
					console = new DpConsoleWeapon(m_ControlId);
					break;
				case AuStation::Table_Game:
				case AuStation::Table_Game_0:
				case AuStation::Table_Game_45:
					console = new DpConsoleGame((AuStation::TYPE)consoleType, m_ControlId);
					break;
				case AuStation::Console_Navigation:
					console = new DpConsoleNav();
					break;
				case AuStation::Console_Helm:
					console = new DpConsoleHelm();
					break;
				case AuStation::Console_Power:
					console = new DpConsolePower();
					break;
				case AuStation::Console_Sensor:
					console = new DpConsoleSensor();
					break;
				case AuStation::Console_Shield:
					console = new DpConsoleShield();
					break;
				case AuStation::Console_Medic:
					console = new DpConsoleSurgery(m_ControlId);
					break;
				case AuStation::Console_Transporter:
					console = new DpConsoleTransporter(m_ControlId);
					break;
				}
				if(console)
				{
					console->Rx(fromServer, *ship);
					delete console;
				}
			}
		}
	}
	else if(m_Console && !IsConsolePose())
	{
		delete m_Console;
		m_Console = 0;
	}

	fromServer >> m_Stamina;

	if(hasWhat & 0x20)
		fromServer >> m_MinutesToFullGrown;
	else
		m_MinutesToFullGrown = 0;

	//if(m_Animalia)
		//m_Animalia->SetGrowthStage(m_MinutesToFullGrown ? Juvenile : Adult);

	if(m_Ordnance)
		delete m_Ordnance;
	m_Ordnance = 0;
	if(hasWhat & 0x80)
	{
		// Only the scene atom receives turret and weapon bay ammo information, for on-screen indicators.
		quint8 itemType;
		fromServer >> itemType;
		if(itemType)
		{
			m_Ordnance = AuItem::Create(itemType);
			if(m_Ordnance)
				fromServer >> *m_Ordnance;
		}
	}

	if(hasWhat & 0x40)
	{
		// The special weapon of my body is ready.
		// Update use time so weapon will show ready on client screen.
		if(hasWhat & 0x0100)
			m_Animalia->UseSpecialWeapon(AuTime::Now().AddDays(-1));
		else
			m_Body->UseSpecialWeapon(AuTime::Now().AddDays(-1));
	}
	else
	{
		// The special weapon of my body is NOT ready.
		// Update use time so weapon will not show ready on client screen.
		if(hasWhat & 0x0100)
			m_Animalia->UseSpecialWeapon(AuTime::Now());
		else
			m_Body->UseSpecialWeapon(AuTime::Now());
	}

	if(hasWhat & 0x0200)
	{
		// Haircut and shave.
		quint8 ui8;
		fromServer >> ui8;
		m_Haircut = (AuHairStyle::STYLE)ui8;
		fromServer >> ui8;
		m_Shave = (AuHairStyle::SHAVE)ui8;
	}

	// If I am in a vehicle, insure my seat index is correct.
	DaVehicle* vehicle = dynamic_cast<DaVehicle*>(Parent());
	if(vehicle)
		m_SeatIndex = vehicle->SeatAt(Location());
	else
		m_SeatIndex = 0;
}

// This is how the server receives the last user settings for a console from the client.
void DaCreature::RxConsoleSettings(QDataStream& fromClient)
{
	if(!IsConsolePose())
		return;

	m_ConsoleLock.lockForRead();
	if(m_Console)
	{
		// Read the byte array out of the datastream.
		QByteArray ba;
		fromClient >> ba;

		// Stream settings in from the byte array, just like when we put them in there.
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		m_Console->RxSettings(in);
		SetModified();
	}
	m_ConsoleLock.unlock();
}

// Refresh data from valid counterpart.
bool DaCreature::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaUnit::Refresh(pAtom, ari);

	const DaCreature* pA = (const DaCreature*)pAtom;
	m_MinutesToFullGrown = pA->m_MinutesToFullGrown;
	m_SeatIndex = pA->m_SeatIndex;
	m_Stamina = pA->m_Stamina;
	m_ControlId = pA->m_ControlId;

	float speedEnc = MovementSpeedMod();
	quint16 stationId = pA->StationId();
	//pA->m_BodyLock.lockForRead();
	m_BodyLock.lockForWrite();
	if(m_Body && pA->m_Animalia)
	{
		// Creature changed from old DpBody to animalia.
		if(ari)
			m_Body->DeleteDeviceObjects(*ari->re);
		delete m_Body;
		m_Body = 0;
		m_Animalia = new AuAnimalia(*pA->m_Animalia);
	}
	else if(m_Body)
	{
		m_Body->SetLeftHanded(pA->IsLeftHanded());
		m_Body->SetDNA(pA->DNA3(), speedEnc);
		m_Body->SetSex(pA->Gender());
		m_Body->UseSpecialWeapon(pA->SpecialWeaponUseTime());	// Readiness of special weapon indicator.
		m_Body->SetHitPoints(pA->CurrentBodyHitPoints());
	}
	else
	{
		pA->m_BodyLock.lockForRead();
		m_Animalia->Refresh(*pA->m_Animalia);
		pA->m_BodyLock.unlock();
	}
	m_Haircut = pA->m_Haircut;
	m_Shave = pA->m_Shave;
	m_BodyLock.unlock();
	//pA->m_BodyLock.unlock();

	if(m_Ordnance)
		delete m_Ordnance;
	if(pA->m_Ordnance)
		m_Ordnance = pA->m_Ordnance->Clone();
	else
		m_Ordnance = 0;

	if(CurrentHeadTilt() != pA->CurrentHeadTilt())
	{
		//qDebug("DaCreature::Refresh - SetHeadTilt %g",
				//pA->CurrentHeadTilt()*RAD2DEG);
		TiltHeadTo(pA->CurrentHeadTilt());
	}

	//if(Id() == AuId::Haxus() && CurrentGeneralPose() != pA->CurrentGeneralPose())
		//qDebug() << "DaCreature::Refresh" << Id() << Name() << "pose" << CurrentGeneralPose();
	if(pA->HasDNA3())
	{
		AuPose poses[DpBody::PS_Count];
		pA->CurrentPoses3(poses);
		SetPoses3(poses, stationId, m_ControlId);
	}
	else
	{
		AuPose poses[AuAnimalPart::PS_Count];
		pA->CurrentPoses4(poses);
		SetPoses4(poses, stationId, m_ControlId);
	}
	//if(IsMounted())
	//qDebug("DaCreature::Refresh %d pose %d",
			//Id().GetID(), CurrentGeneralPose());

	pA->m_ConsoleLock.lockForRead();
	m_ConsoleLock.lockForWrite();
	if(m_Console)
	{
		if(pA->m_Console && m_Console->Type() == pA->m_Console->Type())
		{
			m_Console->Refresh(pA->m_Console, *Parent());
		}
		else if(!IsConsolePose())
		{
			// Refresh gets called on the model's copy of the atom and on the view's copy of the atom.
			// The view's copy of the atom must delete the GL device objects before deleting its console here.
			// We know the OpenGL context is current when a non-zero 're' is passed to us.
			//qDebug("DaCreature::Refresh %x %d deleting unneeded console", (unsigned long int)this, Id().GetID());
			if(ari)
				m_Console->DeleteDeviceObjects(*ari->re);
			delete m_Console;
			m_Console = 0;
		}
	}
	m_ConsoleLock.unlock();
	pA->m_ConsoleLock.unlock();

	return false;
}

bool DaCreature::IsTicker() const
{
	if(DaUnit::IsTicker())
		return true;

	//if(HasConsole() && !IsConsolePose())
		//return true;

	//if(IsTurretIntegratedPose())
		//return true;

	bool isCon;
	bool isTur;
	bool mouthOpen;
	m_BodyLock.lockForRead();
	if(m_Animalia)
	{
		isCon = m_Animalia->IsConsolePose();
		isTur = m_Animalia->IsTurretIntegratedPose();
		mouthOpen = m_Animalia->CurrentMouthPose() != AuPose::MouthClosed;
	}
	else
	{
		isCon = m_Body->IsConsolePose();
		isTur = m_Body->IsTurretIntegratedPose();
		mouthOpen = false;
	}
	m_BodyLock.unlock();
	if(isCon || isTur || mouthOpen)
		return true;

	return false;
}

void DaCreature::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	DaUnit::Tick(last, now);
	if(IsDead())
		return;

	// NOTE: The following console deletion logic is very similar to that in AnimateDeviceObjects.
	if(HasConsole() && !IsConsolePose())
	{
		// Console is no longer needed.
		m_ConsoleLock.lockForWrite();
		if(m_Console)
		{
			delete m_Console;
			m_Console = 0;
		}
		m_ConsoleLock.unlock();
	}
	else if(IsTurretIntegratedPose())
	{
		// Operating a turret aboard a new design ship. I update the ship with my rotation. The gun orients to me, for rendering.
		DaSc* sc = dynamic_cast<DaSc*>(Parent());
		if(sc)
			sc->PositionTurret(*this);
	}

	// Some mouth poses expire after a time, for various reasons.
	bool poseComplete;
	m_BodyLock.lockForRead();
	if(m_Animalia)
		poseComplete = m_Animalia->MouthPoseExpired(m_MouthAnimStart, m_MouthAnimDur);
	else
		poseComplete = false;
	m_BodyLock.unlock();
	if(poseComplete)
	{
		m_BodyLock.lockForWrite();
		m_Animalia->PoseMouth(Id(), m_MouthFinish.MouthPose());
		m_MouthAnimDur = m_MouthFinish.DurationMs();
		if(m_MouthFinish.MouthPose() != m_MouthStationary)
		{
			// Prepare to really close my mouth after this pose finishes, if its animated. Otherwise I'm done.
			m_MouthAnimStart.restart();
			m_MouthFinish = AuVisage(m_MouthStationary, 0);
		}
		else
		{
			// First pose complete. Second is set up. Zero finish dur for the next time through; could be animating finish pose.
			m_MouthFinish.SetDurationMs(0);
		}
		m_tmPoseChange.Set();
		m_BodyLock.unlock();
		SetModified();
	}
}

void DaCreature::TickMovement(const AuElapsedTimer& last)
{
	/*
	if(IsAIControlled())
	{
		POSE pose = CurrentGeneralPose();
		if(pose == AuPose::StandWalk)
		{
			const AuPoint accel = Acceleration();
			const AuPoint drag = Drag();
			const AuPoint fric = Friction();
			const AuPoint dragfric = (drag + fric);
			if(accel.y <= dragfric.y)
			{
				//qDebug() << "DaCreature::TickMovement" << *this << "walk too weak, running";
				SetGeneralPose(AuPose::StandRun);
			}
		}
	}
	*/

	DaUnit::TickMovement(last);

	if(IsBodyDyingOrDead())
		return;

	if(IsJumping())
	{
		//qDebug() << "DaCreature::TickMovement" << Id() << Name() << "touchingGround" << IsTouchingGround() << "poseChanged"
			//<< m_tmPoseChange.ElapsedSec() << "seconds ago";
		bool touchingG = IsTouchingGround();
		m_BodyLock.lockForRead();
		bool stand = touchingG || m_tmPoseChange.ElapsedSec() >= 1;
		m_BodyLock.unlock();
		if(stand)
		{
			//qDebug() << "DaCreature::TickMovement" << Id() << Name() << "done jumping, stand";
			Stand();				// Stop jumping and resume standing.
			InvalidateEnvironment();
			SetModified();
		}
	}
	else if(IsLadderPose() && !InLadderWell())
	{
		// It is possible to slide off the bottom or top of a ladder.
		Stand();					// Resume standing when no longer near a ladder.
		InvalidateEnvironment();
		SetModified();
	}
	else if(IsProne() && IsMovingPose() && IsOnGround() && IsBurning())
	{
		// When moving prone on the ground, fires on my body are put out. This is how you "stop, drop and roll".
		AuCondition* fire;
		const AuCondition* ob = FindConditionLock(AuCondition::Fire);
		if(ob)
			fire = ob->Clone();
		else
			fire = 0;
		UnlockConditions();
		RemoveCondition(fire);
		SetModified();
	}
}

unsigned int DaCreature::IsTocker() const
{
	unsigned int ms = DaUnit::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is at least d_MSecPerMin.

	if(ms > d_MSecPerSec)
	{
		if(!IsBodyDyingOrDead())
			ms = d_MSecPerSec;
		else if(!IsConsolePose() && IsInAtmosphere() && !IsInSpacecraft() && !IsAboardVehicle())
			ms = d_MSecPerSec;
		else if(IsBurning() || HasAcid())
			ms = d_MSecPerSec;
		else
		{
			const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
			if(parentStar)
				ms = d_MSecPerSec;
		}
	}

	if(ms > d_MSecPerSec)
	{
		m_BodyLock.lockForRead();
		bool ise = (m_Body && m_Body->isEmpty()) || (m_Animalia && m_Animalia->Empty());
		m_BodyLock.unlock();
		if(ise)
			ms = d_MSecPerSec;
	}

	if(ms > 10*d_MSecPerSec)
	{
		if(InArenaOutsideBounds())
			ms = 10*d_MSecPerSec;
	}

	if(ms > d_MSecPerMin)
	{
		if(CurrentBodyHitPoints() > 0)
		{
			short int curhits = CurrentHitPoints();
			if(curhits < MaxHitPoints())
				ms = d_MSecPerMin;
		}

		if(!IsFullGrown() && IsInAnyScene())
			ms = d_MSecPerMin;
	}
	return ms;
}

void DaCreature::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaUnit::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;

	m_BodyLock.lockForRead();
	bool ise = (m_Body && m_Body->isEmpty()) || (m_Animalia && m_Animalia->Empty());
	m_BodyLock.unlock();
	if(ise)
	{
		m_BodyLock.lockForWrite();
		if(m_Body)
			m_Body->Init();					// NOTE: Does not clear "needs init" state of creature.
		else
			m_Animalia->Init();				// NOTE: Does not clear "needs init" state of creature.
		m_BodyLock.unlock();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Server needs to know the current gun model CID for effect placement.
	if(!IsConsolePose() && last.ElapsedSecTicks(now) > 0 && IsInAtmosphere() && !IsInSpacecraft() && !IsAboardVehicle())
	{
		// I am on a world not aboard a spacecraft or riding a vehicle; I could be riding a mount.
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld && llworld->Is(ParentId()))
		{
			// I am not riding a mount but I could be carrying a rider.
			// If I am standing inside the teleportation zone of an active henge, teleport to the destination, with my rider.
			AuId fromSysId;
			quint8 fromSun;
			quint8 fromArc;
			quint8 shadowPanels;
			AuId destSysId;
			quint8 destSun;
			quint8 destArc;
			if(llworld->IsHengeTeleportLocation(Location(), fromSysId, fromSun, fromArc, shadowPanels, destSysId, destSun, destArc))
				HengeTeleport(fromSysId, fromSun, fromArc, shadowPanels, destSysId, destSun, destArc);
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Body heals one point per hour naturally, when not starving or dying.
	if(g_Au.m_EnableCreatureDecay)
	{
		if(CurrentBodyHitPoints() > 0)
		{
			short int curhits = CurrentHitPoints();
			if(curhits < MaxHitPoints())
			{
				// Hit points are less than max.
				if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
				{
					// Heal one hit point.
					curhits++;
					SetHitPoints(curhits);
					SetModified();
					tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
				}
			}
		}
	}

	if(last.ElapsedMinTicks(now) > 0)
	{
		// Growing creature counts down until fully grown.
		if(GrowNaturally())
		{
			/*
			m_BodyLock.lockForRead();
			short int hp = m_Body.CurrentHitPoints();
			short int maxhp = m_Body.MaxHitPoints();
			m_BodyLock.unlock();
			qDebug() << "DaCreature::Tock" << Id() << Name() << "minutes" << m_MinutesToFullGrown
				<< "curHp" << CurrentHitPoints()
				<< "maxHp" << MaxHitPoints()
				<< "bodyCurHp" << hp
				<< "bodyMaxHp" << maxhp;
				*/

			m_MinutesToFullGrown--;
			if(IsOnGround())
				ForceMotion();	// So I will adjust to the terrain.
			SetModified();
			tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
		}
	}

	if(last.ElapsedSecTicks(now) > 0)
	{
		if(InArenaOutsideBounds())
		{
			AuCombatAttacker attacker(tr("Departed Arena Bounds"));

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_Mode = WM_Internal;
			weapon.m_w1.m_Effect = WE_Crush;

			AuCombatResult result;
			result.m_Damage1 = MaxHitPoints();

			CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		}

		if(IsCGSubmerged() && (IsBurning() || HasAcid()))
		{
			// Remove fire and acid conditions when CG is under water.
			m_ConditionsLock.lockForWrite();
			AuConditionList::iterator ci = m_Conditions.begin();
			while(ci != m_Conditions.end())
			{
				switch((*ci)->CType())
				{
				case AuCondition::Fire:
					delete *ci;
					ci = m_Conditions.erase(ci);
					break;
				case AuCondition::Acid:
					if((*ci)->Quality() > 1)
					{
						(*ci)->ReduceQuality(1);
						ci++;
					}
					else
					{
						delete *ci;
						ci = m_Conditions.erase(ci);
					}
					break;
				case AuCondition::Antitoxin:
				case AuCondition::Bandage:
				case AuCondition::Bounty:
				case AuCondition::UNUSED:
				case AuCondition::Disease:
				case AuCondition::ImplantDNA:
				case AuCondition::ImplantSpecies:
				case AuCondition::Invalid:
				case AuCondition::Paralysis:
				case AuCondition::Poison:
				case AuCondition::Pregnant:
				case AuCondition::Thought:
					ci++;
					break;
				}
			}
			m_tmConditionChange.Set();
			m_ConditionsLock.unlock();
			SetModified();
		}

		// Test for solar radiation damage if my parent is a sun.
		const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
		if(parentStar)
			ApplySolarRadiation(parentStar, MaxHitPoints());

		if(!IsBodyDyingOrDead())
		{
			int trend;
			int maxStamina;
			AuPose pose;

			m_BodyLock.lockForRead();
			if(m_Body)
			{
				trend = m_Body->StaminaTrend();
				maxStamina = m_Body->Stamina();
				pose = m_Body->CurrentGeneralPose();
			}
			else
			{
				trend = m_Animalia->StaminaTrend();
				maxStamina = m_Animalia->Stamina();
				pose = m_Animalia->CurrentGeneralPose();
			}
			m_BodyLock.unlock();

			// Update mating pose.
			if(pose.IsMating() && IsAIControlled())
			{
				// Do I still have a partner?
				if(!IsRider() && !IsMounted())
				{
					trend = 10;	// Afterglow is restful.
					if(m_Stamina >= maxStamina/2)
					{
						if(IsInsideCapsule())
							pose = AuPose::Stand;
						else
							pose = AuPose::SitOnGround;
						SetGeneralPose(pose);
						TiltHeadTo(0);
					}
				}
			}

			// Update stamina.
			if(!IsInGravity())
				trend += 2;		// Zero-G is restful.
			if(IsWearingEnvironmentSuit() && !pose.IsMating())
				trend -= 1;		// Fully enclosed in environment suit consumes more stamina, even if its seal is broken.
			if(trend > 0)
			{
				// When hunger reaches zero, creatures do not regain stamina.
				// NOTE: "Animals" never reach 0 hunger so they will always regain their stamina.
				// When holding breath under water, creatures do not regain stamina.
				if(CurrentHunger() && !(IsMouthSubmerged() && !LivesInWater() && !DbCommodity::IsEnvironmentHelmet(Helmet())))
				{
					// Body is not starving and not holding breath under water so body regains stamina.
					if(m_Stamina < maxStamina)
					{
						// Treat trend as a percentage of total stamina when regaining stamina. That way creatures with a lot of
						// stamina take a long time to get tired but creatures rest at the same rate.
						int regainStamina = maxStamina * trend / 100;
						if(!regainStamina)
							regainStamina = 1;

						if(m_Stamina + regainStamina >= maxStamina)
							m_Stamina = maxStamina;
						else
							m_Stamina += regainStamina;
						SetModified();
					}
				}
			}
			else if(trend < 0)
			{
				// Body is expending stamina.
				if(m_Stamina > 0)
				{
					// Body still has stamina.
					if(m_Stamina <= -trend)
					{
						// Stamina reached zero.
						m_Stamina = 0;
						if(pose.IsMating())
							Orgasm();
					}
					else
						m_Stamina += trend;
					SetModified();
				}
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

QString DaCreature::Description() const
{
	QString handed;
	if(IsLeftHanded())
		handed = tr("Left-handed");
	else
		handed = tr("Right-handed");

	QString sex = GenderName(Gender());

	QString dna;
	m_BodyLock.lockForRead();
	if(m_Body)
		dna = m_Body->DNA().Description();
	else
		dna = m_Animalia->Description();
	m_BodyLock.unlock();

	QString desc;
	if(m_MinutesToFullGrown)
		desc = tr("Juvenile %1 %2 %3").arg(handed).arg(sex).arg(dna);
	else
		desc = tr("%1 %2 %3").arg(handed).arg(sex).arg(dna);
	return desc;
}

void DaCreature::SetPoses3(const AuPose pose[DpBody::PS_Count], quint16 stationId, quint16 controlId)
{
	if(!m_Body)
		return;
	if(!SetStationPose(pose[DpBody::PS_LowerTorso], stationId, controlId))
	{
		float speedEnc = MovementSpeedMod();
		m_BodyLock.lockForWrite();
		if(m_Body->Pose(pose, HasItemInHand(), speedEnc) || m_StationId != stationId)
		{
			m_StationId = stationId;
			m_tmPoseChange.Set();
			m_ControlId = controlId;
		}
		m_BodyLock.unlock();
	}
}

void DaCreature::SetPoses4(const AuPose pose[AuAnimalPart::PS_Count], quint16 stationId, quint16 controlId)
{
	if(!m_Animalia)
		return;
	if(!SetStationPose(pose[AuAnimalPart::PS_General], stationId, controlId))
	{
		float speedEnc = MovementSpeedMod();
		m_BodyLock.lockForWrite();
		if(m_Animalia->Pose(Id(), pose, HasItemInHand(), speedEnc) || m_StationId != stationId)
		{
			//qDebug() << "DaCreature::SetPoses4" << *this << "mouth" << m_Animalia->CurrentMouthPose();
			m_StationId = stationId;
			m_tmPoseChange.Set();
			m_ControlId = controlId;
		}
		m_BodyLock.unlock();
	}
}

void DaCreature::RestorePose(const AuPose& pose, quint16 stationId)
{
	if(!SetStationPose(pose, stationId, m_ControlId))
	{
		m_ControlId = 0;
		SetGeneralPose(pose, stationId, false);
	}
}

bool DaCreature::SetStationPose(const AuPose& pose, quint16 stationId, quint16 controlId)
{
	AuStation::OCCUPANT_POSITION occPos = AuStation::OccPosForPose(pose);
	switch(pose.Pose())
	{
	case AuPose::GameTableSeat0_0:
		SitAtConsole(stationId, AuStation::Table_Game_0, occPos, 0, controlId);
		break;
	case AuPose::GameTableSeat1_0:
		SitAtConsole(stationId, AuStation::Table_Game_0, occPos, 1, controlId);
		break;
	case AuPose::GameTableSeat2_0:
		SitAtConsole(stationId, AuStation::Table_Game_0, occPos, 2, controlId);
		break;
	case AuPose::GameTableSeat3_0:
		SitAtConsole(stationId, AuStation::Table_Game_0, occPos, 3, controlId);
		break;
	case AuPose::GameTableSeat0_45:
		SitAtConsole(stationId, AuStation::Table_Game_45, occPos, 0, controlId);
		break;
	case AuPose::GameTableSeat1_45:
		SitAtConsole(stationId, AuStation::Table_Game_45, occPos, 1, controlId);
		break;
	case AuPose::GameTableSeat2_45:
		SitAtConsole(stationId, AuStation::Table_Game_45, occPos, 2, controlId);
		break;
	case AuPose::GameTableSeat3_45:
		SitAtConsole(stationId, AuStation::Table_Game_45, occPos, 3, controlId);
		break;
	case AuPose::ConsoleCommand:
		SitAtConsole(stationId, AuStation::Console_Command, occPos, 0, controlId);
		break;
	case AuPose::ConsoleNav:
		SitAtConsole(stationId, AuStation::Console_Navigation, occPos, 0, controlId);
		break;
	case AuPose::ConsoleEngine:
		SitAtConsole(stationId, AuStation::Console_Engine, occPos, 0, controlId);
		break;
	case AuPose::ConsoleFireControl:
		SitAtConsole(stationId, AuStation::Console_FireControl, occPos, 0, controlId);
		break;
	case AuPose::ConsoleSurgery:
		SitAtConsole(stationId, AuStation::Console_Medic, occPos, 0, controlId);
		break;
	case AuPose::ConsolePilot:
		SitAtConsole(stationId, AuStation::Console_Helm, occPos, 0, controlId);
		break;
	case AuPose::ConsolePower:
		SitAtConsole(stationId, AuStation::Console_Power, occPos, 0, controlId);
		break;
	case AuPose::ConsoleSensor:
		SitAtConsole(stationId, AuStation::Console_Sensor, occPos, 0, controlId);
		break;
	case AuPose::ConsoleShield:
		SitAtConsole(stationId, AuStation::Console_Shield, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTransporter:
		SitAtConsole(stationId, AuStation::Console_Transporter, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret0:
		SitAtConsole(stationId, AuStation::Turret_0, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret45:
		SitAtConsole(stationId, AuStation::Turret_45, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret90:
		SitAtConsole(stationId, AuStation::Turret_90, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret135:
		SitAtConsole(stationId, AuStation::Turret_135, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret180:
		SitAtConsole(stationId, AuStation::Turret_180, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret225:
		SitAtConsole(stationId, AuStation::Turret_225, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret270:
		SitAtConsole(stationId, AuStation::Turret_270, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurret315:
		SitAtConsole(stationId, AuStation::Turret_315, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurretBottom:
		SitAtConsole(stationId, AuStation::Turret_Bottom, occPos, 0, controlId);
		break;
	case AuPose::ConsoleTurretTop:
		SitAtConsole(stationId, AuStation::Turret_Top, occPos, 0, controlId);
		break;
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Command, occPos, 0, controlId);
		break;
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Engine, occPos, 0, controlId);
		break;
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_FireControl, occPos, 0, controlId);
		break;
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Helm, occPos, 0, controlId);
		break;
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Medic, occPos, 0, controlId);
		break;
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Navigation, occPos, 0, controlId);
		break;
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Power, occPos, 0, controlId);
		break;
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Sensor, occPos, 0, controlId);
		break;
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Shield, occPos, 0, controlId);
		break;
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Transporter, occPos, 0, controlId);
		break;
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		SitAtConsole(stationId, AuStation::Turret_Integrated, occPos, 0, controlId);
		break;
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
		SitAtConsole(stationId, AuStation::Troop, occPos, 0, controlId);
		break;
	case AuPose::Patient_Prone:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
		SitAtConsole(stationId, AuStation::Integrated_Patient, occPos, 1, controlId);
		break;
	default:
		return false;
	}
	return true;
}

// Set the pose for the appropriate type of console and insure the console exists.
AuPose DaCreature::SitAtConsole(quint16 stationId, AuStation::TYPE stationType, AuStation::OCCUPANT_POSITION occPos,
		unsigned int seatIndex, quint16 controlId)
{
	DaCapsule* ship = dynamic_cast<DaCapsule*>(Parent());

	bool deleteConsole = true;
	AuPose pose = AuStation::OpPose(stationType, occPos, seatIndex);
	switch(stationType)
	{
	case AuStation::Table_Game:
	case AuStation::Table_Game_0:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleGame(stationType, controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Table_Game_45:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleGame(stationType, controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Command:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleEmpty();		// In command mode we just draw the chair.
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Navigation:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleNav();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Engine:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleEngineer(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_FireControl:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleWeapon(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Helm:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleHelm();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Power:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsolePower();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Sensor:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleSensor();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Shield:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleShield();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Medic:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleSurgery(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Console_Transporter:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpConsoleTransporter(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Command:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScCommand();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Engine:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScEngineer(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_FireControl:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScWeapon(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Navigation:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScNav();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Power:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScPower();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Sensor:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScSensor();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Shield:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScShield();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Transporter:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScTransporter(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Helm:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScHelm();
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Medic:
		// Delete console if its not the right kind.
		m_ConsoleLock.lockForWrite();
		if(m_Console && CurrentGeneralPose() != pose)
		{
			delete m_Console;	// Sitting at wrong kind of console.
			m_Console = 0;
		}

		// Create the console if needed.
		deleteConsole = false;
		if(!m_Console)
			m_Console = new DpScSurgery(controlId);
		m_ConsoleLock.unlock();
		break;
	case AuStation::Integrated_Designer:	// We don't ever really get here with this setting.
		qWarning() << "error: DaCreature::SitAtConsole" << Id() << Name() << "invalid station Integrated_Designer";
		[[fallthrough]];
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
	case AuStation::Berth_Captain:
	case AuStation::Berth_Crew:
	case AuStation::Berth_Officer:
	case AuStation::Berth_Passenger:
	case AuStation::Berth_Troop:
	case AuStation::Citizen:
	case AuStation::Console_Patient:
	case AuStation::Integrated_Patient:
	case AuStation::Livestock:
	case AuStation::Troop:
	case AuStation::Turret_0:
	case AuStation::Turret_45:
	case AuStation::Turret_90:
	case AuStation::Turret_135:
	case AuStation::Turret_180:
	case AuStation::Turret_225:
	case AuStation::Turret_270:
	case AuStation::Turret_315:
	case AuStation::Turret_Bottom:
	case AuStation::Turret_Integrated:
	case AuStation::Turret_Top:
		deleteConsole = true;
		break;
	case AuStation::TYPE_Count:
#if defined(QT_NO_DEBUG)
	default:
#endif
		qWarning() << "error: DaCreature::SitAtConsole" << Id() << Name() << "stationType" << stationType
			<< "seatIndex" << seatIndex << "stationType not a console pose, standing instead.";
		break;
	}

	// Delete old console data if its not needed.
	if(deleteConsole)
	{
		m_ConsoleLock.lockForWrite();
		if(m_Console)
		{
			delete m_Console;
			m_Console = 0;
		}
		m_ConsoleLock.unlock();
	}
	else if(HasConsole() && ship)
		ship->SetConsoleEntered(this);

	// Pose the body.
	m_BodyLock.lockForWrite();
	if(m_Body)
	{
		if(m_Body->Pose(pose, HasItemInHand(), 1) || m_StationId != stationId)
		{
			//qDebug() << "DaCreature::SitAtConsole" << Id() << Name() << "stationId" << stationId << "pose" << pose;
			m_StationId = stationId;
			m_tmPoseChange.Set();
		}
	}
	else
	{
		if(m_Animalia->Pose(Id(), pose, HasItemInHand(), 1) || m_StationId != stationId)
		{
			//qDebug() << "DaCreature::SitAtConsole" << Id() << Name() << "stationId" << stationId << "pose" << pose;
			m_StationId = stationId;
			m_tmPoseChange.Set();
		}
	}
	m_BodyLock.unlock();
	m_ControlId = controlId;
	return pose;
}

void DaCreature::ExitConsole()
{
	if(!IsLockedPose())
		return;

	// Exit the game in progress at the game table, if there is one.
	if(IsGameTablePose())
	{
		DaCapsule* cap = dynamic_cast<DaCapsule*>(Parent());
		if(cap)
			cap->GameExitPlayer(StationId(), Id());
	}

	// Get information based on my current pose before changing my pose.
	const double oldHt = GroundHeightAboveCG();		// Sitting ht

	// Change my pose to standing and get the change in Z so I can adjust my body perfectly.
	Stand();
	const double nuHt = GroundHeightAboveCG();		// Standing ht
	const double zAdj = nuHt - oldHt;				// Stand portion of move vec

	// Sitting at a console or somehow unable to exit a turret.
	if(!Move(AuPoint(0,0,zAdj), 0))						// try standing in place
	{
		if(IsLeftHanded())
		{
			// Left-handed people tend to get out on the left.
			if(!Move(AuPoint(-2,0,zAdj), 0))				// try left of the seat
				if(!Move(AuPoint(2,0,zAdj), 0))			// try right of the seat
					if(!Move(AuPoint(0,2,zAdj), 0))		// try forward of the seat
						Move(AuPoint(0,-2,zAdj), 0);		// try aft of the seat
		}
		else
		{
			// Right-handed people tend to get out on the right.
			if(!Move(AuPoint(2,0,zAdj), 0))				// try right of the seat
				if(!Move(AuPoint(-2,0,zAdj), 0))			// try left of the seat
					if(!Move(AuPoint(0,2,zAdj), 0))		// try forward of the seat
						Move(AuPoint(0,-2,zAdj), 0);		// try aft of the seat
		}
	}
	ForceMotion();											// So gravity will adjust me to the floor.
	m_ControlId = 0;
	InvalidateEnvironment();
	SetModified();
}

// Gravity affect is applied both to falling down and pushing up to the surface.
bool DaCreature::IsAffectedByGravity() const
{
	if(NeedsInit())
		return false;

	if(IsLockedPose())
	{
		//qDebug("DaCreature::IsAffectedByGravity %d %s sitting at console is not affected by gravity.", Id().GetID(),
				//(const char*)Name().toAscii());
		return false;	// Don't bother with gravity when sitting at a console.
	}

	if(InLadderWell() || IsLadderPose())
		return false;	// Don't bother with gravity when clinging to a ladder or standing over an open hatch.

	if(IsUnderGround())
	{
		if(LivesUnderGround())
		{
			//qDebug("DaCreature::IsAffectedByGravity %d %s underground "
					//"is not affected by gravity.", Id().GetID(),
					//(const char*)Name());
			return false;	// Diggers are not pushed to surface when under ground.
		}
	}

	if(IsCGSubmerged())
	{
		bool is;
		m_BodyLock.lockForRead();
		if(m_Body)
			is = m_Body->DNA().LivesInWater && !m_Body->IsDead() && !m_Body->IsDying();
		else
			is = m_Animalia->LivesInWater() && !m_Animalia->IsDead() && !m_Animalia->IsDying();
		m_BodyLock.unlock();
		if(is)
		{
			//qDebug("DaCreature::IsAffectedByGravity %d %s living under water "
					//"is not affected by gravity.", Id().GetID(),
					//(const char*)Name());
			return false;	// Swimmers hold their desired altitude under water.
		}
	}

	if(IsFlying() && CanFly())
	{
		//qDebug("DaCreature::IsAffectedByGravity %d %s flying pose %d "
				//"is not affected by gravity.", Id().GetID(),
				//(const char*)Name(), m_ePose);
		return false;	// Flyers are not affected by gravity while flying.
	}

	return true;
}

float DaCreature::GroundHeightAboveCG() const
{
	//if(m_Body.CurrentGeneralPose() != AuPose::Straddle &&
	//		m_Body.CurrentGeneralPose() != AuPose::EyesOnly &&
	//		DNA().LegPairs &&
	//		DNA().BodyCarriage == 3 &&
	//		DNA().HasAnkleBend())
	//{
	//	// We must allow a little extra space for the thickness of the foot.
	//	// Calculate foot thickness.
	//	const float ct = m_Body.CalfThickness();
	//	const float fl = m_Body.FootLength();
	//	float ft = fl/2.f;
	//	if(ft > ct)
	//		ft = ct * 1.1f;	// Feet are a tiny bit bigger than calves.
	//	return m_Body.CGHeight() + ft;
	//}
	//else
	AuPose pose = CurrentGeneralPose();
	if(pose == AuPose::ConsoleTurretBottom)
		return AuStation::SeatHeight() - 1;		// Seat is sunken down into the bottom turret by 1'.
	else if(pose == AuPose::ConsoleTurretTop)
		return DeckHeight() - DpBody::SeatDrop();	// Seat dangles 2.5' below the ceiling turret.
	return CGHeight();
}

// Used for mini me display on screen.
void DaCreature::RenderMiniMe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, double scale)
{
	//qDebug("DaCreature::RenderMiniMe");
	if((m_Body && m_Body->NeedsInit()) ||
			(m_Animalia && m_Animalia->NeedsInit()))
		return;

	// Set current ambient light.
	ari.re->SetAmbientLight(1.f, 1.f, 1.f, 1.f);

	// Multiply by current transformation.
	AuMatrix4x4 tThis(model);
	tThis.Scale(scale, scale, scale);
	AuMatrix4x4 tBody(tThis);
	BodyPoseAdjustTransform(tBody);

	AuPoint growthScale(GrowthScaleBody());

	// Calculate ambient light up as against gravity. That is generally correct.
	AuPoint ambientLightUp = tBody.ProjectVector(AuPoint(0,0,1));
	ambientLightUp.Unit();
	//ambientLightUp = AuPoint(0,1,0);
	ari.re->DisableSuns();
	ari.re->DisableLights();

	// Calculate matrix for the body model, not applied to guns and consoles.
	AuStation station;
	float headTilt = 0;
	bool restoreHeadTilt = false;
	if(m_RenderedHeadTurn != 0)
	{
		// Position body straight in the station.
		tBody.RotateZ(-m_RenderedHeadTurn);

		// Turn head to match real body rotation.
		float headTurn = m_RenderedHeadTurn;
		headTilt = CurrentHeadTilt();
		if(m_Body)
		{
			if(headTurn > PI/2)
				headTurn = PIf/2;
			else if(headTurn < -PI/2)
				headTurn = -PIf/2;
			m_Body->TiltHeadsTo(headTilt, headTurn);
			m_Body->PositionHeads();
		}
		else
		{
			m_Animalia->TiltHeadsTo(headTilt, headTurn);
			m_Animalia->AnimatePose(Id(), AuElapsedTimer::Now());
		}
		restoreHeadTilt = true;
	}

	AuMatrix4x4 tThisBody(tBody);

	// Must offset my body model by the CG offset to my body's pivot point, which is where it renders.
	AuPoint cgOffset(CGOffset());
	if(!cgOffset.IsZero() || !IsFullGrown())
	{
		// Offset and scale my personal body rendering.
		tBody.Translate(cgOffset);
		tBody.Scale(growthScale);
	}

	// Render the body of this creature.
	if(m_Body)
		m_Body->RenderSkin(projection, tBody, *ari.re, ambientLightUp, false);
	else
	{
		quint8 visorState = (Helmet() != CID_Invalid) ? 1 : 0;
		m_Animalia->RenderAnimal(projection, tBody, *ari.re, false, visorState, ambientLightUp);
	}

	if(restoreHeadTilt)
	{
		if(m_Body)
		{
			m_Body->TiltHeadsTo(headTilt, 0);
			m_Body->PositionHeads();
		}
		else
		{
			m_Animalia->TiltHeadsTo(headTilt, 0);
			m_Animalia->PositionHeads();
		}
	}

	AuRenderLastMap renderLast;
	{
		// Render the console at which this atom is sitting.
		// Don't render anything when sitting at a game table.
		DpStation* console = LockConsoleForRead();
		if(console && !AuStation::IsGameTable(console->Type()))
			console->RenderMini(projection, tThisBody, this, ari);
		UnlockConsole();

		// Render the item in hand. RenderItemInHand already accounts for my CGOffset, if any.
		bool renderItemInHand = HasItemInHand() &&
			((m_Body && !m_Body->CurrentGeneralPose().HeadOnly() && !m_Body->IsTurretIntegratedPose()) ||
			((m_Animalia && !m_Animalia->CurrentGeneralPose().HeadOnly() && !m_Animalia->IsTurretIntegratedPose())));
		if(renderItemInHand)
			RenderItemInHand(projection, tThisBody, ari, 1, this, &renderLast);
	}

	AuModelMatrixHash rmod;
	rmod.insert(Id(), tThis);

	// The dying body check is for the entity effect on dying highborn.
	// The minutes to full grown check is for nuborn highborn.
	if(HasDNA4() || IsAnyConditionVisible() || IsBodyDyingOrDead())
		renderLast.Insert(rmod, this);
	else if(!IsFullGrown() && m_MinutesToFullGrown >= GrowthTime()-1)
		renderLast.Insert(rmod, this);							// Newborn for the first minute to two minutes.

	// Base atom class needs to render special effects.
	DaAtomList shadSc;
	DaAtomList shadScFog;
	DaAtomList shadScGlobe;
	DaAtomList shadScGlobeFog;
	DaAtomList shadScRingworld;
	DaAtomList shadScRingworldFog;
	DaUnit::RenderEnvironmentObject(projection, ari, rmod, (const DaPerson*)this, renderLast, PASS_Near, shadSc, shadScFog,
			shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
	if(renderLast.size())
		RenderLast(projection, ari, rmod, (const DaPerson*)this, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog);
}

// Renders this object in a scene.
// View transformation has already been set up.
void DaCreature::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Do not render distant creatures.
	if(pass != PASS_Near)
		return;

	//qDebug("DaCreature::RenderEnvironmentObject");
	if((m_Body && m_Body->NeedsInit()) ||
			(m_Animalia && m_Animalia->NeedsInit()) ||
			AgentInvisible())
	{
		// Don't render anything, not even a noise.
		//qDebug() << "DaCreature::RenderEnvironmentObject" << Id() << Name() << "NeedsInit";
		return;
	}

	// Multiply by current transformation.
	AuMatrix4x4 tThis = rmod.Matrix(Id());
	const AuPoint& pv = rmod.ViewLoc(Id());
	AuMatrix4x4 tThisBody(tThis);

	// Determine if this creature is in front of the camera plane.
	//qDebug() << "DaCreature::RenderEnvironmentObject" << Id() << Name() << "pv.z" << pv.z;
	if(pv.z < 0 || pv.Length2() < 10*10)
	{
		// Set current ambient light and fog states.
		float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
		bool sunsBlocked;
		SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(),
				sunsBlocked);
		ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);
		ari.re->SetFog(fogR, fogG, fogB, fogDensity);

		// Disable suns when they are blocked at my location. Prepare to restore them.
		QList<int> sunsDisabled;
		if(sunsBlocked)
			ari.re->DisableSuns(sunsDisabled);

		// This creature is in front of the camera plane or within 10' of the camera.
		BodyPoseAdjustTransform(tThis);
		tThisBody = tThis;

		// Render atoms subordinate to this animal, i.e. riding.
		DaAtomList subs;
		ListSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
					shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		// Scene atom has an option to turn off rendering their body.
		// Scene atom's body is not rendered when manning a turret so it doesn't get in the way.
		AuPoint scale(GrowthScaleBody());
		if(this != sceneAtom || g_Au.DrawBodyModel())
		{
			quint8 visorState = (Helmet() != CID_Invalid) ? 1 : 0;

			// Calculate ambient light up as against gravity. That is generally correct.
			AuPoint ambientLightUp = tThis.ProjectVector(GravityNormal());
			ambientLightUp.Unit();

			// Must offset my body model by the CG offset to my body's pivot point, which is where it renders.
			// When riding a mount, make adjustments to my body position to appear to stay exactly positioned at the saddle.
			AuPoint cgOffset(CGOffset());
			const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
			if(mount)
			{
				// Determine proper location of mount's saddle.
				AuPoint rideLoc = mount->CGOffset();				// Riders sit at mount's body pivot point.
				float bodyThick = mount->TorsoThickness()/2;		// Allow for mount's body thickness.
				rideLoc += AuPoint(0, -bodyThick, bodyThick);		// Move back and up by mount's body thickness.

				// Creature rider positioning. Location of creature bodies when riding an animal is their waist/pivot point.
				rideLoc.z += TorsoThickness()/2;					// Move up by the rider's body thickness.
				cgOffset += rideLoc - Location();
			}

			// Calculate matrix for the body model.
			AuMatrix4x4 tBody(tThis);
			AuStation station;
			float headTilt = 0;
			bool restoreHeadTilt = false;
			if(!mount && Station(station) && station.IsStationary())
			{
				// Calculate z rotation needed to position body straight in the station.
				AuPoint myX = Transform().ProjectVector(1,0,0);	// My X vector relative to my parent atom.
				myX = station.OpM().UnProjectVector(myX);		// My X vector relative to the station.
				m_RenderedHeadTurn = myX.ZRotation();

				// Position body straight in the station.
				tThisBody = rmod.Matrix(ParentId()) * station.OpM();
				tBody.RotateZ(-m_RenderedHeadTurn);

				// Turn head to match real body rotation.
				float headTurn = m_RenderedHeadTurn;
				headTilt = CurrentHeadTilt();
				if(m_Body)
				{
					if(headTurn > PI/2)
						headTurn = PIf/2;
					else if(headTurn < -PI/2)
						headTurn = -PIf/2;
					m_Body->TiltHeadsTo(headTilt, headTurn);
					m_Body->PositionHeads();
				}
				else
				{
					m_Animalia->TiltHeadsTo(headTilt, headTurn);
					m_Animalia->AnimatePose(Id(), AuElapsedTimer::Now());
				}
				restoreHeadTilt = true;
			}
			else
				m_RenderedHeadTurn = 0;
			if(!cgOffset.IsZero() || !IsFullGrown())
			{
				// Offset and scale my personal body rendering.
				tBody.Translate(cgOffset);
				tBody.Scale(scale);
			}

			// Render the body of this creature using the appropriate shader.
			bool headless = this == sceneAtom && g_Au.DrawBodyModel() == 2;	// Scene atom may want his body to be headless.
			if(IsInAtmosphere())
			{
				const DaWorld* world = FindParentPlanetInEnv();
				if(world)
				{
					const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
					if(rsworld)
					{
						// Inside atmosphere of a ringworld.
						float shadowPanelSunAngles[MAX_SUNS] = {0, 0, 0};
						for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
						{
							const AuGLSun& sun = ari.re->Sun(sunIndex);
							shadowPanelSunAngles[sunIndex] = rsworld->ShadowPanelSunAngle(sun.m_ShadowAngle);
						}
						if(m_Body)
							m_Body->RenderSkinFogRingworld(projection, tBody, *ari.re, ambientLightUp, shadowPanelSunAngles,
									headless);
						else
							m_Animalia->RenderAnimalRingworldFog(projection, tBody, *ari.re, false, visorState, ambientLightUp,
									shadowPanelSunAngles);
					}
					else
					{
						// Globe position in eye space is needed to calculate sun over-the-horizon effect.
						AuMatrix4x4 m;
						GetTransformRelativeToParent(m, world);
						AuPoint globeP = m.UnProject(0,0,0);		// Globe's position relative to me.
						globeP = tBody.Project(globeP);				// Globe's position in eye space.
						if(m_Body)
							m_Body->RenderSkinFogGlobe(projection, tBody, *ari.re, ambientLightUp, globeP, headless);
						else
							m_Animalia->RenderAnimalGlobeFog(projection, tBody, *ari.re, false, visorState, ambientLightUp, globeP);
					}
				}
			}
			else
			{
				if(m_Body)
					m_Body->RenderSkin(projection, tBody, *ari.re, ambientLightUp, headless);
				else
					m_Animalia->RenderAnimal(projection, tBody, *ari.re, false, visorState, ambientLightUp);
			}

			if(restoreHeadTilt)
			{
				if(m_Body)
				{
					m_Body->TiltHeadsTo(headTilt, 0);
					m_Body->PositionHeads();
				}
				else
				{
					m_Animalia->TiltHeadsTo(headTilt, 0);
					m_Animalia->PositionHeads();
				}
			}
		}

		// For the scene atom, the gun in hand and the console at which the scene atom is sitting are rendered in :RenderScene.
		if(this != sceneAtom)
		{
			// Render the console at which this non-scene atom is sitting.
			// Don't render anything when sitting at a game table.
			if(m_Console && !AuStation::IsGameTable(m_Console->Type()))
				m_Console->Render(projection, projection, tThis, *sceneAtom, this, ari);

			// Render the item in hand. RenderItemInHand already accounts for my CGOffset, if any.
			bool renderItemInHand = HasItemInHand() &&
				((m_Body && !m_Body->CurrentGeneralPose().HeadOnly() && !m_Body->IsTurretIntegratedPose()) ||
				((m_Animalia && !m_Animalia->CurrentGeneralPose().HeadOnly() && !m_Animalia->IsTurretIntegratedPose())));
			if(renderItemInHand)
				RenderItemInHand(projection, tThisBody, ari, 1, sceneAtom, &renderLast);
		}

		// Restore suns that I disabled.
		ari.re->EnableSuns(sunsDisabled);

		// Show sight surfaces for debugging.
		/*
		{
			// Select vertex shader and configure its parameters.
			CGprogram vp;
			vp = re.SelectVSolar();
			re.SetModelView(vp);
			re.SetModelViewProjection(vp);
			re.SelectFPassthrough();

			AuQuadArray aTargetSurfaces;
			EnumSightSurfaces(aTargetSurfaces, AuMatrix4x4());
			re.glDisable(GL_CULL_FACE);
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for(AuQuadArray::const_iterator qi = aTargetSurfaces.cend()-1; qi >= aTargetSurfaces.cbegin(); qi--)
			{
				re.glColor4f(1,1,1,.25);
				re.glBegin(GL_QUADS);
				re.glNormal3d((*qi).nll.x, (*qi).nll.y, (*qi).nll.z);
				re.glVertex3d((*qi).vll.x, (*qi).vll.y, (*qi).vll.z);
				re.glVertex3d((*qi).vul.x, (*qi).vul.y, (*qi).vul.z);
				re.glVertex3d((*qi).vur.x, (*qi).vur.y, (*qi).vur.z);
				re.glVertex3d((*qi).vlr.x, (*qi).vlr.y, (*qi).vlr.z);
				re.glEnd();

				re.glColor4f(1,1,1,1);
				re.glBegin(GL_LINE_STRIP);
				re.glNormal3d((*qi).nll.x, (*qi).nll.y, (*qi).nll.z);
				re.glVertex3d((*qi).vll.x, (*qi).vll.y, (*qi).vll.z);
				re.glVertex3d((*qi).vul.x, (*qi).vul.y, (*qi).vul.z);
				re.glVertex3d((*qi).vur.x, (*qi).vur.y, (*qi).vur.z);
				re.glVertex3d((*qi).vlr.x, (*qi).vlr.y, (*qi).vlr.z);
				re.glVertex3d((*qi).vll.x, (*qi).vll.y, (*qi).vll.z);
				re.glEnd();
			}
			re.glDisable(GL_BLEND);
			re.glEnable(GL_CULL_FACE);

			re.DeselectCg();
		}
		*/

		if(HasDNA4() || IsAnyConditionVisible())
			renderLast.Insert(rmod, this);
	}

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, tThisBody, ari.al, ari.fade);

	// Base atom class needs to render special effects.
	DaUnit::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

void DaCreature::RenderItemInHand(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, int pass,
			const DaAtom* sceneAtom, AuRenderLastMap* renderLast)
{
	bool isLefty;
	AuPose gp;
	if(m_Body)
	{
		isLefty = m_Body->IsLeftHanded();
		gp = m_Body->CurrentGeneralPose();
	}
	else
	{
		isLefty = m_Animalia->IsLeftHanded();
		gp = m_Animalia->CurrentGeneralPose();
	}
	bool isBerth = gp.IsBerth();
	bool isDead = gp.IsDead();
	bool isDying = gp.IsDying();
	bool isPatient = gp.IsPatientProne();

	// Multiply by current transformation (Translation, rotation, and scale)
	AuMatrix4x4 tcon;
	if(HasDNA3())
	{
		tcon.Translate(WeaponLoc());		// Move to body location.
		//qDebug() << "DaCreature::RenderItemInHand" << *this << "weaponLoc" << WeaponLoc();
		if(isDying || isDead || isBerth || isPatient)
		{
			switch(gp.Pose())
			{
			default:
				break;
			case AuPose::BerthBottomFaceLeft:
			case AuPose::BerthTopFaceLeft:
			case AuPose::DeadFaceLeft:
			case AuPose::WritheFaceLeft:
				if(isLefty)
					tcon.Rotate(AuPoint(0,0,PI/2));
				else
					tcon.Rotate(AuPoint(-PI/2,0,PI/2));
				break;
			case AuPose::BerthBottomFaceRight:
			case AuPose::BerthTopFaceRight:
			case AuPose::DeadFaceRight:
			case AuPose::WritheFaceRight:
				if(isLefty)
					tcon.Rotate(AuPoint(-PI/2,0,PI/2));
				else
					tcon.Rotate(AuPoint(0,0,PI/2));
				break;
			case AuPose::BerthBottomFaceDown:
			case AuPose::BerthTopFaceDown:
			case AuPose::DeadFaceDown:
			case AuPose::WritheFaceDown:
				tcon.Rotate(AuPoint(-PI,0,PI/2));
				break;
			case AuPose::BerthBottomFaceUp:
			case AuPose::BerthTopFaceUp:
			case AuPose::DeadFaceUp:
			case AuPose::WritheFaceUp:
			case AuPose::ReceptiveMate:
			case AuPose::MateOnBottom:
			case AuPose::Patient_Prone:
				if(isLefty)
					tcon.Rotate(AuPoint(0,PI,-PI/2));
				else
					tcon.Rotate(AuPoint(-PI,0,PI/2));
				break;
			}
		}
		else
			tcon.Rotate(WeaponRot());			// Rotate to proper look direction.
	}
	else
	{
		tcon = WeaponMatrix();
		switch(ItemInHandAimAxis())
		{
		case 0:
			// Model was created with positive Z up, aiming down the positive X axis.
			break;

		case 1:
			// Model was created aiming down the positive Y axis, with Z up.
			tcon.RotateZ(-PI/2);
			break;

		case 2:	// I don't think any models are created this way.
		default:
			qWarning() << "error: DaCreature::RenderItemInHand" << g_Au.CommodityName(ItemInHandCID())
				<< "unexpected item aim axis" << ItemInHandAimAxis();
			break;
		}
	}

	m_ItemInHandLock.lockForWrite();
	if(!m_ItemInHand)
	{
		m_ItemInHandLock.unlock();
		return;
	}

	// Multiply by current transformation (Translation, rotation, and scale.)
	AuMatrix4x4 tThis = model * tcon;

	if(m_ItemInHand->CommodityId() == CID_Head && m_ItemInHand->DNA() && m_ItemInHand->DNA()->Version() == 3)
	{
		// Heads seem to be centered between the eyes. It makes holding a head in hand look funny, fist sticking through the face.
		tThis.Translate(0, .5, 0);
	}

	AuPoint gup = GravityNormal() - AuPoint(Vector().x, Vector().y, 0)*AirDensity();
	gup = tcon.UnProjectVector(gup);

	// Flower pot wants wind vector in eye space.
	AuPoint windVec;
	if(DbCommodity::IsFlowerPot(m_ItemInHand->CommodityId()))
		windVec = tThis.ProjectVector(Wind());

	if(IsMini())
	{
		// Mini me display.
		AuPoint ambientUp = tThis.ProjectVector(AuPoint(0,0,1));
		ambientUp.Unit();
		m_ItemInHand->Render(projection, tThis, *ari.re, ari.Models(), Id(), 1,1,1, ambientUp, gup, windVec, 1, pass, MiniScale());
		if(renderLast)
			renderLast->Insert(0, this);
	}
	else
	{
		// Get ambient lighting and fog values.
		float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
		bool sunsBlocked;
		SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(),
				sunsBlocked);
		AuPoint ambientUp = tThis.ProjectVector(GravityNormal());

		// Disable suns when they are blocked at my location. Prepare to restore them.
		QList<int> sunsDisabled;
		if(sunsBlocked)
			ari.re->DisableSuns(sunsDisabled);

		if(IsInAtmosphere())
		{
			const DaWorld* world = FindParentPlanetInEnv();
			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
			if(world)
			{
				if(rsworld)
				{
					// Inside atmosphere of a ringworld.
					float shadowPanelSunAngles[MAX_SUNS];
					for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
						shadowPanelSunAngles[sunIndex] = rsworld->ShadowPanelSunAngle(ari.re->Sun(sunIndex).m_ShadowAngle);

					double sceneDepth = m_ItemInHand->RenderOnRingworld(projection, tThis, *ari.re, ari.Models(), Id(), ambientR,
							ambientG, ambientB, ambientUp, gup, fogDensity, fogR, fogG, fogB, shadowPanelSunAngles, windVec, 1,
							pass);
					if(renderLast && sceneDepth)
						renderLast->Insert(sceneDepth, this);
				}
				else
				{
					// Inside atmosphere of a globe.
					// Globe position in eye space is needed to calculate sun over-the-horizon effect.
					AuMatrix4x4 m;
					GetTransformRelativeToParent(m, world);
					AuPoint globeP = m.UnProject(0,0,0);		// Globe's position relative to me.
					globeP = tThis.Project(globeP);				// Globe's position in eye space.

					double sceneDepth = m_ItemInHand->RenderOnGlobe(projection, tThis, *ari.re, ari.Models(), Id(), ambientR,
							ambientG, ambientB, ambientUp, gup, fogDensity, fogR, fogG, fogB, globeP, windVec, 1, pass);
					if(renderLast && sceneDepth)
						renderLast->Insert(sceneDepth, this);
				}
			}
		}
		else
		{
			// Not inside atmosphere range of a world.
			double sceneDepth = m_ItemInHand->Render(projection, tThis, *ari.re, ari.Models(), Id(), ambientR, ambientG, ambientB,
					ambientUp, gup, windVec, 1, pass, 1.);
			if(renderLast && sceneDepth)
				renderLast->Insert(sceneDepth, this);
		}

		// Restore suns that I disabled.
		ari.re->EnableSuns(sunsDisabled);
	}

	m_ItemInHandLock.unlock();
}

void DaCreature::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	DaUnit::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);

	bool renderItemInHand = HasItemInHand() && g_Au.DrawGun() &&
		((m_Body && !m_Body->CurrentGeneralPose().HeadOnly() && !m_Body->IsTurretIntegratedPose()) ||
		((m_Animalia && !m_Animalia->CurrentGeneralPose().HeadOnly() && !m_Animalia->IsTurretIntegratedPose())));
	if(renderItemInHand)
	{
		//AuMatrix4x4 tThis(model);
		//BodyPoseAdjustTransform(tThis);

		// Render the item in hand. RenderItemInHand already accounts for my CGOffset, if any.
		// Must offset my body model by the CG offset to my body's pivot point, which is where it renders.
		// When riding a mount, make adjustments to my body position to appear to stay exactly positioned at the saddle.
		const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
		if(mount)
		{
			// Determine proper location of mount's saddle.
			AuPoint rideLoc = mount->CGOffset();				// Riders sit at mount's body pivot point.
			float bodyThick = mount->TorsoThickness()/2;		// Allow for mount's body thickness.
			rideLoc += AuPoint(0, -bodyThick, bodyThick);		// Move back and up by mount's body thickness.

			// Creature rider positioning. Location of creature bodies when riding an animal is their waist/pivot point.
			rideLoc.z += TorsoThickness()/2;					// Move up by the rider's body thickness.

			// Save the world transformation matrix so I can restore it on exit.
			AuMatrix4x4 tBody(rmod.Matrix(Id()));
			tBody.Translate(rideLoc - Location());
			RenderItemInHand(projection, tBody, ari, 2, sceneAtom, 0);
		}
		else
		{
			// Save the world transformation matrix so I can restore it on exit.
			RenderItemInHand(projection, rmod.Matrix(Id()), ari, 2, sceneAtom, 0);
		}
	}

	if(m_Animalia)
	{
		// Set current ambient light and fog states.
		float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
		bool sunsBlocked;
		SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(),
				sunsBlocked);
		ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);
		ari.re->SetFog(fogR, fogG, fogB, fogDensity);

		// Disable suns when they are blocked at my location. Prepare to restore them.
		QList<int> sunsDisabled;
		if(sunsBlocked)
			ari.re->DisableSuns(sunsDisabled);

		// This creature is in front of the camera plane or within 10' of the camera.
		AuMatrix4x4 tBody(rmod.Matrix(Id()));
		BodyPoseAdjustTransform(tBody);

		// Must offset my body model by the CG offset to my body's pivot point, which is where it renders.
		AuPoint cgOffset(CGOffset());
		if(m_RenderedHeadTurn != 0)
			tBody.RotateZ(-m_RenderedHeadTurn);
		if(!cgOffset.IsZero() || !IsFullGrown())
		{
			// Offset and scale my personal body rendering.
			AuPoint growthScale(GrowthScaleBody());
			tBody.Translate(cgOffset);
			tBody.Scale(growthScale);
		}

		quint8 visorState = (Helmet() != CID_Invalid) ? 1 : 0;

		AuPoint ambientLightUp = tBody.ProjectVector(GravityNormal());
		ambientLightUp.Unit();
		if(IsInAtmosphere())
		{
			const DaWorld* world = FindParentPlanetInEnv();
			if(world)
			{
				const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
				if(rsworld)
				{
					// Inside atmosphere of a ringworld.
					float shadowPanelSunAngles[MAX_SUNS] = {0, 0, 0};
					for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
					{
						const AuGLSun& sun = ari.re->Sun(sunIndex);
						shadowPanelSunAngles[sunIndex] = rsworld->ShadowPanelSunAngle(sun.m_ShadowAngle);
					}
					m_Animalia->RenderAnimalRingworldFog(projection, tBody, *ari.re, true, visorState, ambientLightUp,
							shadowPanelSunAngles);
				}
				else
				{
					// Globe position in eye space is needed to calculate sun over-the-horizon effect.
					AuMatrix4x4 m;
					GetTransformRelativeToParent(m, world);
					AuPoint globeP = m.UnProject(0,0,0);		// Globe's position relative to me.
					globeP = tBody.Project(globeP);				// Globe's position in eye space.
					m_Animalia->RenderAnimalGlobeFog(projection, tBody, *ari.re, true, visorState, ambientLightUp, globeP);
				}
			}
		}
		else
			m_Animalia->RenderAnimal(projection, tBody, *ari.re, true, visorState, ambientLightUp);

		// Restore suns that I disabled.
		ari.re->EnableSuns(sunsDisabled);
	}

	// Render smoke and fire on burning/smoldering creatures before shields.
	if(IsAnyConditionVisible())
	{
		AuMatrix4x4 myM(rmod.Matrix(Id()));
		myM.Translate(TargetLocation());
		float effectRadius = TorsoThickness() * 2;	// Bigger than my body so it can be seen.
		if(IsBurning())
		{
			// Render fire with smoke on burning ships.
			RenderFire(projection, myM, *ari.re, FireRadius());
			RenderSmoke(projection, myM, *ari.re, effectRadius * 1.5f);
		}
		//else if(IsSmoldering())
			//RenderSmoke(projection, myM, *ari.re, effectRadius * 1.5);

		if(HasAcid())
			RenderAcid(projection, myM, *ari.re, effectRadius);

		if(HasDisease())
			RenderDisease(projection, myM, *ari.re, effectRadius);

		if(IsParalyzed())
			RenderParalysis(projection, myM, *ari.re, effectRadius * 1.5f);

		if(IsPoisoned())
			RenderPoison(projection, myM, *ari.re, effectRadius);
	}
}

void DaCreature::RenderScene(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari)
{
	DaCapsule* ship;
	if(IsInsideCapsule())
	{
		ship = FindParentCapsuleInEnv();
		if(ship)
			ari.re->DisableAtomLights(ship->Id());
	}
	else
		ship = 0;

	if(m_Console)
	{
		// Set current ambient light and fog states.
		float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
		bool sunsBlocked;
		SceneRenderParameters(this, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);
		ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);
		ari.re->SetFog(fogR, fogG, fogB, fogDensity);

		// Render the console at which this scene atom is sitting.
		// Don't render anything when sitting at a game table.
		if(!AuStation::IsGameTable(m_Console->Type()))
		{
			// Disable suns when they are blocked at my location. Prepare to restore them.
			QList<int> sunsDisabled;
			if(sunsBlocked)
				ari.re->DisableSuns(sunsDisabled);

			m_Console->Render(projection, farProjection, model, *(DaPerson*)this, this, ari);

			// Restore suns that I disabled.
			ari.re->EnableSuns(sunsDisabled);
		}
	}
	else if((m_Body && m_Body->IsTurretPose()) ||
			(m_Animalia && m_Animalia->IsTurretPose()))
	{
		// Render the sensor lock blip when manning a turret.
		if(IsInSpacecraft())
		{
			if(ship && !ship->LockedBlip().IsNull())
				DpStation::RenderSensorLock(ship->LockedBlip(), ship->LockedStance(), ship->LockedIsCivilian(),
						ship->LockedSysLoc(), model, *this, ari);
		}
	}

	bool renderItemInHand = HasItemInHand() && g_Au.DrawGun() &&
		((m_Body && !m_Body->CurrentGeneralPose().HeadOnly() && !m_Body->IsTurretIntegratedPose()) ||
		((m_Animalia && !m_Animalia->CurrentGeneralPose().HeadOnly() && !m_Animalia->IsTurretIntegratedPose())));
	if(renderItemInHand)
	{
		AuMatrix4x4 tThis(model);
		BodyPoseAdjustTransform(tThis);

		// Render the item in hand. RenderItemInHand already accounts for my CGOffset, if any.
		// Must offset my body model by the CG offset to my body's pivot point, which is where it renders.
		// When riding a mount, make adjustments to my body position to appear to stay exactly positioned at the saddle.
		const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
		if(mount)
		{
			// Determine proper location of mount's saddle.
			AuPoint rideLoc = mount->CGOffset();				// Riders sit at mount's body pivot point.
			float bodyThick = mount->TorsoThickness()/2;		// Allow for mount's body thickness.
			rideLoc += AuPoint(0, -bodyThick, bodyThick);		// Move back and up by mount's body thickness.

			// Creature rider positioning. Location of creature bodies when riding an animal is their waist/pivot point.
			rideLoc.z += TorsoThickness()/2;					// Move up by the rider's body thickness.

			// Save the world transformation matrix so I can restore it on exit.
			AuMatrix4x4 tBody(tThis);
			tBody.Translate(rideLoc - Location());
			RenderItemInHand(projection, tBody, ari, 0, this, 0);
		}
		else
		{
			if(m_RenderedHeadTurn != 0)
				tThis.RotateZ(-m_RenderedHeadTurn);
			RenderItemInHand(projection, tThis, ari, 0, this, 0);
		}
	}
	if(ship)
		ari.re->EnableAtomLights(ship->Id());
}

// Sets up camera when this atom is going to render the scene.
void DaCreature::SetupCamera(AuMatrix4x4& tCamera, double& rNearClip, double& rFarClip, AuAtomRenderInfo& ari) const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return;
	// Set up clipping planes.
	// Far clip must allow for the largest object that gets rendered in the near pass, which is a ringworld arc section plus both
	// of its neighboring sections, ugh.
	rFarClip = DaWorldRingSection::MinimumSectionLength() * 2;

	quint16 remoteCameraId = RemoteCameraId();
	if(remoteCameraId)
	{
		//qDebug() << "DaCreature::SetupCamera" << *this << "camId" << remoteCameraId;
		const DaCapsule* cap = dynamic_cast<const DaCapsule*>(parent);
		if(cap && cap->RemoteCameraVerify(remoteCameraId))
		{
			// Configure for remote camera.
			rNearClip = 3;
			AuMatrix4x4 mRemoteCam;
			mRemoteCam.Translate(cap->RemoteCameraLocation(remoteCameraId));
			mRemoteCam.RotateZ(Rotation().z);
			if(m_Body)
				mRemoteCam.RotateX(PI/2 + m_Body->CurrentHeadTilt());
			else
				mRemoteCam.RotateX(PI/2 + m_Animalia->CurrentHeadTilt());
			tCamera = Transform().Inverse() * mRemoteCam;
			return;
		}
	}

	// If near clip is too small, distant objects merge too much. Too large and the canopy braces of the space
	// space fighter get clipped too much.
	if(dynamic_cast<const DaVehicle*>(parent))
		rNearClip = 2;	// Keeps from clipping too much of the canopy.
	else if(IsProne())
		rNearClip = 1;
	else if(InDesignerNotPreview())
		rNearClip = 1;			// For close in work.
	else
		rNearClip = 3;	// Clouds on ringworld sections at farthest orbit altitude just start to have Z collisions with the ground.

	AuPose gp;
	float headTilt;
	if(m_Body)
	{
		gp = m_Body->CurrentGeneralPose();
		headTilt = m_Body->CurrentHeadTilt();
	}
	else
	{
		gp = m_Animalia->CurrentGeneralPose();
		headTilt = m_Animalia->CurrentHeadTilt();
	}
	if(gp.IsTurretIntegrated())
	{
		const DaSc* sc = dynamic_cast<const DaSc*>(parent);
		if(sc)
		{
			// Aboard ship with new style turrets.
			float baseRot = Rotation().z;
			AuMatrix4x4 mTur;
			sc->TurretSightMatrix(StationId(), baseRot, headTilt, mTur);
			tCamera = Transform().Inverse() * mTur;
			return;
		}
		else
		{
			const DaBd* bd = dynamic_cast<const DaBd*>(parent);
			if(bd)
			{
				// In bd with new style turrets.
				float baseRot = Rotation().z;
				AuMatrix4x4 mTur;
				bd->TurretSightMatrix(StationId(), baseRot, headTilt, mTur);
				tCamera = Transform().Inverse() * mTur;
				return;
			}
		}
	}

	// Move camera to eye position.
	AuPoint eyepos = CameraOffset();
	if(ari.IsViewThirdPerson())
	{
		// Calculate camera position behind/above the eye based on distance and angle.
		float behind = cosf(ari.m_ViewAngle3rd) * ari.m_ViewDist3rd;
		float above = sinf(ari.m_ViewAngle3rd) * ari.m_ViewDist3rd;
		eyepos += AuPoint(0, -behind, above);
	}
	bool constrainPosition = true;
	tCamera.LoadIdentity();
	if(gp.IsDying() || gp.IsDead() || gp.IsBerth() || gp.IsPatientProne())
	{
		// Body is sleeping, dead or dying.
		tCamera.Translate(eyepos);
		int tilDead;
		if(m_Body)
			tilDead = m_Body->CurrentHitPoints() - m_Body->DeadHitPoints();
		else
			tilDead = m_Animalia->CurrentHitPoints() - m_Animalia->DeadHitPoints();
		if(gp.IsDead() || tilDead <= 10)
		{
			// When dead or almost dead the camera departs the body.
			if(tilDead < 0)
				tilDead = 0;
			int camHts[] = {8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8};
			tCamera.Translate(0,0,camHts[tilDead]);
			constrainPosition = false;
		}
		else
		{
			if(ari.IsViewThirdPerson())
			{
				// In 3rd person view, position the camera above the body looking down toward it.
				tCamera.Translate(0,0,1);			// Lift above the centerline of the prone body.
				tCamera.RotateX(PI/2. - ari.m_ViewAngle3rd);	// Look toward head.
			}
			else
			{
				// When sleeping or dying, we rotate the camera to match the body facing.
				switch(gp.Pose())
				{
				case AuPose::BerthBottomFaceDown:
				case AuPose::BerthTopFaceDown:
				case AuPose::WritheFaceDown:	// Rotate camera to body facing direction.
					tCamera.RotateX(PI/2.);	// Tilt chin up instead of face plant.
					break;
				case AuPose::BerthBottomFaceLeft:
				case AuPose::BerthTopFaceLeft:
				case AuPose::WritheFaceLeft:	// Rotate camera to body facing direction.
					tCamera.Translate(0,0,.8);
					tCamera.RotateY(PI/2.);
					break;
				case AuPose::BerthBottomFaceRight:
				case AuPose::BerthTopFaceRight:
				case AuPose::WritheFaceRight:	// Rotate camera to body facing direction.
					tCamera.Translate(0,0,.8);
					tCamera.RotateY(-PI/2.);
					break;
				case AuPose::BerthBottomFaceUp:
				case AuPose::BerthTopFaceUp:
				case AuPose::WritheFaceUp:		// Rotate camera to body facing direction.
				case AuPose::Patient_Prone:
				default:
					tCamera.RotateY(PI);
					break;
				}
			}
		}
	}
	else if(gp == AuPose::ReceptiveMate || gp == AuPose::MateOnBottom)
	{
		if(HasDNA3())
		{
			tCamera.RotateY(PI);
			tCamera.Translate(eyepos);
			tCamera.RotateX(headTilt);

			// Put camera behind head so the clip plane is exactly at the eyes.
			double headThick;
			if(m_Body)
				headThick = m_Body->HeadThickness();
			else
				headThick = m_Animalia->HeadThickness();
			tCamera.Translate(0, 0, rNearClip/2.0 - headThick);

			// Move camera back from the eye position, in the direction they are looking.
			if(ari.IsViewFirstPerson())
			{
				// Calculate camera position behind/above the eye based on distance and angle.
				float behind = cosf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
				float above = sinf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
				tCamera.Translate(0, above, behind);
			}
		}
		else
		{
			// Rotate camera to eye view direction.
			tCamera.Translate(eyepos);
			tCamera.RotateX(165*DEG2RADf + headTilt);

			// Put camera behind head so the clip plane is exactly at the eyes.
			double headThick;
			if(m_Body)
				headThick = m_Body->HeadThickness();
			else
				headThick = m_Animalia->HeadThickness();
			tCamera.Translate(0, 0, rNearClip/2.0 - headThick);

			// Move camera back from the eye position, in the direction they are looking.
			if(ari.IsViewFirstPerson())
			{
				// Calculate camera position behind/above the eye based on distance and angle.
				float behind = cosf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
				float above = sinf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
				tCamera.Translate(0, above, behind);
			}
		}
	}
	else if(gp == AuPose::MateOnTop && HasDNA4())
	{
		// Rotate camera to eye view direction.
		tCamera.Translate(eyepos);
		if(Gender() == Male)
			tCamera.RotateX(headTilt + 20*DEG2RADf);
		else
			tCamera.RotateX(headTilt + 60*DEG2RADf);

		// Put camera behind head so the clip plane is exactly at the eyes.
		double headThick;
		if(m_Body)
			headThick = m_Body->HeadThickness();
		else
			headThick = m_Animalia->HeadThickness();
		tCamera.Translate(0, 0, rNearClip/2.0 - headThick);

		// Move camera back from the eye position, in the direction they are looking.
		if(ari.IsViewFirstPerson())
		{
			// Calculate camera position behind/above the eye based on distance and angle.
			float behind = cosf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
			float above = sinf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
			tCamera.Translate(0, above, behind);
		}
	}
	else
	{
		// Rotate camera to eye view direction.
		tCamera.Translate(eyepos);
		tCamera.RotateX(PI/2. + headTilt);

		// Put camera behind head so the clip plane is exactly at the eyes.
		double headThick;
		if(m_Body)
			headThick = m_Body->HeadThickness();
		else
			headThick = m_Animalia->HeadThickness();
		tCamera.Translate(0, 0, rNearClip/2.0 - headThick);

		// Move camera back from the eye position, in the direction they are looking.
		if(ari.IsViewFirstPerson())
		{
			// Calculate camera position behind/above the eye based on distance and angle.
			float behind = cosf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
			float above = sinf(ari.m_ViewAngle1st) * ari.m_ViewDist1st;
			tCamera.Translate(0, above, behind);
		}
	}

	if(constrainPosition)
	{
		// Insure parent atom is ok with camera position.
		parent->ConstrainCamera(this, tCamera);
	}
}

void DaCreature::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaUnit::CreateDeviceObjects(ari);

	COMMODITYID armor = Armor();
	COMMODITYID helmet = Helmet();
	COMMODITYID wornArmor = WornArmor();
	COMMODITYID wornClothing = WornClothing();
	COMMODITYID wornHat = WornHat();
	float speedEnc = MovementSpeedMod();
	AuPose gp;
	m_BodyLock.lockForWrite();
	if(m_Body)
	{
		gp = m_Body->CurrentGeneralPose();
		m_Body->CreateDeviceObjects(*ari.re, ari.Models(), armor, helmet, speedEnc);
	}
	else
	{
		gp = m_Animalia->CurrentGeneralPose();
		bool bodyChange = m_Animalia->SetHairStyle(m_Haircut, m_Shave);
		if(m_Animalia->SetArmor(wornArmor))
			bodyChange = true;
		if(m_Animalia->SetClothing(wornClothing))
			bodyChange = true;
		if(m_Animalia->SetHat(wornHat))
			bodyChange = true;
		if(m_Animalia->SetGrowthStage(GrowthStage()))
			bodyChange = true;	// Change from juvenile to adult body.
		
		/*
		bool useThread = true;
		if(useThread)
		{
			if(bodyChange)
				mm.DeleteAnimalia(Id());
		*/
			if(bodyChange || !m_Animalia->HasVAOs())
			{
				//qDebug() << "DaCreature::CreateDeviceObjects" << *this << "CreateClothingTex";
				m_Animalia->CreateClothingTex(*ari.re);

				// Get the appropriate VAO from the model maker based on my current hair and clothing.
				ari.mm.Animalia(*ari.re, Id(), *m_Animalia);
			}
		/*
		}
		else
		{
			if(bodyChange)
				m_Animalia->DeleteGL();

			if(bodyChange || !m_Animalia->HasVAOs())
				m_Animalia->CreateClothingTex(re);

			if(!m_Animalia->HasVAOs())
				m_Animalia->CreateAnimalVAO(re);
		}
		*/
	}
	m_BodyLock.unlock();

	if(m_Console)
	{
		// My parent atom must be a spacecraft.
		if(IsInCapsule())
		{
			const DaCapsule* cap = dynamic_cast<const DaCapsule*>(Parent());
			if(cap)
				m_Console->CreateDeviceObjects(ari, *cap, IsInBd());
		}
	}
}

void DaCreature::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaUnit::AnimateDeviceObjects(ari, sceneAtom, lastAnim);
	m_BodyLock.lockForWrite();
	if(m_Animalia)
	{
		m_Animalia->AnimatePose(Id(), lastAnim);
		//AuPoint l = m_Animalia->CurrentRightHandLoc() + CGOffset(*m_Animalia, 0, 1, false);
		//qDebug() << "DaCreature::AnimateDeviceObjects" << *this << "weaponLoc" << l;
	}
	m_BodyLock.unlock();
}

void DaCreature::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaUnit::DeleteDeviceObjects(ari);
	if(m_Body)
		m_Body->DeleteDeviceObjects(*ari.re);
	else
	{
		ari.mm.DeleteAnimalia(Id());
		m_Animalia->DeleteGL();
	}

	if(m_Console)
		m_Console->DeleteDeviceObjects(*ari.re);

	// OpenAL audio sources
	for(int cs=0; cs<CREATURE_SOUNDS; cs++)
	{
		ari.StopSound(m_AudioCreature[cs]);
		m_Sound[cs] = SOUND_None;
	}
}

bool DaCreature::CanSeeBridgeIndicators() const
{
	if(RemoteCameraId())
		return false;
	//Q_ASSERT(g_Au.IsSceneApp());
	if(IsHelmStationPose())
		return true;
	else if(IsAboardVehicle())
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
		switch(vehicle->CommodityId())
		{
		case CID_DirtBike:
		case CID_SailBoat:
		case CID_Submarine:
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
		default:
			break;				// no nav beacon
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
			// Must be in the control seat to see the beacon.
			// Space fighters only have one seat.
			return true;
		case CID_SpaceTransport:
		case CID_SpaceRocket:
		case CID_Helicopter:
			// Must be in the control seat to see the beacon.
			return vehicle->IsControlSeat(VehicleSeatIndex());
		}
	}
	else if(IsInsideSpacecraft())
	{
		QString shipRoom(ShipRoomName());
		if(!shipRoom.isEmpty())
		{
			const DaVessel* ves = FindParentVesselInEnv();
			if(ves)
				return ves->BridgeVisible(shipRoom);
		}
	}
	return false;
}

bool DaCreature::CanSeeOrbitRings() const
{
	if(RemoteCameraId())
		return false;
	if(IsInVehicle())
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
		if(vehicle)
			return vehicle->IsOrbitRingsOn();
	}
	else if(m_Console && m_Console->IsRingsVisible())
		return true;
	else if(IsInsideCapsule())
	{
		QString capRoom(CapsuleRoomName());
		if(!capRoom.isEmpty())
		{
			const DaCapsule* cap = FindParentCapsuleInEnv();
			if(cap)
				return cap->OrbitRingsVisible(capRoom);
		}
	}
	return false;
}

bool DaCreature::CanSeeMoonOrbitRings() const
{
	if(RemoteCameraId())
		return false;
	//Q_ASSERT(g_Au.IsSceneApp());
	if(IsInVehicle())
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
		if(vehicle)
			return vehicle->IsOrbitRingsOn();
	}
	else if(m_Console && m_Console->IsMoonRingsVisible())
		return true;
	else if(IsInsideCapsule())
	{
		QString capRoom(CapsuleRoomName());
		if(!capRoom.isEmpty())
		{
			const DaCapsule* cap = FindParentCapsuleInEnv();
			if(cap)
				return cap->MoonOrbitRingsVisible(capRoom);
		}
	}
	return false;
}

bool DaCreature::CanSeeWormholes() const
{
	if(RemoteCameraId())
		return false;
	//Q_ASSERT(g_Au.IsSceneApp());
	if(m_Console && m_Console->IsWormholesVisible())
		return true;
	else if(IsInsideCapsule())
	{
		QString capRoom(CapsuleRoomName());
		if(!capRoom.isEmpty())
		{
			const DaCapsule* cap = FindParentCapsuleInEnv();
			if(cap)
				return cap->WormholeLinesVisible(capRoom);
		}
	}
	return false;
}

int DaCreature::CanSeeSystemNames() const
{
	if(RemoteCameraId())
		return false;
	//Q_ASSERT(g_Au.IsSceneApp());
	int visible = 0;
	if(m_Console)
	{
		visible = m_Console->SystemNamesVisible();
		if(visible >= 0)
			return visible;
		visible = 0;
	}
	if(IsInsideCapsule())
	{
		QString capRoom(CapsuleRoomName());
		if(!capRoom.isEmpty())
		{
			const DaCapsule* cap = FindParentCapsuleInEnv();
			if(cap)
				visible = cap->SystemNamesVisible(capRoom);
		}
	}
	return visible;
}

bool DaCreature::IsMonitoringSystem(const AuId& idSolarSystem) const
{
	if(!HasConsole())
		return false;

	m_ConsoleLock.lockForRead();
	bool v = m_Console && m_Console->DisplayedSystem() == idSolarSystem;
	m_ConsoleLock.unlock();
	return v;
}

bool DaCreature::IsMonitoringDifferentSystem(const AuId& idSolarSystem) const
{
	if(!HasConsole())
		return false;

	m_ConsoleLock.lockForRead();
	if(m_Console && AuStation::IsNavigation(m_Console->Type()))
	{
		const DpStationNav* pca = (const DpStationNav*)m_Console;
		if(pca->DisplayedSystem() != idSolarSystem)
		{
			m_ConsoleLock.unlock();
			return true;
		}
	}
	m_ConsoleLock.unlock();
	return false;
}

DaAtom* DaCreature::SightTargetInSight(AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield) const
{
	if(IsTurretIntegratedPose())
	{
		const DaSc* sc = dynamic_cast<const DaSc*>(Parent());
		if(sc)
		{
			float baseRot = Rotation().z;
			float gunRot = CurrentHeadTilt();
			AuMatrix4x4 mTur;
			sc->TurretMuzzleMatrix(StationId(), baseRot, gunRot, mTur);
			AuPoint muzzleLoc = mTur.Translation();							// Relative to the ship.
			AuPoint lookVec = mTur.ProjectVector(0,0,1);
			//muzzleLoc += lookVec * (2*c_FeetPerMeter);
			AuPoint muzzleSight = muzzleLoc + lookVec * c_CreatureSightRange;
			AuPoint eyeLoc = Transform().UnProject(muzzleLoc);							// Relative to me.
			AuPoint lookLoc = Transform().UnProject(muzzleSight);
			return DaUnit::SightTarget(sightedLoc, sightedPart, sightedShield, eyeLoc, lookLoc);
		}
	}

	AuPoint eyeLoc = CameraOffset();
	AuPoint lookVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());
	if(CurrentGeneralPose() == AuPose::ReceptiveMate || CurrentGeneralPose() == AuPose::MateOnBottom)
	{
		if(HasDNA3())
		{
			eyeLoc = eyeLoc.RotateY(PI);
			lookVec = lookVec.RotateY(PI);
		}
		lookVec = lookVec.RotateX(PI/2);
	}
	AuPoint lookLoc = eyeLoc + lookVec * c_CreatureSightRange;
	return DaUnit::SightTarget(sightedLoc, sightedPart, sightedShield, eyeLoc, lookLoc);
}

bool DaCreature::SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
		unsigned int& sightedPart, bool& sightedShield) const
{
	// Get body transformation matrix.
	AuMatrix4x4 tBody(tModel);
	BodyPoseAdjustTransform(tBody);
	tBody.Translate(CGOffset());

	// Project the sight segment into the space of this atom.
	AuPoint oEyeLoc = tBody.UnProject(eyeLoc);
	AuPoint oSightedLoc = tBody.UnProject(sightedLoc);

	// If we clipped the sight segment, project the end point back into the space of the scene atom.
	// 'sightedPart' is set to the bone that was hit.
	m_BodyLock.lockForRead();
	bool hitMe;
	if(m_Body)
		hitMe = m_Body->SightTargetDetails(oEyeLoc, oSightedLoc, sightedPart);
	else if(m_Animalia)
		hitMe = m_Animalia->SightTargetDetails(oEyeLoc, oSightedLoc, sightedPart);
	else
		hitMe = false;
	m_BodyLock.unlock();
	if(hitMe)
	{
		sightedLoc = tBody.Project(oSightedLoc);
		sightedShield = false;
	}
	return hitMe;
}

bool DaCreature::RoughExtents(AuBox &fExt) const
{
	AuPoint min, max;
	m_BodyLock.lockForRead();
	if(m_Body)
		m_Body->RoughExtents(min, max);
	else
		m_Animalia->SightExtents(min, max);
	m_BodyLock.unlock();

	if(!IsFullGrown())
	{
		// Scale body extents by growth factor.
		AuPoint scale = GrowthScaleBody();
		min.x *= scale.x;
		min.y *= scale.y;
		min.z *= scale.z;
		max.x *= scale.x;
		max.y *= scale.y;
		max.z *= scale.z;
	}

	fExt.SetFromMinMax(min, max);

	// Get body transformation matrix.
	AuMatrix4x4 tBody;
	BodyPoseAdjustTransform(tBody);
	tBody.Translate(CGOffset());

	// Project the result.
	fExt.Transform(tBody);

	return true;
}

bool DaCreature::CanFly() const
{
	// Can this critter fly?
	m_BodyLock.lockForRead();
	bool fly;
	if(m_Body)
		fly = m_Body->DNA().CanFly();
	else
		fly = m_Animalia->IsFlyer();
	m_BodyLock.unlock();
	if(!fly)
		return false;

	// We have environment information.
	// Can the atmosphere support flying creatures? We'll be generous here.
	if(AirDensity() < .15f)
		return false;	// Not enough air or outside atmosphere.
	if(InDesignerNotPreview())
		return false;

	// Cannot fly when health is below 50%.
	float curhp = CurrentHitPoints();
	float maxhp = MaxHitPoints();
	float frac = curhp/maxhp;
	if(frac < .5f)
		return false;

	return true;
}

bool DaCreature::CanHover() const
{
	// Only certain types of wings can hover.
	m_BodyLock.lockForRead();
	bool hov;
	if(m_Body)
		hov = m_Body->DNA().ArmType == DhDNA::ARM_WingDragonFly || m_Body->DNA().ArmType == DhDNA::ARM_WingFairy;
	else
		hov = m_Animalia->CanHover();
	m_BodyLock.unlock();
	if(!hov)
		return false;

	return CanFly();
}

bool DaCreature::BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
		int& ammoFull) const
{
	loc = AuDNABase::WL_None;
	ammoCur = 0;
	ammoFull = 0;

	// Target range is measured from my cg. Body considers everything measured from the body pivot point.
	// Add the 2D distance, from my body's pivot point to my loc, to the target range. This is a close approximation of the
	// additional range.
	float cgOffset = CGOffset().Length_2D();
	float targetRangeToBody = targetRange + cgOffset;

	bool specWeapon;
	if(!IsFullGrown())
	{
		m_BodyLock.lockForRead();
		if(m_Body)
			specWeapon = m_Body->BestWeaponAtRange(targetRangeToBody, loc, weapon, false);
		else
			specWeapon = false;	//DHH animalia
		m_BodyLock.unlock();

		// Scale damage effects according to growth factor.
		int growTime = GrowthTime();
		if(weapon.m_w1.m_Damage)
			weapon.m_w1.m_Damage = weapon.m_w1.m_Damage * (growTime - (m_MinutesToFullGrown - 1)) / growTime;
		if(weapon.m_w2.m_Damage)
			weapon.m_w2.m_Damage = weapon.m_w2.m_Damage * (growTime - (m_MinutesToFullGrown - 1)) / growTime;
	}
	else
	{
		m_BodyLock.lockForRead();
		if(m_Body)
			specWeapon = m_Body->BestWeaponAtRange(targetRangeToBody, loc, weapon, true);
		else
			specWeapon = false;	//DHH animalia
		m_BodyLock.unlock();
	}

	// Contact weapons must reduce the range as it was measured from my body's cg location.
	if(weapon.m_Mode == WM_Contact)
	{
		// Allow a minimum reach range for contact weapons. This is measured from my eye.
		weapon.m_Range -= cgOffset;
		if(weapon.m_Range < 6)
			weapon.m_Range = 6;
	}
	else if(weapon.m_Mode == WM_Projectile)
	{
		// Add a little reach range to the range of projectile weapons. This is measured from my eye.
		weapon.m_Range += 6;
	}

	if(IsUnderGround() && !IsInsideBd())
	{
		// Creature is under ground and not inside a room of a building. Only psionic weapons can be used under ground.
		if(weapon.m_Mode != WM_Psionic)
		{
			specWeapon = false;
			weapon.m_w1.m_Effect = WE_None;
			weapon.m_w2.m_Effect = WE_None;
		}
	}
	
	// Select appropriate special CID_ for body location.
	COMMODITYID bodycids[] =
	{
		CID_Invalid,		// WL_None
		CID_My_Anus,		// WL_Anus
		CID_My_Eyes,		// WL_Eyes
		CID_My_Feet,		// WL_Feet
		CID_My_Hands,		// WL_Hands
		CID_My_Head,		// WL_Head
		CID_My_Tail,		// WL_Tail
		CID_My_Torso,		// WL_Torso
	};
	weapon.m_CID = bodycids[loc];

	// Special weapon quality is the weapon size factor.
	if(specWeapon)
	{
		int wpnStep = (255 - QualityBaseline()) / 7;	// seven steps above weaponsize 0
		m_BodyLock.lockForRead();
		if(m_Body)
			weapon.m_Quality = QualityBaseline() + m_Body->DNA().WeaponSize * wpnStep;
		else
			weapon.m_Quality = 1;	//DHH animalia
		m_BodyLock.unlock();
	}
	else
		weapon.m_Quality = QualityBaseline();

	return specWeapon;
}

void DaCreature::WeaponAtUsePos(const AuGearPos& pos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const
{
	INVENTORYPOS wpos = pos.MountBodyWeaponPos();
	if(wpos != IP_NumInventoryPos)
	{
		// Weapon gear position is that of the animal I am riding.
		DaAnimal* mount = dynamic_cast<DaAnimal*>(Parent());
		if(mount)
		{
			// Body weapon of mounted animal. We do not control the body weapons of mounted people.
			// Calculate inventory position relative to the mount.
			mount->WeaponAtUsePos(AuGearPos(wpos), sub, weapon, ammoCur, ammoFull);
			weapon.m_CID = (COMMODITYID)pos.BodyPos();	// Use CID_ value to indicate mount's body part. 1:1 correspondence to IP_.
			//qDebug("DaCreature::WeaponInfo %d %s mount weapon pos %d mountpos %d sub %d mode %d effect1 %d effect2 %d",
					//Id().GetID(), (const char*)Name().toAscii(), pos, mountPos, sub, mode, effect1, effect2);
			return;
		}
	}

	wpos = pos.PersonalBodyWeaponPos();
	if(wpos != IP_NumInventoryPos)
	{
		// Weapon gear position is that of my body.
		weapon.m_CID = (COMMODITYID)pos.BodyPos();	// Use CID_ value to indicate my body part. 1:1 correspondence to IP_.
		ammoCur = 0;
		ammoFull = 0;
		AuDNABase::WEAPONLOC loc = (AuDNABase::WEAPONLOC)(wpos - IP_My_Anus + 1);
		// 'sub' IPS_Item is physical attack.
		// 'sub' IPS_Weapon1 is special attack.
		// Special weapon cannot be used by growing embryo.
		bool useSpecialWeapon = (sub == IPS_Item || !IsFullGrown()) ? false : true;
		m_BodyLock.lockForRead();
		if(m_Body)
			m_Body->WeaponAtPos(loc, useSpecialWeapon, weapon);
		else
			m_Animalia->WeaponAtPos(loc, useSpecialWeapon, weapon);
		m_BodyLock.unlock();

		// Contact weapons must reduce the range as it was measured from my body's cg location.
		if(weapon.m_Mode == WM_Contact)
		{
			// Target range is measured from my eye. Body considers everything measured from the body pivot point.
			// Allow a minimum reach range for contact weapons. This is measured from my eye.
			float cgOffset = CGOffset().Length_2D();
			weapon.m_Range -= cgOffset;
			if(weapon.m_Range < 6)
				weapon.m_Range = 6;
		}
		else if(weapon.m_Mode == WM_Projectile)
		{
			// Add a little reach range to the range of projectile weapons. This is measured from my eye.
			weapon.m_Range += 6;
		}
		//qDebug() << "DaCreature::WeaponAtUsePos" << Id() << Name() << "range" << weapon.m_Range;

		// Special weapon quality is the weapon size factor.
		if(useSpecialWeapon)
		{
			int wpnStep = (255 - QualityBaseline()) / 7;	// seven steps above weaponsize 0
			m_BodyLock.lockForRead();
			if(m_Body)
				weapon.m_Quality = QualityBaseline() + m_Body->DNA().WeaponSize * wpnStep;
			else
				weapon.m_Quality = 1;	//DHH animalia
			m_BodyLock.unlock();
		}
		else
			weapon.m_Quality = QualityBaseline();

		// Scale damage effects according to growth factor.
		if(!IsFullGrown())
		{
			int growTime = GrowthTime();
			if(weapon.m_w1.m_Damage)
				weapon.m_w1.m_Damage = weapon.m_w1.m_Damage * (growTime - (m_MinutesToFullGrown - 1)) / growTime;
			if(weapon.m_w2.m_Damage)
				weapon.m_w2.m_Damage = weapon.m_w2.m_Damage * (growTime - (m_MinutesToFullGrown - 1)) / growTime;
		}
		return;
	}

	// Creatures do not carry any inventory items. Set null values.
	weapon = AuCombatWeapon(WM_Not);
	ammoCur = 0;
	ammoFull = 0;
}

void DaCreature::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	// Do not accept additional damage if my body is already dead.
	if(IsBodyDead() || !IsNormal())
		return;

	DaUnit::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);

	if(result.m_Damage1 || result.m_Damage2)
	{
		// When a creature is growing from an emerged implant, scale everything about it, including its actual and max hit points.
		if(!IsFullGrown())
		{
			// This creature's reported current and maximum hits have been scaled down according to its growth scale.
			// We must scale UP the damage here, so it does the appropriate proportion of damage to my body.
			int growTime = GrowthTime();
			if(result.m_Damage1)
				result.m_Damage1 = result.m_Damage1 * growTime / (growTime - (m_MinutesToFullGrown - 1));
			if(result.m_Damage2)
				result.m_Damage2 = result.m_Damage2 * growTime / (growTime - (m_MinutesToFullGrown - 1));
		}

		// Record damage dealt by atom into this unit's damage log.
		if(!attacker.m_Id.IsNull() || weapon.m_CID != CID_Invalid)
		{
			quint32 damageDealt = result.m_Damage1 + result.m_Damage2;
			if(damageDealt)
				RecordDamage(attacker.m_Id, damageDealt, weapon.m_CID, weapon.m_Quality);
		}

		// Calculate where my hit points will be after deducting the damage.
		int curHits = CurrentHitPoints();
		bool invincible = (AgentBits() & (AGENT_Entity|AGENT_Invincible)) && weapon.m_w1.m_Effect != WE_Bleeding;
		if(!invincible)
		{
			if(result.m_Damage1)
				curHits -= result.m_Damage1;
			if(result.m_Damage2)
				curHits -= result.m_Damage2;
		}

		int deadHits;
		if(m_Body)
			deadHits = m_Body->DeadHitPoints();
		else
			deadHits = m_Animalia->DeadHitPoints();

		if(curHits <= 0)
		{
			if(CurrentBodyHitPoints() > 0)
			{
				// Body has fallen but has not died yet.
				// Body was still conscious on entry to this function but it is unconscious now.
				// Fall off of my mount or unenclosed vehicle.
				if(IsRider())
					DismountCreature();							// Body falls from creature no matter what.
				else if(IsOnVehicle())
					DismountVehicle();							// Body stays on vehicle if exit location is blocked.

				// Don't allow a live body to be blown to bits. It looks odd.
				// Limit the minimum to half way between unconscious and dead.
				if(curHits < deadHits/2)
					curHits = deadHits/2;
			}

			// Randomly pick a facing for the dying pose.
			if((weapon.m_CID != CID_InternalInjuries || !IsBodyDying() || IsAnyConditionVisible()) && !IsRider())
			{
				SetDyingPose();
				ForceMotion();
			}
		}

		// Pose the body for dead or dying state.
		if(curHits <= deadHits)
		{
			// This creature's body has died.
			result.m_IsDestroyed = true;
			curHits = deadHits;
			AuId killerId = KillerId();	// Award the kill to the atom that did the most damage; this could be a spaceship.
			if(!killerId.IsNull())
			{
				DaAtom* system = FindEnvironmentAtom();
				if(system)
				{
					// Locate the atom that killed me.
					DaAtom* killer = system->FindIdRecursive(killerId);
					if(killer)
						killer->UnitKilled(this);
				}
			}

			// Fall off of my mount or unenclosed vehicle.
			if(IsRider())
				DismountCreature();							// Body falls from creature no matter what.
			else if(IsOnVehicle())
				DismountVehicle();							// Body stays on vehicle if exit location is blocked.

			// Pick a dead facing that matches the dying pose, if it was dying before entry to this function anyway.
			SetDeadPose();
		}

		if(!result.m_IsDestroyed && weapon.m_CID != CID_MedicalProcedure && !invincible)
		{
			// This creature was not killed by the damage inflicted.
			// The damage was not inflicted by a medical procedure.
			if((weapon.m_w1.m_Effect == WE_Poison && result.m_Damage1) ||
					(weapon.m_w2.m_Effect == WE_Poison && result.m_Damage2))
			{
				// Poison damage may induce a poisoned condition on this creature.
				if(weapon.m_CID != CID_Poison && !IsPoisoned())
				{
					// The damage was not from an existing poisoned condition.
					// This creature got poisoned.
					// Strength of the poison is determined by total unabsorbed strength of the poison damage.
					int poisonStrength;
					if(weapon.m_w1.m_Effect == WE_Poison)
						poisonStrength = result.m_Damage1;
					else
						poisonStrength = result.m_Damage2;

					// Add the poison as a condition on this creature.
					//qDebug() << Id() << Name() << "poisoned, strength" << poisonStrength;
					AuConPoison* poison = new AuConPoison(poisonStrength, weapon.m_Quality, AuTime::Now());
					AddCondition(poison);
				}
			}
			if((result.m_Damage1 && weapon.IsIncendiary1()) || (result.m_Damage2 && weapon.IsIncendiary2()))
			{
				// Fire damage may induce a fire condition on this creature.
				if(weapon.m_CID != CID_Fire && !IsBurning())
				{
					// The damage was not from an existing fire condition.
					// This creature got burned.
					// 10% chance to add fire as a condition on this creature.
					if(g_Au.Dice().Roll(1,100) <= 10)
					{
						AuConFire* fire = new AuConFire(AuTime::Now());
						AddCondition(fire);
					}
				}
			}
			if((weapon.m_w1.m_Effect == WE_Disease && result.m_Damage1) ||
					(weapon.m_w2.m_Effect == WE_Disease && result.m_Damage2))
			{
				// Disease damage may induce a diseased condition on this creature.
				if(weapon.m_CID != CID_Disease && !attacker.m_Id.IsNull() && !HasDisease())
				{
					// The damage was not from an existing disease condition.
					// This creature got diseased.
					// Strength of the diseased is determined by total unabsorbed strength of the disease damage.
					int diseaseStrength;
					if(weapon.m_w1.m_Effect == WE_Disease)
						diseaseStrength = result.m_Damage1;
					else
						diseaseStrength = result.m_Damage2;

					// Determine contagion level of disease.
					quint8 contagion;
					if(weapon.m_CID < CID_Invalid)
					{
						// Disease must have come from a creature.
						const DaAtom* system = FindEnvironmentAtom();
						Q_ASSERT(system);
						const DaCreature* host = dynamic_cast<const DaCreature*>(system->FindIdRecursive(attacker.m_Id));
						if(host)
						{
							if(host->HasDNA3())
							{
								switch(host->DNA3().WeaponMode)
								{
								case DhDNA::DWM_Area:
									contagion = 96 + weapon.m_Quality;		// Range is 96..127
									break;
								case DhDNA::DWM_Projectile:
									contagion = 64 + weapon.m_Quality;		// Range is 64..95
									break;
								case DhDNA::DWM_Contact:
									contagion = 32 + weapon.m_Quality;		// Range is 32..63
									break;
								case DhDNA::DWM_Psionic:
									contagion = weapon.m_Quality;			// Range is 0..31
									break;
								default:
									contagion = 0;
									break;
								}
							}
							else
							{
								//DHH animalia
								contagion = 0;
							}
						}
						else
							contagion = weapon.m_Quality;	// This should never happen.
					}
					else
					{
						// Disease may have come from an antitoxin.
						contagion = 16 + weapon.m_Quality*3;		// Range is 16..109, stride three.
					}

					// Don't give the same disease to a creature more than once.
					//if(!HasDisease(diseaseStrength, contagion, weapon.m_Quality))
					{
						// Add the disease as a condition on this creature.
						//qDebug() << Id() << Name() << "diseased, strength" << diseaseStrength;
						AuConDisease* disease = new AuConDisease(diseaseStrength, contagion, weapon.m_Quality, AuTime::Now());
						AddCondition(disease);
					}
				}
			}
			if((weapon.m_w1.m_Effect == WE_Acid && result.m_Damage1) || (weapon.m_w2.m_Effect == WE_Acid && result.m_Damage2))
			{
				// Acid damage may induce an acid condition on this creature.
				if(weapon.m_CID != CID_Acid && !HasAcid())
				{
					// The damage was not from an existing acid condition.
					// This creature got corroded.
					// Strength of the acid is determined by total unabsorbed strength of the acid damage.
					int acidStrength;
					if(weapon.m_w1.m_Effect == WE_Acid)
						acidStrength = result.m_Damage1;
					else
						acidStrength = result.m_Damage2;

					// Add the acid as a condition on this creature.
					//qDebug() << Id() << Name() << "corroded with acid, strength" << acidStrength;
					AuConAcid* acid = new AuConAcid(acidStrength, weapon.m_Quality, AuTime::Now());
					AddCondition(acid);
				}
			}
			if(((weapon.m_w1.m_Effect == WE_Stun && result.m_Damage1) ||
						(weapon.m_w2.m_Effect == WE_Stun && result.m_Damage2)) && !IsParalyzed())
			{
				// Paralysis damage may induce a paralyzed condition on this creature.
				// This creature got paralyzed.
				// Strength of the paralysis is determined by total unabsorbed strength of the paralysis damage.
				int paralysisStrength;
				if(weapon.m_w1.m_Effect == WE_Stun)
					paralysisStrength = result.m_Damage1;
				else
					paralysisStrength = result.m_Damage2;

				// Add the disease as a condition on this creature.
				//qDebug() << Id() << Name() << "paralyzed, strength" << paralysisStrength;
				AuConParalysis* paralysis = new AuConParalysis(paralysisStrength, weapon.m_Quality, AuTime::Now());
				AddCondition(paralysis);
			}
			if((weapon.m_w1.m_Effect == WE_Implant && result.m_Damage1) || (weapon.m_w2.m_Effect == WE_Implant && result.m_Damage2))
			{
				// Implant damage may induce an implanted condition on this creature.
				if(weapon.m_CID != CID_ImplantBirth && weapon.m_CID != CID_GivingBirth)
				{
					// The damage was not from an existing implant being born.
					const DaAtom* system = FindEnvironmentAtom();
					Q_ASSERT(system);
					const DaCreature* implanter = dynamic_cast<const DaCreature*>(system->FindIdRecursive(attacker.m_Id));
					if(implanter)
					{
						// Implant only makes sense if the implanter is a creature so we can get its DNA.
						// This creature got implanted.
						// Add the implant as a condition on this creature.
						//qDebug() << Id() << Name() << "implanted, DNA" << implanter->DNA().Description();
						AuConImplantSpecies* implant = new AuConImplantSpecies(implanter->Species(), AuTime::Now());
						AddCondition(implant);

						const DaPerson* person = dynamic_cast<const DaPerson*>(implanter);
						if(person)
						{
							QString text(tr("%1 implanted into %2.").arg(implant->Name()).arg(Name()));
							AuCommThought msg(text, person->Id(), person->Name());
							g_Au.SendAtomChat(msg);
						}
					}
				}
			}
		}
		SetHitPoints(curHits);
		SetModified();
	}
	else
	{
		// Randomly pick a new facing for the dying pose.
		if(IsBodyDying() && !IsRider())
		{
			SetDyingPose();
			ForceMotion();
		}
	}

	// Creatures aren't set into motion when seated at a console, climbing ladders, in a vehicle, or riding a creature.
	if(!IsLockedPose() && !IsLadderPose() && !IsAboardVehicle() && !IsRider())
	{
		// Kick this object into motion.
		if(Mass() > 0 && wpnLoc != hitLoc)
		{
			// Calculate movement based on kinetic force applied.
			double movement = result.Armor1() + result.Armor2();
			if(IsKineticEffect(weapon.m_w1.m_Effect))
				movement += result.m_Damage1;
			if(IsKineticEffect(weapon.m_w2.m_Effect))
				movement += result.m_Damage2;
			if(movement > 0)
			{
				movement *= KineticDamageFtLbsPerSec() / Mass();
				movement *= 2;
				AuPoint fromDir = wpnLoc - hitLoc;
				fromDir.Unit(movement);

				ForceVector(Vector() - fromDir);
				SetModified();
				TickTock();	// Get a thread to process me right away, so my body doesn't hang dead in the air.
			}
		}
	}
}

void DaCreature::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	DaUnit::CombatArmorEffectiveness(mode, effect, hitShield, wpnLoc, hitLoc, baseAbsorb, baseBlock, auxAbsorb, auxBlock);

	// My own skin can be treated like leather clothing.
	m_BodyLock.lockForRead();
	if(m_Animalia)
	{
		float skinAbsorb, skinBlock;
		AuItemDNA mySkin(&m_Animalia->DNA(), m_Animalia->Chromatid(), true, false, m_Animalia->Handedness(), GrowthStage(),
				Haircut(), Shave(), CID_LeatherClothes, QualityBaseline(), 0, 1);
		mySkin.CombatArmorEffectiveness(mode, effect, skinAbsorb, skinBlock);
		baseAbsorb += skinAbsorb;
		baseBlock += skinBlock;
	}
	else if(m_Body)
	{
		float skinAbsorb, skinBlock;
		HANDEDNESS hand = m_Body->IsLeftHanded() ? LeftHanded : RightHanded;
		AuItemDNA mySkin(&m_Body->DNA(), m_Body->Chromatid(), true, false, hand, GrowthStage(), Haircut(), Shave(),
				CID_LeatherClothes, QualityBaseline(), 0, 1);
		mySkin.CombatArmorEffectiveness(mode, effect, skinAbsorb, skinBlock);
		baseAbsorb += skinAbsorb;
		baseBlock += skinBlock;
	}
	m_BodyLock.unlock();
}

void DaCreature::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	if(!al)
		return;
	// Adjust gain based on body volume.
	// A square root is used due to the huge size of large creatures.
	// This factor produces a gain of 3.674234614/3 = 1.224744871 for the 
	// nominal human, which has a body volume of 13.5;
	float vol = Volume();

	const DaCreature* rider = dynamic_cast<const DaCreature*>(Rider());
	if(rider)
		vol += rider->Volume();

	float gain = sqrtf(vol) / 2.f * RelativeAudioGain(sceneAtom) * fade;

	SOUND_EFFECT sound1 = SOUND_None;
	AuPoint epos1;	// Sound emission location
	float gain1 = gain;
	SOUND_EFFECT sound2 = SOUND_None;
	AuPoint epos2;	// Sound emission location
	float gain2 = gain;
	SOUND_EFFECT sound3 = SOUND_None;
	AuPoint epos3;	// Sound emission location
	float gain3 = gain;
	float pitch = 1;

	//if(!IsInVacuum())
	{
		// On a ground surface.
		//  and
		// Not in a vacuum.
		bool isFlying = false;
		bool isDigging = false;
		bool isWalking = false;
		bool isDragging = false;
		bool isSwimming = false;
		float speed = 1;
		AuPose gp = CurrentGeneralPose();
		switch(gp.Pose())
		{
		case AuPose::Init:
		case AuPose::Spawn:
		case AuPose::Beckon:
		case AuPose::Bite:
		case AuPose::Block:
		case AuPose::BerthBottomFaceDown:
		case AuPose::BerthTopFaceDown:
		case AuPose::GameTableSeat0_0:
		case AuPose::GameTableSeat1_0:
		case AuPose::GameTableSeat2_0:
		case AuPose::GameTableSeat3_0:
		case AuPose::GameTableSeat0_45:
		case AuPose::GameTableSeat1_45:
		case AuPose::GameTableSeat2_45:
		case AuPose::GameTableSeat3_45:
		case AuPose::ConsoleCommand:
		case AuPose::ConsoleNav:
		case AuPose::ConsoleEngine:
		case AuPose::ConsoleFireControl:
		case AuPose::ConsoleSurgery:
		case AuPose::ConsolePilot:
		case AuPose::ConsolePower:
		case AuPose::ConsoleSensor:
		case AuPose::ConsoleShield:
		case AuPose::ConsoleTransporter:
		case AuPose::ConsoleSurgeryUnit:
		case AuPose::ConsoleTurret0:
		case AuPose::ConsoleTurret45:
		case AuPose::ConsoleTurret90:
		case AuPose::ConsoleTurret135:
		case AuPose::ConsoleTurret180:
		case AuPose::ConsoleTurret225:
		case AuPose::ConsoleTurret270:
		case AuPose::ConsoleTurret315:
		case AuPose::ConsoleTurretBottom:
		case AuPose::ConsoleTurretTop:
		case AuPose::Crouch:
		case AuPose::DeadFaceDown:
		case AuPose::Eat:
		case AuPose::EyesOnly:
		case AuPose::Glide:
		case AuPose::Jump:
		case AuPose::Kick:
		case AuPose::Ladder:
		case AuPose::Lash:
		case AuPose::Point:
		case AuPose::Prone:
		case AuPose::Psionic:
		case AuPose::SitOnGround:
		case AuPose::Stand:
		case AuPose::Stomp:
		case AuPose::Straddle:
		case AuPose::MateOnTop:
		case AuPose::MateOnBottom:
		case AuPose::Strike:
		case AuPose::Throw:
		case AuPose::TreadWater:
		case AuPose::Upright:
		case AuPose::Wag:
		case AuPose::Wave:
		case AuPose::WritheFaceDown:
		case AuPose::DeadFaceLeft:
		case AuPose::WritheFaceLeft:
		case AuPose::BerthBottomFaceLeft:
		case AuPose::BerthTopFaceLeft:
		case AuPose::DeadFaceRight:
		case AuPose::WritheFaceRight:
		case AuPose::BerthBottomFaceRight:
		case AuPose::BerthTopFaceRight:
		case AuPose::DeadFaceUp:
		case AuPose::WritheFaceUp:
		case AuPose::ReceptiveMate:
		case AuPose::BerthBottomFaceUp:
		case AuPose::BerthTopFaceUp:
		case AuPose::Command_Sit:
		case AuPose::Command_SitStationary:
		case AuPose::Command_Stand:
		case AuPose::Command_StandStationary:
		case AuPose::Engine_Sit:
		case AuPose::Engine_SitStationary:
		case AuPose::Engine_Stand:
		case AuPose::Engine_StandStationary:
		case AuPose::FireControl_Sit:
		case AuPose::FireControl_SitStationary:
		case AuPose::FireControl_Stand:
		case AuPose::FireControl_StandStationary:
		case AuPose::Helm_Sit:
		case AuPose::Helm_SitStationary:
		case AuPose::Helm_Stand:
		case AuPose::Helm_StandStationary:
		case AuPose::Hug:
		case AuPose::Medic_Sit:
		case AuPose::Medic_SitStationary:
		case AuPose::Medic_Stand:
		case AuPose::Medic_StandStationary:
		case AuPose::Nav_Sit:
		case AuPose::Nav_SitStationary:
		case AuPose::Nav_Stand:
		case AuPose::Nav_StandStationary:
		case AuPose::Patient_Sit:
		case AuPose::Patient_SitStationary:
		case AuPose::Patient_Stand:
		case AuPose::Patient_StandStationary:
		case AuPose::Patient_Prone:
		case AuPose::Post_Sit:
		case AuPose::Post_Stand:
		case AuPose::Power_Sit:
		case AuPose::Power_SitStationary:
		case AuPose::Power_Stand:
		case AuPose::Power_StandStationary:
		case AuPose::Sensor_Sit:
		case AuPose::Sensor_SitStationary:
		case AuPose::Sensor_Stand:
		case AuPose::Sensor_StandStationary:
		case AuPose::Shield_Sit:
		case AuPose::Shield_SitStationary:
		case AuPose::Shield_Stand:
		case AuPose::Shield_StandStationary:
		case AuPose::Transporter_Sit:
		case AuPose::Transporter_SitStationary:
		case AuPose::Transporter_Stand:
		case AuPose::Transporter_StandStationary:
		case AuPose::Turret_Sit:
		case AuPose::Turret_SitStationary:
		case AuPose::Turret_SitSwivel3D:
		case AuPose::Turret_Stand:
		case AuPose::Turret_StandStationary:
		case AuPose::MouthPursed:
		case AuPose::MouthClosed:
		case AuPose::MouthOpen:
		case AuPose::MouthAgape:
		case AuPose::MouthOpenWide:
		case AuPose::MouthYawn:
		case AuPose::MouthBite:
		case AuPose::MouthLaugh:
		case AuPose::MouthYap:
		case AuPose::MouthWhisper:
		case AuPose::MouthTalk:
		case AuPose::MouthJabber:
		case AuPose::MouthYell:
		case AuPose::MouthSmile:
		case AuPose::MouthGrin:
		case AuPose::MouthSad:
		case AuPose::MouthFrown:
		case AuPose::MouthCry:
		case AuPose::MouthPerplexed:
		case AuPose::MouthGrim:
		case AuPose::MouthSardonic:
			break;
		case AuPose::CrouchCreep:
		case AuPose::LadderClimb:
			if(IsOnGround())
			{
				isWalking = true;
				speed = .5f;
			}
			break;
		case AuPose::SitOnGroundCreep:
			if(IsOnGround())
			{
				isWalking = true;
				speed = .3f;
			}
			break;
		case AuPose::Dig:
			if(IsOnGround())
			{
				isWalking = true;
				isDigging = true;
				speed = 1;
			}
			break;
		case AuPose::Fly:
		case AuPose::Hover:
			isFlying = true;
			speed = .75f;
			break;
		case AuPose::FlyFast:
			isFlying = true;
			speed = 1;
			break;
		case AuPose::ProneCreep:
			if(IsOnGround())
			{
				isDragging = true;
				speed = 1;
			}
			break;
		case AuPose::StandRun:
			if(IsOnGround())
			{
				isWalking = true;
				speed = 2;
			}
			break;
		case AuPose::StandWalk:
			if(IsOnGround())
			{
				isWalking = true;
			}
			break;
		case AuPose::Swim:
		case AuPose::SwimFast:
			if(!IsEyeSubmerged() || SeaDepth() < 2)
			{
				// Only make swimming noises when near the surface.
				isSwimming = true;
			}
			break;
		}

		// Walkers will be dragging if they have no arms or legs.
		if(isWalking)
		{
			// Determine number of walking pairs of limbs and if body drags.
			m_BodyLock.lockForRead();
			int walkingPairs;
			if(m_Body)
			{
				walkingPairs = m_Body->DNA().LegPairs;
				if(!walkingPairs)
					isDragging = true;	// must drag body if no legs
				if(m_Body->DNA().WalksOnHands())
					walkingPairs += m_Body->DNA().ArmPairs;
			}
			else
			{
				walkingPairs = 1;	//DHH animalia
			}
			m_BodyLock.unlock();
			if(!walkingPairs)
				isDragging = true;	// must drag body if no arms or legs

			// First walking pair makes sound1 noise at ground level.
			if(walkingPairs)
			{
				sound1 = (SOUND_EFFECT)(OnGroundSound() - 1 + SOUND_FootstepsForest);
				epos1 = model.Project(AuPoint(0,0,-GroundHeightAboveCG()));
			}

			// Additional walking pair makes sound2 noise.
			if(walkingPairs > 1)
			{
				sound2 = (SOUND_EFFECT)(OnGroundSound() - 1 + SOUND_FootstepsForest);
				epos2 = epos1;
			}
		}
		else if(isSwimming)
		{
			// Determine number of swimming pairs of limbs.
			m_BodyLock.lockForRead();
			int swimmingPairs;
			if(m_Body)
				swimmingPairs = m_Body->DNA().LegPairs + m_Body->DNA().ArmPairs;
			else
				swimmingPairs = 1;	//DHH animalia
			m_BodyLock.unlock();

			// First swimming pair makes sound1 noise.
			if(swimmingPairs)
			{
				sound1 = SOUND_FootstepsWater;
				epos1 = model.Project(CGOffset());	// at body pivot point
			}

			// Additional swimming pair makes sound2 noise.
			if(swimmingPairs > 1)
			{
				sound2 = SOUND_FootstepsWater;
				epos2 = epos1;
			}
		}

		// Body drag makes drag noise.
		if(isDragging)
		{
			sound3 = (SOUND_EFFECT)(OnGroundSound() - 1 + SOUND_DragForest);
			if(sound3 == SOUND_DragForest)
				gain3 /= 2;										// Forest dragging sound is particularly loud.
			epos3 = model.Project(CGOffset());					// Sound emits from body pivot point.
			if(sceneAtom == this)
				gain3 /= 4;										// Dragging sound is loud to scene atom with ear at sound emit loc.
		}

		if(IsEatingPose())
		{
			if(IsHerbivore())
				sound2 = SOUND_EatingPlants;
			else
				sound2 = SOUND_EatingMeat;
			epos2 = model.Project(CameraOffset());	// at head
		}
		else if(gp.IsBiting())
		{
			sound2 = SOUND_EatingMeat;
			epos2 = model.Project(CameraOffset());	// at head
		}

		if(isFlying)
		{
			sound2 = SOUND_WingFlap;
			epos2 = model.Project(CGOffset());				// At body pivot point.
			if(sceneAtom == this)
				gain2 /= 8;			// Flapping sounds are too loud to scene atom with ear at sound emit loc.
		}

		// Rather than create another sound source, a digger will use one of
		// the three footstep noises for the digging noise.
		if(isDigging)
		{
			// The gain of the dig sound is increased so it's actually audible
			// on the surface.
			const float digGain = 3.f;
			if(sound2 == SOUND_None)
			{
				sound2 = SOUND_Dig;
				epos2 = model.Project(CGOffset());			// At body pivot point.
				gain2 *= digGain;
			}
			else if(sound1 == SOUND_None)
			{
				sound1 = SOUND_Dig;
				epos1 = model.Project(CGOffset());			// At body pivot point.
				gain1 *= digGain;
			}
			else if(sound3 == SOUND_None)
			{
				sound3 = SOUND_Dig;
				epos3 = model.Project(CGOffset());			// At body pivot point.
				gain3 *= digGain;
			}
			else
			{
				// Give up plural footsteps in exchange for dig sound.
				sound2 = SOUND_Dig;
				epos2 = model.Project(CGOffset());			// At body pivot point.
				gain2 *= digGain;
			}
		}

		// Calculate pitch from speed.
		m_BodyLock.lockForRead();
		float period;
		if(m_Body)
			period = 1.f + 4.f/((float)m_Body->DNA().Speed+1.f);	// secs/cycle
		else
			period = 2;	//DHH animalia
		m_BodyLock.unlock();
		pitch = speed / period;		// cycles per second
		pitch *= 1.5f;				// adjust for step rate in audio sample
#if !defined(Q_OS_WIN)
		// Linux OpenAL appears to be limited to 2x pitch. More than 2 results
		// in no pitch change at all. Consequently, my walking mount doesn't
		// sound like he's running when I run with him.
		if(pitch > 2.f)
			pitch = 2.f;
#endif
	}

	// Turn on/off the appropriate sound sources.
	// This will turn them off if not making any noise.
	float refDist1 = (float)AuSound::SoundReferenceDistance(sound1);
	float rolloff1 = AuSound::SoundRolloffFactor(sound1);
	float refDist2 = (float)AuSound::SoundReferenceDistance(sound2);
	float rolloff2 = AuSound::SoundRolloffFactor(sound2);
	float refDist3 = (float)AuSound::SoundReferenceDistance(sound3);
	float rolloff3 = AuSound::SoundRolloffFactor(sound3);

	// Primary feet sound.
	SOUND_EFFECT cur = (SOUND_EFFECT)m_Sound[0];
	al->Play(m_AudioCreature[0], cur, sound1, epos1.x, epos1.y, epos1.z, gain1, pitch, true, refDist1, rolloff1, 0);
	m_Sound[0] = cur;

	// Secondary feet sound for multiple legged walkers.
	cur = (SOUND_EFFECT)m_Sound[1];
	al->Play(m_AudioCreature[1], cur, sound2, epos2.x, epos2.y, epos2.z, gain2, pitch, true, refDist2, rolloff2, 1);
	m_Sound[1] = cur;

	// Drag sound.
	cur = (SOUND_EFFECT)m_Sound[2];
	al->Play(m_AudioCreature[2], cur, sound3, epos3.x, epos3.y, epos3.z, gain3, pitch, true, refDist3, rolloff3, 0);
	m_Sound[2] = cur;
}

double DaCreature::OrientationSpeed() const
{
	return PI;
}

bool DaCreature::OkToOrient() const
{
	if(IsLockedPose() || IsClimbingLadder())
		return false;
	else if(IsBodyDyingOrDead())
	{
		// Lifeless body orients when it hits the ground or water.
		if(IsOnGround() || IsInWater())
			return DaUnit::OkToOrient();
	}
	else if(IsProne())
	{
		// Prone body orients to the ground when touching ground, otherwise orients to gravity when in control.
		// Jumpers naturally remain upright while midair.
		if(IsOnGround())
			return DaUnit::OkToOrient();
		else if(CanControlMovement() || (IsInAtmosphere() && CanJump()))
			return DaUnit::OkToOrient();
	}
	else
	{
		// Live non-prone body orients to gravity when in control.
		// Jumpers naturally remain upright while midair.
		if(CanControlMovement() || (IsInAtmosphere() && CanJump()))
			return DaUnit::OkToOrient();
	}
	return false;
}

bool DaCreature::Orient(const AuPoint& atLoc, const AuPoint& terNormal)
{
	if(!OkToOrient())
	{
		SetOrientRotation(Rotation());
		return false;	// Rotation was not changed.
	}

	AuPoint oldVec;
	bool rotvec = false;
	if(!CanControlMovement())
	{
		oldVec = Transform().ProjectVector(Vector());
		rotvec = true;
	}
	AuPoint otn = Transform().ProjectVector(TerrainNormal());
	AuPoint ogn = Transform().ProjectVector(GravityNormal());

	// NOTE: The body should orient under the same conditions under which the head tilts, in DaPerson::OkToTiltHead.
	//       When the body does not orient, tilting of the head changes the orientation of the body instead.
	bool oriented = false;
	if(IsBodyDyingOrDead())
	{
		// Lifeless body orients when it hits the ground or water.
		if(IsOnGround() || IsInWater())
			oriented = DaUnit::Orient(atLoc, terNormal);	// orient to terrain angle
	}
	else if(IsProne())
	{
		// Prone body orients to the ground when touching ground, otherwise orients to gravity when in ship or atmosphere.
		if(IsOnGround())
			oriented = DaUnit::Orient(atLoc, terNormal);	// orient to terrain angle
		else if(!rotvec || IsInSpacecraft() || IsInAtmosphere())
			oriented = DaUnit::Orient(atLoc, ogn);			// orient to gravity vector
	}
	else
	{
		// Live non-prone body orients to gravity when in ship or atmosphere.
		if(!rotvec || IsInSpacecraft() || IsInAtmosphere())
			oriented = DaUnit::Orient(atLoc, ogn);			// orient to gravity vector
	}

	if(!oriented)
		return false;

	// When this atom is not touching ground, its movement vector must be
	// held in the direction it was going, sort of like real inertia.
	if(rotvec)
	{
		AuPoint nuVec = Transform().UnProjectVector(oldVec);
		SetVector(nuVec);
	}
	AuPoint nuVec = Transform().UnProjectVector(ogn);
	SetGravityNormal(nuVec);
	nuVec = Transform().UnProjectVector(otn);
	SetTerrainNormal(nuVec);
	return true;
}

// Acceleration in ft/sec/sec.
AuPoint DaCreature::Acceleration() const
{
	if(!IsAffectedByGravity())
		return AuPoint(0,0);
	return DaUnit::Acceleration();
}

// Only called when floating 'in' water or 'on' water.
// 1 is maximum float, 0 is neutral, -1 is maximum sink
float DaCreature::Buoyancy(const AuQueryEnvironment& qe) const
{
	// No floating in the designer preview.
	if(qe.InDesignPreview())
		return 0;

	// Do not float up once my eyes are at or above the water level.
	if(qe.SeaDepth() == 0)
	{
		// Eye is at or above the sea surface.
		double altmsl = qe.AltitudeMSL();						// MSL altitude of my CG.
		if(altmsl > 0)
		{
			// My CG is above the water surface.
			double cght = GroundHeightAboveCG();
			if(altmsl >= cght)
				return -1;										// Just barely in the water or above it. Maximum gravity vector.
			return -altmsl / cght;								// Sink down to my cg. Modulate gravity vector as I sink.
		}
		else
		{
			// My CG is below or level with the water and my eye is supposed to be above or level with the water.
			// How far above the water is it?
			double camalt = CameraOffset().z;					// Altitude of eye above CG.
			if(camalt > 0)
			{
				// Camera is above CG.
				double eyealt = altmsl + camalt;				// MSL altitude of eye.
				if(eyealt >= 0)
				{
					// Eye is above or level with the water surface and CG is below or level with the water.
					double frac = eyealt / camalt;				// Fraction of eye height above the water.
					if(frac < .5)
						return 0.;								// Half my neck is allowed out of the water before I sink.

					// The amount of my head above the halfway length of my neck is weighing me down.
					return 1 - frac * 2;						// Returns 0 to -1 to be multiplied by gravity vector.
				}
			}
		}
	}

	// Sink or float based on my buoyancy.
	float density = Density();
	if(density == DensityOfWater() || density == 0)
		return 0;							// Neither ascend nor descend in water.

	// Float up while our eyes are below the water.
	return DensityOfWater() / density - 1;
}

/*
// Drag in ft/sec/sec.
// Drag is such that a creature will fall at 120mph in standard atmosphere at
// standard gravity acceleration (32ft/sec/sec).
#define CREATURE_DRAG_FACTOR(maxMph, accel)	((5280./3600.*maxMph) * (5280./3600.*maxMph) / accel)
const double s_CreatureDF = CREATURE_DRAG_FACTOR(120, 32);
AuPoint DaCreature::Drag() const
{
	float airDensity;
	if(IsInSpacecraft() && IsAirBreathable())
		airDensity = .5f;	// standard atmosphere aboard spacecraft
	else
		airDensity = AtmosphereDensity();
	double drag = (m_Vector.Length2() / s_CreatureDF) * airDensity * 2;
	return AuPoint(drag, drag, drag);
}
*/

// In square feet. Used by drag calculation.
// X is lateral
// Y is forward
// Z is vertical
// See http://www.newton.dep.anl.gov/askasci/phy99/phy99x88.htm
AuPoint DaCreature::CrossSectionalArea() const
{
	// Increase Z area when flying/swimming level to stop climbing/descending.
	//if(Is(AuId(47791798)))
		//qDebug() << Id() << Name() << "isFlying" << IsFlying() << "inWater" << IsInVisibleWater()
			//<< "headTilt" << CurrentHeadTilt()*RAD2DEGf << "levelHeaded" << (bool)(CurrentHeadTilt() == 0);
	if((IsFlying() || IsInVisibleWater()) && CurrentHeadTilt() == 0)
		return AuPoint(2,1,25);

	return AuPoint(2,1,2);
}

AuPoint DaCreature::Friction() const
{
	// Determine friction due to ground surface contact.
	// Assumes I'm oriented upright to the surface.
	double fricFwd = 0, fricLat = 0, fricVrt = 0;
	switch(OnGround())
	{
	case OG_None:			// Not on the ground so no additional drag from environment. Could be over an open hatch.
	case OG_WaterSoundOnly:	// Not really returned by OnGround().
		// Need friction for stability when floating in some environments.
		if(CanHover() && CurrentGeneralPose() == AuPose::Hover &&
				(fabs(CurrentHeadTilt()) < 10*DEG2RAD || dynamic_cast<const DaPerson*>(this)))
		{
			// Creature is hovering.
			// Animals get drag when not trying to climb or descend.
			// People always get drag when hovering.
			fricFwd = 15;
			fricLat = 15;
			fricVrt = 5;
		}
		else if(CanFly() && IsFlying() &&
				(fabs(CurrentHeadTilt()) < 10*DEG2RAD || dynamic_cast<const DaPerson*>(this)))
		{
			// Creature is flying.
			// Animals get vertical drag when not trying to climb or descend.
			// People always get vertical drag when flying to help maintain alt.
			fricLat = 15;
			fricVrt = 5;
		}
		else if(CurrentGeneralPose() == AuPose::Ladder)
		{
			// Stationary ladder pose.
			// Same as metal floor.
			fricFwd = 21;
			fricLat = 31;
			fricVrt = 50;	// Not trying to move on ladder (pose is stationary) so give a lot of vertical friction.
		}
		else if(IsLadderPose())
		{
			// Same as metal floor.
			fricFwd = 21;
			fricLat = 31;
			fricVrt = 31;
		}
		else if(IsInVisibleWater())
		{
			// Friction in water must not exceed the creature's ability to move.
			fricFwd = MovementSpeed(1)/2;
			if(fricFwd && fricFwd < 5)
			{
				// Creature is moving with weak force.
				fricLat = fricFwd;
				fricVrt = fricFwd/5;
			}
			else
			{
				fricFwd = 5;
				fricLat = 5;
				fricVrt = 1;
			}
		}
		else if(IsInGravityWell() || InLadderWell())
		{
			// Friction in gravity well must not exceed the creature's ability to move.
			fricFwd = MovementSpeed(1)/3;
			if(fricFwd && fricFwd < 5)
			{
				// Creature is moving with weak force.
				fricLat = fricFwd;
				fricVrt = fricFwd;
			}
			else
			{
				fricFwd = 5;
				fricLat = 5;
				fricVrt = 5;
			}
		}
		else if(InDesignerNotPreview())
		{
			const DaDesignBase* pde = FindParentDesignBaseInEnv();
			if(pde && !pde->IsRenderSolid(Id()) && !pde->IsPreview())
			{
				// Wireframe-rendered spacecraft.
				// Movement force is a consistent 20 at walking speed in design environment, regardless of my DNA speed, see
				// DaPerson::Acceleration.
				// Need friction to slow down high speed movement.
				fricFwd = MovementSpeed(1)/2;
				if(fricFwd && fricFwd < 10)
				{
					// Creature is moving with weak force.
					fricLat = fricFwd;
					fricVrt = fricFwd;
				}
				else
				{
					fricFwd = 10;
					fricLat = 10;
					fricVrt = 10;
				}
			}
			else if(!IsInCapsule())
			{
				// Not in solid-rendered spacecraft so apply some floating friction.
				// Friction must not exceed the creature's ability to move.
				fricFwd = MovementSpeed(1)/2;
				if(fricFwd && fricFwd < 4)
				{
					// Creature is moving with weak force.
					fricLat = fricFwd;
					fricVrt = fricFwd;
				}
				else
				{
					fricFwd = 4;
					fricLat = 4;
					fricVrt = 4;
				}
			}
			else
			{
				// Somewhere inside a solid-rendered spacecraft and not touching the floor.
				// Friction must not exceed the creature's ability to move.
				fricFwd = MovementSpeed(1)/3;
				if(fricFwd && fricFwd < 3)
				{
					// Creature is moving with weak force.
					fricLat = fricFwd;
					fricVrt = fricFwd;
				}
				else
				{
					fricFwd = 3;
					fricLat = 3;
					fricVrt = 3;
				}
			}
		}
		else if(IsInCapsule())
		{
			// Somewhere inside a spacecraft and not touching the floor.
			// Friction must not exceed the creature's ability to move.
			fricFwd = MovementSpeed(1)/3;
			if(fricFwd && fricFwd < 3)
			{
				// Creature is moving with weak force.
				fricLat = fricFwd;
				fricVrt = fricFwd;
			}
			else
			{
				fricFwd = 3;
				fricLat = 3;
				fricVrt = 3;
			}
		}
		break;
	case OG_Forest:
		fricFwd = 29;
		fricLat = 39;
		fricVrt = 29;
		break;
	case OG_Snow:
		fricFwd = 31;
		fricLat = 41;
		fricVrt = 31;
		break;
	case OG_HardFloor:
		fricFwd = 22;
		fricLat = 32;
		fricVrt = 22;
		break;
	case OG_Metal:
		fricFwd = 21;
		fricLat = 31;
		fricVrt = 31;
		break;
	case OG_Pavement:
		fricFwd = 25;
		fricLat = 35;
		fricVrt = 25;
		break;
	case OG_WoodFloor:
		fricFwd = 22;
		fricLat = 32;
		fricVrt = 22;
		break;
	case OG_Rock:
		fricFwd = 30;
		fricLat = 40;
		fricVrt = 30;
		break;
	case OG_Sand:
		fricFwd = 26;
		fricLat = 36;
		fricVrt = 26;
		break;
	case OG_Savanna:
		fricFwd = 21;
		fricLat = 31;
		fricVrt = 21;
		break;
	}

//#if !defined(QT_NO_DEBUG)
	//if((g_Au.m_LogMessages & LOG_Physics) && FindPersonOnLineRecursive())
		//qDebug() << "DaCreature::Friction" << Id() << Name() << AuPoint(fricLat, fricFwd, fricVrt);
//#endif
	return AuPoint(fricLat, fricFwd, fricVrt);
}

bool DaCreature::CanBeUnderGround() const
{
	if(DaUnit::CanBeUnderGround())
		return true;	// Must be above an open hatch.

	// Only subterranean creatures can be under ground.
	if(!LivesUnderGround())
		return false;

	// Creatures don't dig into spacecraft or vehicle floors.
	// Base class already tested for an open hatch.
	if(IsInSpacecraft() || IsAboardVehicle())
		return false;

	// Dying creatures must not fall under ground.
	if(IsBodyDyingOrDead())
		return false;

	// Creatures cannot go under ground while flying.
	if(IsFlying())
		return false;

	// Creatures cannot go under ground while mounted.
	if(IsMounted())
		return false;

	// All looks ok for this subterranean creature to go below the ground surface.
	return true;
}

// NOTE: Rendering states must already be configured. Check actual usage of this function.
void DaCreature::RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
		POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
		const AuMatrix4x4& tDevOrtho, float hudIntensity)
{
	Q_UNUSED(targetPart);
	Q_UNUSED(sceneAtom);
	Q_UNUSED(tDevPersp);
	Q_UNUSED(tDevOrtho);
	Q_UNUSED(hudIntensity);

	// Three horizontal bars for body stats.
	const float barwid = wid;
	const float barspacing = .005f * g_Au.BodyStatSize()/100.f;
	const float barht = (ht - barspacing)/2;					// I will draw two bars.

	// Stance toward target and Health
	{
		float valbot = bottom + barspacing + barht;

		if(isTarget)
		{
			// Name is also the political stance indicator, by color.
			QColor col = PoliticalStanceColor(stance, IsCivilian());

			// Name
			// Calculate pixel location of text.
			QRect rcClient = ari.glw->rect();
			float namebot = valbot + barht + barspacing;
			float nameleft = left * AuGLEffects::WindowAspectRatio(rcClient);
			int nx = rcClient.width()/2 + (int)(nameleft * (float)rcClient.width()/2.f);
			int ny = rcClient.height()/2 - (int)(namebot * (float)rcClient.height()/2.f);
			ari.RenderText(nx, ny, Name(), col);
		}

		// Hit points bar.
		float curhp = CurrentHitPoints();
		float maxhp = MaxHitPoints();
		float frac = curhp/maxhp;

		float rl,gl,bl, rr,gr,br;
		if(frac <= .1f || IsAnyConditionVisible())
		{
			// bright red
			rl = 1;
			gl = .2f;
			bl = .2f;

			// dim red
			rr = .2f;
			gr = .04f;
			br = .04f;
		}
		else if(frac < .5f)
		{
			// bright amber
			rl = 1;
			gl = .7f;
			bl = .2f;

			// dim amber
			rr = .2f;
			gr = .14f;
			br = .04f;
		}
		else
		{
			// bright green
			rl = .2f;
			gl = 1;
			bl = .2f;
			// dim green
			rr = .04f;
			gr = .2f;
			br = .04f;
		}

		ari.RenderBar(left,valbot, barwid,barht, frac, 2, rl,gl,bl, rr,gr,br);
	}

	if(!isTarget)
	{
		// Stamina/Lungs
		float valbot = bottom;
		float curval = m_Stamina;
		float maxval;
		if(m_Body)
			maxval = m_Body->Stamina();
		else
			maxval = m_Animalia->Stamina();
		float frac = curval/maxval;
		float r,g,b;
		if(frac <= 1.f/6.f)
		{
			// purple
			r = 140.f/255.f;
			g = 51.f/255.f;
			b = 222.f/255.f;
		}
		else if(frac <= 1.f/3.f)
		{
			// blue
			r = 150.f/255.f;
			g = 155.f/255.f;
			b = 223.f/255.f;
		}
		else
		{
			// pink
			r = 1;
			g = 188.f/255.f;
			b = 188.f/255.f;
		}
		ari.RenderBar(left,valbot, barwid,barht, frac, 3, r,g,b, r/10.f,g/10.f,b/10.f, 1.f);
	}
}

float DaCreature::HullStress() const
{
	// Certain tests have already been made.
	Q_ASSERT(IsInAtmosphere());

	float hullStress = (float)Drag().Length2() / (float)(800*800);
	return hullStress;
}

void DaCreature::TestHullStress()
{
	if(!IsInAtmosphere())
		return;
	if(IsInVehicle())
		return;
	if(IsInsideSpacecraft())
		return;

	float hullStress = HullStress();
	if(hullStress > 1)
	{
		if(hullStress <= 1.5f)
		{
			// Stress is enough to induce damage but not outright destruction.
			// Determine hits of damage in one second at this speed.
			AuCombatAttacker attacker(tr("excess external stress"));

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_w1.m_Effect = WE_Sonic;
			weapon.m_w2.m_Effect = WE_Crush;

			AuCombatResult result;
			int hits = MaxHitPoints()/5 + 1;	// 10% damage per second
			result.m_Damage1 = g_Au.Dice().Roll(1,hits);
			result.m_Damage2 = g_Au.Dice().Roll(1,hits);

			CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			//if(IsBodyDead())
			//{
				//qDebug("DaCreature::TestHullStress %d %s killed by excess external stress %g.", Id().GetID(),
						//(const char*)Name().toAscii(), hullStress);
			//}
		}
		else
		{
			// Stress is enough to flat-out kill the creature.
			AuCombatAttacker attacker(tr("critical external stress"));

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_Mode = WM_Internal;
			weapon.m_w1.m_Effect = WE_Sonic;
			weapon.m_w2.m_Effect = WE_Crush;

			AuCombatResult result;
			int hits = MaxHitPoints();
			result.m_Damage1 = hits;
			result.m_Damage2 = hits;

			CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			//qDebug("DaCreature::TestHullStress %d %s killed by critical external stress %g.", Id().GetID(),
					//(const char*)Name().toAscii(), hullStress);
		}
	}

	if(!IsMouthSubmerged() && !IsUnderGround() && AirDensity() > .95f && !IsProtectedByEnvironmentSuit())
	{
		// High atmospheric pressure. Induce 10% damage per second.
		AuCombatAttacker attacker(tr("atmospheric pressure"));

		AuCombatWeapon weapon(WM_Internal);
		weapon.m_w1.m_Effect = WE_Crush;

		AuCombatResult result;
		result.m_Damage1 = MaxHitPoints()/10;

		CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		SetModified();
	}
}

void DaCreature::DismountAllRiders()
{
	DaAtomList dismount;
	ListSubatoms(dismount);
	while(!dismount.isEmpty())
	{
		DaAtom* rider = dismount.takeFirst();
		DaCreature* creature = dynamic_cast<DaCreature*>(rider);
		if(creature)
			creature->DismountCreature();					// We have code for dismounting creatures from creatures.
		else
		{
			// Transform the passenger atom, whatever it is, to my location and rotation.
			AuPoint rideRot = rider->Rotation();
			AuMatrix4x4 toff;
			toff.Rotate(rideRot);
			AuMatrix4x4 t = Transform() * toff;

			// Transfer my movement vector to the passenger.
			AuPoint vec = toff.UnProject(Vector());
			rider->ForcePosMove(AuAtomPos(ParentId(), t), vec);
			rider->SetModified();
		}
	}
}

// Calculate gun position relative to body center.
AuPoint DaCreature::WeaponLoc() const
{
	const double offdist = 2.5;
	const double odxy = offdist * .7071;	// xy for 45 deg angles
	AuPoint wpnloc;
	AuPose gp = CurrentGeneralPose();
	switch(gp.Pose())
	{
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		if(IsInSpacecraft())
		{
			const DaSc* sc = dynamic_cast<const DaSc*>(Parent());
			if(sc)
			{
				float baseRot = Rotation().z;
				float gunRot;
				if(m_Body)
					gunRot = m_Body->CurrentHeadTilt();
				else
					gunRot = m_Animalia->CurrentHeadTilt();
				AuMatrix4x4 mTur;
				sc->TurretMuzzleMatrix(StationId(), baseRot, gunRot, mTur);
				AuPoint muzzleLoc = mTur.Translation();						// Relative to the ship.
				muzzleLoc = Transform().UnProject(muzzleLoc);					// Relative to my body.
				return muzzleLoc;
			}
		}
		break;
	case AuPose::ConsoleTurret0:
		if(IsInCapsule())
			return AuPoint(offdist,0,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurret45:
		if(IsInCapsule())
			return AuPoint(odxy,odxy,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurret90:
		if(IsInCapsule())
			return AuPoint(0,offdist,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurret135:
		if(IsInCapsule())
			return AuPoint(-odxy,odxy,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurret180:
		if(IsInCapsule())
			return AuPoint(-offdist,0,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurret225:
		if(IsInCapsule())
			return AuPoint(-odxy,-odxy,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurret270:
		if(IsInCapsule())
			return AuPoint(0,-offdist,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurret315:
		if(IsInCapsule())
			return AuPoint(odxy,-odxy,(4-(c_GunMaxZ+c_GunMinZ)/2) -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurretBottom:
		if(IsInCapsule())
			return AuPoint(0,0,-c_GunMaxZ -GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	case AuPose::ConsoleTurretTop:
		if(IsInCapsule())
			return AuPoint(0,0,(DeckHeight()-c_GunMinZ) - GroundHeightAboveCG()).RotateZ(-Rotation().z);
		break;
	default:
		// Weapon is always in the dominant hand, even if I don't have hands.
		if(IsLeftHanded())
			return LeftHandLoc();
		else
			return RightHandLoc();
	}
	return AuPoint(0,0);
}

// Matrix of weapon model relative to me. +X is firing direction.
AuMatrix4x4 DaCreature::WeaponMatrix() const
{
	AuMatrix4x4 m;
	m.Translate(CGOffset());
	m_BodyLock.lockForRead();
	if(m_Animalia)
	{
		if(m_Animalia->Handedness() == RightHanded)
			m *= m_Animalia->RightHandMatrix();
		else
			m *= m_Animalia->LeftHandMatrix();
	}
	m_BodyLock.unlock();
	return m;
}

// Location for weapon muzzle flash effect.
AuPoint DaCreature::WeaponMuzzleLoc() const
{
	if(HasDNA3())
	{
		switch(ItemInHandCID())
		{
		default:
			return WeaponLoc();
		case CID_HeavyWeapon:
			// The CID_HeavyWeapon model is 7.00207' from the pivot point to the end of the barrel.
			return WeaponLoc() + AuPoint(0,6.5,0).RotateX(CurrentHeadTilt());	// start effect just inside barrel end
		case CID_AK47:
			return WeaponLoc() + AuPoint(0,2.18,.335).RotateX(CurrentHeadTilt());
		case CID_Rifle:
			return WeaponLoc() + AuPoint(0,2.18,.335).RotateX(CurrentHeadTilt());
		case CID_Shotgun:
			return WeaponLoc() + AuPoint(0,2.61,.37).RotateX(CurrentHeadTilt());
		case CID_Pistol:
		case CID_LaserPistol:
			return WeaponLoc() + AuPoint(0,.464,.251).RotateX(CurrentHeadTilt());
		case CID_LaserRifle:
			return WeaponLoc() + AuPoint(0,1.8388,.128).RotateX(CurrentHeadTilt());
		}
	}
	else
	{
		AuPoint l = DbCommodity::WeaponMuzzleLoc(ItemInHandCID());
		if(l.IsZero())
			return WeaponLoc();
		return WeaponMatrix().Project(l);
	}
}

AuPoint DaCreature::WeaponMuzzleVec() const
{
	AuPose gp = CurrentGeneralPose();
	switch(gp.Pose())
	{
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		if(IsInSpacecraft())
		{
			const DaSc* sc = dynamic_cast<const DaSc*>(Parent());
			if(sc)
			{
				float baseRot = Rotation().z;
				float gunRot;
				if(m_Body)
					gunRot = m_Body->CurrentHeadTilt();
				else
					gunRot = m_Animalia->CurrentHeadTilt();
				AuMatrix4x4 mTur;
				sc->TurretMuzzleMatrix(StationId(), baseRot, gunRot, mTur);
				AuPoint lookVec = mTur.ProjectVector(0,0,1);					// Relative to the ship.
				lookVec = Transform().UnProjectVector(lookVec);					// Relative to my body.
				return lookVec;
			}
		}
		break;
	default:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
		break;
	}
	if(HasDNA3())
	{
		AuPoint emitDir = AuPoint(0,1,0).RotateX(CurrentHeadTilt());
		return emitDir;
	}
	else
	{
		return WeaponMatrix().ProjectVector(1,0,0);
	}
}

// Rotation for effect at weapon muzzle loc.
AuPoint DaCreature::WeaponMuzzleRot() const
{
	return AuPoint(CurrentHeadTilt(),0,0);
}

AuPoint DaCreature::WeaponRot() const
{
	//Q_ASSERT(HasDNA3());
	// The head tilt rotation makes the final tilt align with the look direction.
	switch(ItemInHandCID())
	{
	case CID_Head:
	case CID_LaserRifle:
	case CID_LaserPistol:
		// These models are created aiming down the positive Y axis, with Z up.
		return AuPoint(CurrentHeadTilt(),0,0);

	case CID_PreonRingO:
	case CID_PreonRingB:
	case CID_PreonRingA:
	case CID_PreonRingF:
	case CID_PreonRingG:
	case CID_PreonRingK:
	case CID_PreonRingM:
		if(IsLeftHanded())
			return AuPoint(CurrentHeadTilt(),0,90*DEG2RAD);
		else
			return AuPoint(CurrentHeadTilt(),0,-90*DEG2RAD);

	default:
		// The older models were created with positive Z up, aiming down the positive X axis.
		// The model needs to be rotated to aim down the positive Y axis.
		return AuPoint(CurrentHeadTilt(),0,90*DEG2RAD);
	}
}

void DaCreature::SetAtRest()
{
	//qDebug("DaCreature::SetAtRest %d %s", Id().GetID(), (const char*)Name().toAscii());
	DaUnit::SetAtRest();
	Orient(Location(), Transform().ProjectVector(TerrainNormal()));	// Instant orient when at rest.
}

void DaCreature::FilterFinalMoveVector(AuPoint& vector) const
{
	//qDebug() << "DaCreature::FilterFinalMoveVector" << Id() << Name() << "vector" << vector << "onGnd" << IsOnGround()
		//<< "canCtrl" << CanControlMovement();
	// When lying on the ground, eliminate the downward portion of the movement vector.
	// This is the same problem faced by vehicles and spacecraft, which are sort of like prone bodies because of the way they
	// orient to the terrain. Any downward Z here causes them to creep across the terrain when sitting on a hill.
	if(IsInGravity() && IsTouchingGround() && vector.z < 0 && !CanBeUnderGround())
	{
		if(IsProne() || (!IsLadderPose() && vector.Length2_2D() <= 1) || InDesignerNotPreview())
			vector.z = 0;
	}

	// Limit movement vector to the speed of light.
	if(!vector.IsZero() && vector.Length2() > SpeedOfLight2())
		vector.Unit(SpeedOfLight());

	if(IsLadderPose())
	{
		// Cannot move laterally while on a ladder.
		vector.x = 0;
		vector.y = 0;
	}
}

void DaCreature::FilterPostMoveVector(AuPoint& vector) const
{
	if(!IsInGravity())
		return;
	if(vector.z >= 0)
		return;
	// Keep some gravity in the vector when moving or it's bouncy going down hills.
	// We need to limit gravity while touching ground or it grows and grows and starts dragging us around the terrain.
	if(IsTouchingGround())
	{
		if(MoveResultBlocked())
			vector.z = 0;	// Eliminate gravity vector when blocked to help get free from the side of buildings.
		else if(IsInSpacecraft())
		{
			// Eliminate remaining gravity vector when touching the floor in a spacecraft. There are no hills to bounce down
			// and the gravity vector causes a drop when entering a gravity well.
			if(!IsLadderPose())
				vector.z = 0;
		}
		else
		{
			// Must be touching terrain.
			if(vector.z < -GravityStandard() && !CanBeUnderGround())
				vector.z = -GravityStandard();
		}
	}
}

// Create a special effect of an item functioning.
// cid is the commodity id of the item, CID_Invalid when using a body part.
// wMode indicates how the item was used
void DaCreature::UseItemEffect(COMMODITYID cid, WEAPONMODE wMode, WEAPONEFFECT explodeEffect, double explodeRadius,
		INVENTORYSUB subMode, const AuPoint& targetLoc)
{
	SOUND_EFFECT sound = SOUND_None;
	VISUAL_EFFECT visual = VISUAL_None;
	WEAPONEFFECT energyBeamEffect = WE_None;
	AuPoint loc = WeaponMuzzleLoc();
	float gain = 1.f;
	if(wMode == WM_Contact || cid == CID_Invalid)
	{
		sound = SOUND_KnifeSwish;
		visual = VISUAL_KnifeSwish;
	}
	else
	{
		switch(cid)
		{
		case CID_Pistol:
			sound = SOUND_FirePistol;
			visual = VISUAL_FirePistol;
			break;
		case CID_AK47:
			sound = SOUND_FireAK47;
			visual = VISUAL_FireRifle;
			break;
		case CID_Rifle:
			sound = SOUND_FireRifle;
			visual = VISUAL_FireRifle;
			break;
		case CID_Shotgun:
			sound = SOUND_FireShotgun;
			visual = VISUAL_FireShotgun;
			break;
		case CID_LaserPistol:
			if(subMode == IPS_Weapon1)
			{
				sound = SOUND_FireLaserCannon;
				visual = VISUAL_FireEnergyBeam;
				energyBeamEffect = WE_Electric;
			}
			else
			{
				sound = SOUND_FireLaserPulse;
				visual = VISUAL_FireEnergyBeam;
				energyBeamEffect = WE_Heat;
			}
			gain = .1f;
			break;
		case CID_LaserRifle:
			if(subMode == IPS_Weapon1)
			{
				sound = SOUND_FireLaserCannon;
				visual = VISUAL_FireEnergyBeam;
				energyBeamEffect = WE_Electric;
			}
			else
			{
				sound = SOUND_FireLaserPulse;
				visual = VISUAL_FireEnergyBeam;
				energyBeamEffect = WE_Heat;
			}
			gain = .2f;
			break;
		case CID_Mount_Anus:
		case CID_Mount_Eyes:
		case CID_Mount_Feet:
		case CID_Mount_Hands:
		case CID_Mount_Head:
		case CID_Mount_Tail:
		case CID_Mount_Torso:
		case CID_My_Anus:
		case CID_My_Eyes:
		case CID_My_Feet:
		case CID_My_Hands:
		case CID_My_Head:
		case CID_My_Tail:
		case CID_My_Torso:
			if(wMode == WM_Projectile)
			{
				visual = VISUAL_FireEnergyBeam;
				energyBeamEffect = WE_Radiation;
			}
			else if(wMode == WM_Psionic)
			{
				visual = VISUAL_FireEnergyBeam;
				energyBeamEffect = WE_Sonic;
			}
			else if(wMode == WM_Area)
				visual = VISUAL_Explosion;

			// Calculate weapon emit location.
			switch(cid)
			{
			default:
				break;
			case CID_My_Hands:
				// Use default weapon left/right hand location.
				break;
			case CID_My_Anus:
			case CID_My_Tail:
				loc = CGOffset();
				break;
			case CID_My_Eyes:
			case CID_My_Head:
				loc = CameraOffset();
				break;
			case CID_My_Feet:
				loc = CGOffset();
				loc.z -= GroundHeightAboveCG();
				break;
			case CID_My_Torso:
				loc = (CGOffset() + CameraOffset())/2;
				break;

			case CID_Mount_Hands:
				{
					const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
					if(mount)
						loc = Transform().UnProject(mount->Transform().Project(mount->WeaponLoc()));
				}
				break;
			case CID_Mount_Anus:
			case CID_Mount_Tail:
				{
					const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
					if(mount)
						loc = Transform().UnProject(mount->Transform().Project(mount->CGOffset()));
				}
				break;
			case CID_Mount_Eyes:
			case CID_Mount_Head:
				{
					const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
					if(mount)
						loc = Transform().UnProject(mount->Transform().Project(mount->CameraOffset()));
				}
				break;
			case CID_Mount_Feet:
				{
					const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
					if(mount)
					{
						loc = mount->CGOffset();
						loc.z -= mount->GroundHeightAboveCG();
						loc = Transform().UnProject(mount->Transform().Project(loc));
					}
				}
				break;
			case CID_Mount_Torso:
				{
					const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
					if(mount)
						loc = Transform().UnProject(mount->Transform().Project((mount->CGOffset() + mount->CameraOffset())/2));
				}
				break;
			}
			break;

		default:
			break;
		}
	}

	if(sound != SOUND_None || visual != VISUAL_None)
	{
		// Effect emanates from dominant hand.
		//if(!IsTurretPose())
			//m_Body.AnimatePose(HasItemInHand());	// Position hand for muzzle flash placement.
		AuPoint rot = WeaponMuzzleRot();

		// Waver z rotation slightly for contact weapons.
		if(wMode == WM_Contact)
			rot.z += g_Au.Dice().Roll(1,11,-6) * DEG2RADf;

		// Add the effect.
		AuEffect effect(loc, rot, AuPoint(0,0), sound, gain, 1, visual);
		effect.SetTargetLoc(targetLoc);
		if(visual == VISUAL_FireEnergyBeam)
			effect.SetEnergyBeamEffect(energyBeamEffect);
		else if(visual == VISUAL_Explosion)
			effect.SetExplosionEffect1(explodeEffect, explodeRadius);
		AddEffect(effect);
	}
}

DaItem* DaCreature::DropItem(const AuItem& item, const AuPoint& offsetVec)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	//qDebug() << "DaCreature::DropItem" << Id() << Name() << "item" << item.SayName();
	if(!item.IsValid())
	{
		qWarning("DaCreature::DropItem item cid %d is not valid", item.CommodityId());
		return 0;
	}

	// Create a commodity atom to encapsulate the inventory item.
	// The item is initially 'in motion' so it will fall/rise from wherever we place it to its natural resting point.
	DaItem *pc = new DaItem(item);
	pc->ConnectSignals();
	pc->Create();

	// Place the item in the environment.
	DaCreature* mount = dynamic_cast<DaCreature*>(Parent());
	if(mount)
	{
		// Add the item at my location relative to the mount's parent.
		AuMatrix4x4 align = item.GroundUpVector().GetAlignmentTransform(AuPoint(0,0,1));
		AuMatrix4x4 t = mount->Transform();
		t.Translate(offsetVec);
		t *= align;

		// Item gets mount's move vector, adding vector due to offset. The vector is in the space of the item.
		AuPoint vec = align.UnProjectVector(mount->Vector() + offsetVec/10);

		pc->SetPosAndOrient(AuAtomPos(mount->ParentId(), t));
		pc->SetVector(vec);
		pc->SetInMotion();
	}
	else
	{
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(Parent());
		if(vehicle)
		{
			// Add the item at an empty seat position in the vehicle.
			int itemSeat = vehicle->PreviousUnoccupiedSeat(VehicleSeatIndex());
			if(itemSeat >= 0)
			{
				// Found an empty seat for the item.
				AuPoint seatLoc = vehicle->SeatLoc(itemSeat);
				if(!vehicle->SeatPose(itemSeat).HeadOnly())
					seatLoc.z += pc->GroundHeightAboveCG();

				// Add the item just above the empty seat location relative to my parent atom.
				AuMatrix4x4 t;
				t.Translate(seatLoc);
				t *= item.GroundUpVector().GetAlignmentTransform(AuPoint(0,0,1));

				pc->SetPosAndOrient(AuAtomPos(ParentId(), t));
			}
			else
			{
				// No empty seats for the item. Throw it overboard.
				AuPoint offLoc = vehicle->OffLoc(VehicleSeatIndex());

				// Add the item at the offload location for my seat.
				AuMatrix4x4 align = item.GroundUpVector().GetAlignmentTransform(AuPoint(0,0,1));
				AuMatrix4x4 t = vehicle->Transform();
				t.Translate(offLoc + offsetVec);
				t *= align;

				// Item gets vehicle's move vector, adding vector due to offset. The vector is in the space of the item.
				AuPoint vec = align.UnProjectVector(vehicle->Vector() + offsetVec/10);

				pc->SetPosAndOrient(AuAtomPos(vehicle->ParentId(), t));
				pc->SetVector(vec);
				pc->SetInMotion();
			}
		}
		else
		{
			// Add the item at my location relative to my parent atom.
			AuMatrix4x4 align = item.GroundUpVector().GetAlignmentTransform(AuPoint(0,0,1));
			AuMatrix4x4 t = Transform();
			t.Translate(offsetVec);
			t *= align;

			// Item gets my move vector, adding vector due to offset. The vector is in the space of the item.
			AuPoint vec = align.UnProjectVector(Vector() + offsetVec/10);
			//qDebug() << "DaCreature::DropItem" << Id() << Name() << "vec" << vec << "atRest" << IsAtRest();

			pc->SetPosAndOrient(AuAtomPos(ParentId(), t));
			pc->SetVector(vec);
			pc->SetInMotion();
		}
	}
	ether->AddAtom(pc);

	// Notify the world of the nu atom.
	g_Au.AtomAdded(pc->Id());

	pc->SetInScene();					// Don't decay too fast right away.
	pc->SetNormal();					// New atom is ready for normal thread processing.
	pc->TickTock();						// Get it moving right away.
	return pc;
}

bool DaCreature::DropRandomItem(AuGear& gear, bool attachThought)
{
	//qDebug() << "DaCreature::DropRandomItem" << Id() << Name();
	// Drop all slippery items first, even those associated with a story instance. So far, story items are the only slippery ones.
	bool anyDropped = false;
	AuGear::iterator ii = gear.begin();
	while(ii != gear.end())
	{
		if(!(*ii).Item().Content() || (*ii).Item().Content()->empty())
		{
			// Top level item is not a container or it is empty.
			// Don't drop story props.
			if((*ii).Item().IsSlipperyOnDeath())
			{
				if(attachThought)
				{
					if((*ii).Item().Content())
						AddCondition(new AuConThought(tr("I dropped an empty %1.").arg((*ii).Item().SayName()), AuTime::Now()));
					else
						AddCondition(new AuConThought(tr("I dropped %1.").arg((*ii).Item().SayName()), AuTime::Now()));
				}

				DropItem((*ii).Item());
				ii = gear.erase(ii);
				anyDropped = true;
			}
			else
				ii++;
		}
		else
		{
			// Top level item is a container that is not empty.
			AuItemContent::iterator ci = (*ii).Item().Content()->begin();
			while(ci != (*ii).Item().Content()->end())
			{
				if((*ci).second->IsSlipperyOnDeath())
				{
					// Drop an item out of the container.
					//qDebug() << "DaCreature::DropRandomItem" << Id() << Name() << "dropping" << (*ci).second->SayName()
						//<< "out of pack" << (*ii)->SayName();
					if(attachThought)
						AddCondition(new AuConThought(tr("I dropped %1 out of my %2.")
									.arg((*ci).second->SayName())
									.arg((*ii).Item().SayName()), AuTime::Now()));

					DropItem(*(*ci).second);
					ci = (*ii).Item().Content()->Erase(ci);
					anyDropped = true;
				}
				else
					ci++;
			}
			ii++;
		}
	}

	// If we dropped any slippery items, that's enough.
	if(anyDropped)
		return true;

	// Drop a random item that is not sticky.
	// Count the valid items in my inventory, including those in containers. Empty containers are also counted.
	// Containers with contents are not counted.
	int items = 0;	// Top level items that can be dropped.
	for(AuGear::const_iterator ii = gear.begin(); ii != gear.end(); ii++)
	{
		if(!(*ii).Item().Content() || (*ii).Item().Content()->empty())
		{
			// Top level item is not a container or it is empty.
			// Don't drop story props. They will be marked 'permanent' if they can be dropped, which 0's the story instance.
			if((*ii).Item().StoryInstance() && g_Au.IsStoryActive((*ii).Item().StoryInstance()))
				continue;
			else if(!(*ii).Item().IsDroppableOnDeath())
				continue;

			items++;
		}
		else
		{
			// Top level item is a container that is not empty.
			for(AuItemContent::const_iterator ci = (*ii).Item().Content()->begin(); ci != (*ii).Item().Content()->end(); ci++)
			{
				// Don't drop story props. They will be marked 'permanent' if they can be dropped, which 0's the story instance.
				if((*ci).second->StoryInstance() && g_Au.IsStoryActive((*ci).second->StoryInstance()))
					continue;
				else if(!(*ci).second->IsDroppableOnDeath())
					continue;

				items++;	// Contained item that can be dropped.
			}
		}
	}

	if(!items)
		return false;	// Nothing to drop.

	int dropIndex = g_Au.Dice().Roll(1,items,-1);
	items = 0;
	for(AuGear::iterator ii = gear.begin(); ii != gear.end(); ii++)
	{
		if(!(*ii).Item().Content() || (*ii).Item().Content()->empty())
		{
			// Top level item is not a container or it is empty.
			// Don't drop story props. They will be marked 'permanent' if they can be dropped, which 0's the story instance.
			if((*ii).Item().StoryInstance() && g_Au.IsStoryActive((*ii).Item().StoryInstance()))
				continue;
			else if(!(*ii).Item().IsDroppableOnDeath())
				continue;

			if(items == dropIndex)
			{
				// Drop a top level item.
				if(attachThought)
				{
					if((*ii).Item().Content())
						AddCondition(new AuConThought(tr("I dropped an empty %1.").arg((*ii).Item().SayName()), AuTime::Now()));
					else
						AddCondition(new AuConThought(tr("I dropped %1.").arg((*ii).Item().SayName()), AuTime::Now()));
				}
				DropItem((*ii).Item());
				gear.erase(ii);
				return true;
			}
			items++;
		}
		else
		{
			// Top level item is a container that is not empty.
			for(AuItemContent::iterator ci = (*ii).Item().Content()->begin(); ci != (*ii).Item().Content()->end(); ci++)
			{
				// Don't drop story props. They will be marked 'permanent' if they can be dropped, which 0's the story instance.
				if((*ci).second->StoryInstance() && g_Au.IsStoryActive((*ci).second->StoryInstance()))
					continue;
				else if(!(*ci).second->IsDroppableOnDeath())
					continue;

				if(items == dropIndex)
				{
					// Drop an item out of the container.
					if(attachThought)
						AddCondition(new AuConThought(tr("I dropped %1 out of my %2.")
									.arg((*ci).second->SayName())
									.arg((*ii).Item().SayName()), AuTime::Now()));
					DropItem(*(*ci).second);
					(*ii).Item().Content()->Erase(ci);
					return true;
				}
				items++;
			}
		}
	}
	return false;
}

//static
AuPoint DaCreature::ConsolePoseOffset(AuStation::TYPE consoleType, AuStation::OCCUPANT_POSITION occPos, float cgHeightStanding,
		float torsoThickness, int seatIndex)
{
	switch(consoleType)
	{
	case AuStation::Console_Command:
	case AuStation::Console_Engine:
	case AuStation::Console_FireControl:
	case AuStation::Console_Helm:
	case AuStation::Console_Medic:
	case AuStation::Console_Navigation:
	case AuStation::Console_Power:
	case AuStation::Console_Sensor:
	case AuStation::Console_Shield:
	case AuStation::Console_Transporter:
		// Control console station location is at the seat. No offset is required.
		return AuPoint(0,0,0);
	case AuStation::Console_Patient:
		return AuPoint(0,0, cgHeightStanding);			// Creatures stand inside the surgery unit.
	case AuStation::Berth_1_0:
		return AuPoint(1.5,0,2);
	case AuStation::Berth_1_45:
		return AuPoint(1.5,0,2).RotateZ(45*DEG2RAD);
	case AuStation::Berth_1_90:
		return AuPoint(1.5,0,2).RotateZ(90*DEG2RAD);
	case AuStation::Berth_1_135:
		return AuPoint(1.5,0,2).RotateZ(135*DEG2RAD);
	case AuStation::Berth_1_180:
		return AuPoint(1.5,0,2).RotateZ(180*DEG2RAD);
	case AuStation::Berth_1_225:
		return AuPoint(1.5,0,2).RotateZ(225*DEG2RAD);
	case AuStation::Berth_1_270:
		return AuPoint(1.5,0,2).RotateZ(270*DEG2RAD);
	case AuStation::Berth_1_315:
		return AuPoint(1.5,0,2).RotateZ(315*DEG2RAD);
	case AuStation::Berth_2_0:
		if(seatIndex)
			return AuPoint(1.5,0,5.5);							// Top Bunk
		else
			return AuPoint(1.5,0,2);								// Bottom Bunk
	case AuStation::Berth_2_45:
		if(seatIndex)
			return AuPoint(1.5,0,5.5).RotateZ(45*DEG2RAD);		// Top Bunk
		else
			return AuPoint(1.5,0,2).RotateZ(45*DEG2RAD);			// Bottom Bunk
	case AuStation::Berth_2_90:
		if(seatIndex)
			return AuPoint(1.5,0,5.5).RotateZ(90*DEG2RAD);		// Top Bunk
		else
			return AuPoint(1.5,0,2).RotateZ(90*DEG2RAD);			// Bottom Bunk
	case AuStation::Berth_2_135:
		if(seatIndex)
			return AuPoint(1.5,0,5.5).RotateZ(135*DEG2RAD);		// Top Bunk
		else
			return AuPoint(1.5,0,2).RotateZ(135*DEG2RAD);			// Bottom Bunk
	case AuStation::Berth_2_180:
		if(seatIndex)
			return AuPoint(1.5,0,5.5).RotateZ(180*DEG2RAD);		// Top Bunk
		else
			return AuPoint(1.5,0,2).RotateZ(180*DEG2RAD);			// Bottom Bunk
	case AuStation::Berth_2_225:
		if(seatIndex)
			return AuPoint(1.5,0,5.5).RotateZ(225*DEG2RAD);		// Top Bunk
		else
			return AuPoint(1.5,0,2).RotateZ(225*DEG2RAD);			// Bottom Bunk
	case AuStation::Berth_2_270:
		if(seatIndex)
			return AuPoint(1.5,0,5.5).RotateZ(270*DEG2RAD);		// Top Bunk
		else
			return AuPoint(1.5,0,2).RotateZ(270*DEG2RAD);			// Bottom Bunk
	case AuStation::Berth_2_315:
		if(seatIndex)
			return AuPoint(1.5,0,5.5).RotateZ(315*DEG2RAD);		// Top Bunk
		else
			return AuPoint(1.5,0,2).RotateZ(315*DEG2RAD);			// Bottom Bunk
	case AuStation::Turret_Bottom:
		return AuPoint(0,0, AuStation::SeatHeight()-1);			// Move up to seat ht, directly above turret.
	case AuStation::Turret_Top:
		return AuPoint(0,0,-DpBody::SeatDrop());				// Move down to seat position, directly below turret.
	case AuStation::Turret_0:
		return AuPoint(-.5,0, AuStation::SeatHeight());
	case AuStation::Turret_45:
		return AuPoint(-.35,-.35, AuStation::SeatHeight());
	case AuStation::Turret_90:
		return AuPoint(0,-.5, AuStation::SeatHeight());
	case AuStation::Turret_135:
		return AuPoint(.35,-.35, AuStation::SeatHeight());
	case AuStation::Turret_180:
		return AuPoint(.5,0, AuStation::SeatHeight());
	case AuStation::Turret_225:
		return AuPoint(.35,.35, AuStation::SeatHeight());
	case AuStation::Turret_270:
		return AuPoint(0,.5, AuStation::SeatHeight());
	case AuStation::Turret_315:
		return AuPoint(-.35,.35, AuStation::SeatHeight());
	case AuStation::Table_Game:
		break;
	case AuStation::Table_Game_0:
		// Four seats clockwise around the unrotated game table.
		// Array the seat locations around the table according to angle.
		switch(seatIndex)
		{
		case 0:
			return AuPoint(4, 4, AuStation::SeatHeight());
		case 1:
			return AuPoint(4, -4, AuStation::SeatHeight());
		case 2:
			return AuPoint(-4, -4, AuStation::SeatHeight());
		case 3:
			return AuPoint(-4, 4, AuStation::SeatHeight());
		}
		break;
	case AuStation::Table_Game_45:
		// Four seats clockwise around the game table that is rotated 45 degrees.
		// Array the seat locations around the table according to angle.
		switch(seatIndex)
		{
		case 0:
			return AuPoint(4, 4, AuStation::SeatHeight()).RotateZ(45*DEG2RAD);
		case 1:
			return AuPoint(4, -4, AuStation::SeatHeight()).RotateZ(45*DEG2RAD);
		case 2:
			return AuPoint(-4, -4, AuStation::SeatHeight()).RotateZ(45*DEG2RAD);
		case 3:
			return AuPoint(-4, 4, AuStation::SeatHeight()).RotateZ(45*DEG2RAD);
		}
		break;
	case AuStation::Berth_Captain:
	case AuStation::Berth_Crew:
	case AuStation::Berth_Officer:
	case AuStation::Berth_Passenger:
	case AuStation::Berth_Troop:
	case AuStation::Citizen:
	case AuStation::Integrated_Command:
	case AuStation::Integrated_Designer:	// We don't ever really get here with this setting.
	case AuStation::Integrated_Engine:
	case AuStation::Integrated_FireControl:
	case AuStation::Integrated_Helm:
	case AuStation::Integrated_Medic:
	case AuStation::Integrated_Navigation:
	case AuStation::Integrated_Patient:
	case AuStation::Integrated_Power:
	case AuStation::Integrated_Sensor:
	case AuStation::Integrated_Shield:
	case AuStation::Integrated_Transporter:
	case AuStation::Livestock:
	case AuStation::Troop:
	case AuStation::Turret_Integrated:
		switch(occPos)
		{
		case AuStation::Occupant_Prone:
			return AuPoint(0,0, torsoThickness/2);
		case AuStation::Occupant_Sit:
		case AuStation::Occupant_SitSwivel3D:
		case AuStation::Occupant_SitStationary:
			return AuPoint(0,0, 0);
		case AuStation::Occupant_Stand:
		case AuStation::Occupant_StandStationary:
			return AuPoint(0,0, cgHeightStanding);
		}
		break;
	case AuStation::TYPE_Count:
		break;
	}
	return AuPoint(0,0);
}

SYSTEM DaCreature::StationSystem() const
{
	AuPose gp = CurrentGeneralPose();
	switch(gp.Pose())
	{
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretTop:
	case AuPose::ConsoleTurretBottom:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		return SYS_Turret;
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleNav:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsoleTransporter:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Patient_Prone:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
		return SYS_CommandAndControl;
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	default:
		break;
	}
	return SYS_Count;
}

void DaCreature::operator>>(QDataStream& out) const
{
	DaUnit::operator>>(out);
	out << m_AirInLungs;
	out << m_BodyQ;
	out << m_ControlId;
	out << m_MinutesToFullGrown;
	out << m_SeatIndex;
	out << m_Stamina;

	m_BodyLock.lockForRead();

	if(m_Body)
	{
		out << (quint8)0;
		*m_Body >> out;
	}
	else
	{
		out << (quint8)1;
		out << (quint8)m_Animalia->Chromatid();
		out << m_Animalia->DNA();
		out << (quint8)m_Animalia->Handedness();
		out << (quint8)m_Haircut;
		out << (quint8)m_Shave;
		out << m_Animalia->CurrentGeneralPose();
		out << m_Animalia->CurrentMouthPose();
		out << m_Animalia->CurrentHitPoints();
		out << m_MouthStationary;
	}

	out << m_StationId;
	out << AuTime();	//m_tmBodyChange;
	out << AuTime();	//m_tmHairChange;
	out << AuTime();	//m_tmHeadChange;
	out << AuTime();	//m_tmHPChange;
	out << m_MouthAnimDur;
	AuTime mouthAnimStart;
	out << mouthAnimStart;
	out << m_MouthFinish;
	out << m_MouthStationary;
	out << AuTime();	//m_tmPoseChange;
	m_BodyLock.unlock();

	m_HungerLock.lockForRead();
	out << m_Hunger;
	out << AuTime();	//m_tmHungerChange;
	m_HungerLock.unlock();

	m_ItemInHandLock.lockForRead();
	if(m_ItemInHand)
	{
		out << (quint8)m_ItemInHand->Type();
		out << *m_ItemInHand;
	}
	else
		out << (quint8)0;
	m_ItemInHandLock.unlock();
}

void DaCreature::operator<<(QDataStream& in)
{
	DaUnit::operator<<(in);
	in >> m_AirInLungs;
	in >> m_BodyQ;
	in >> m_ControlId;
	quint8 minToFullGrown;
	in >> minToFullGrown;
	in >> m_SeatIndex;
	in >> m_Stamina;

	m_BodyLock.lockForWrite();

	AuPose aPose, aMouth;
	quint8 ui8;
	in >> ui8;
	if(ui8 == 0)
	{
		if(m_Animalia)
		{
			delete m_Animalia;
			m_Animalia = 0;
		}
		if(!m_Body)
			m_Body = new DpBody;
		*m_Body << in;
		aPose = m_Body->CurrentGeneralPose();
	}
	else
	{
		if(m_Body)
		{
			delete m_Body;
			m_Body = 0;
		}
		if(m_Animalia)
			delete m_Animalia;

		in >> ui8;
		AuDNA::CHROMATID chromatid = (AuDNA::CHROMATID)ui8;
		AuDNA dna;
		in >> dna;
		in >> ui8;
		HANDEDNESS handedness = (HANDEDNESS)ui8;
		in >> ui8;
		m_Haircut = (AuHairStyle::STYLE)ui8;
		in >> ui8;
		m_Shave = (AuHairStyle::SHAVE)ui8;
		in >> aPose;
		in >> aMouth;
		qint16 hp;
		in >> hp;
		in >> m_MouthStationary;

		m_Animalia = new AuAnimalia(dna, chromatid, handedness, Mature);
		m_Animalia->SetHitPoints(hp);
	}

	quint16 stationId;
	in >> stationId;
	AuTime tmBodyChange;
	in >> tmBodyChange;
	AuTime tmHairChange;
	in >> tmHairChange;
	AuTime tmHeadChange;
	in >> tmHeadChange;
	AuTime tmHPChange;
	in >> tmHPChange;
	in >> m_MouthAnimDur;
	AuTime mouthAnimStart;
	in >> mouthAnimStart;
	in >> m_MouthFinish;
	in >> m_MouthStationary;
	AuTime tmPoseChange;
	in >> tmPoseChange;
	m_BodyLock.unlock();

	m_HungerLock.lockForWrite();
	in >> m_Hunger;
	AuTime tmHungerChange;
	in >> tmHungerChange;
	m_HungerLock.unlock();

	m_ItemInHandLock.lockForWrite();
	if(m_ItemInHand)
		delete m_ItemInHand;
	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_ItemInHand = AuItem::Create(itemType);
		if(m_ItemInHand)
		{
			in >> *m_ItemInHand;

			// Items rarely change type. e.g. Computer changed from a regular item to a storage device.
			AuItem* replacement = m_ItemInHand->FixItem();
			if(replacement)
			{
				delete m_ItemInHand;
				m_ItemInHand = replacement;
			}
		}
	}
	else
		m_ItemInHand = 0;
	m_ItemInHandLock.unlock();

	SetMinutesToFullGrown(minToFullGrown);

	// Console poses require the console object to be created.
	RestorePose(aPose, stationId);
	if(m_Animalia)
		m_Animalia->PoseMouth(Id(), aMouth);
}

void DaCreature::ApplyAtmosphericDamage()
{
	quint32 nomDmg = MaxHitPoints() / 10;
	if(!nomDmg)
		nomDmg = 1;

	AuCombatAttacker attacker;

	AuCombatWeapon weapon(WM_Internal);
	weapon.m_w1.m_Effect = WE_Suffocate;

	AuCombatResult result;
	result.m_Damage1 = nomDmg;

	if(IsInVacuum())
	{
		// In vacuum just take suffocation damage.
		attacker.m_Name = tr("vacuum");
	}
	else
	{
		// Not in vacuum so take suffocation damage and temperature damage based on orbit zone.
		switch(OrbitZone())
		{
		case OZONE_Inferno:
			attacker.m_Name = tr("inferno atmosphere");
			weapon.m_CID = CID_InfernoAtmosphere;
			weapon.m_w2.m_Effect = WE_Heat;
			result.m_Damage2 = nomDmg*2;
			break;
		case OZONE_Frigid:
		case OZONE_DeepSpace:
			attacker.m_Name = tr("frigid atmosphere");
			weapon.m_CID = CID_FrigidAtmosphere;
			weapon.m_w2.m_Effect = WE_Cold;
			result.m_Damage2 = nomDmg;
			break;
		case OZONE_Inner:
		case OZONE_Habitable:
		case OZONE_Outer:
			switch(InAtmosphereType())
			{
			case ATMTYPE_Standard:
			case ATMTYPE_Tainted:
			case ATMTYPE_Count:
				attacker.m_Name = tr("toxic atmosphere");
				weapon.m_CID = CID_Atmosphere;
				break;
			case ATMTYPE_Exotic:
				attacker.m_Name = tr("exotic atmosphere");
				weapon.m_CID = CID_Atmosphere;
				weapon.m_w2.m_Effect = WE_Stun;
				result.m_Damage2 = nomDmg;
				break;
			case ATMTYPE_Corrosive:
				attacker.m_Name = tr("corrosive atmosphere");
				weapon.m_CID = CID_Atmosphere;
				weapon.m_w2.m_Effect = WE_Acid;
				result.m_Damage2 = nomDmg*3/2;
				break;
			case ATMTYPE_Insidious:
				attacker.m_Name = tr("insidious atmosphere");
				weapon.m_CID = CID_Atmosphere;
				weapon.m_w2.m_Effect = WE_Poison;
				result.m_Damage2 = nomDmg*2;
				break;
			}
			break;
		}
	}
	CombatApplyDamageLater(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
}

void DaCreature::ApplyLiquidDamage()
{
	quint32 nomDmg = MaxHitPoints() / 10;
	if(!nomDmg)
		nomDmg = 1;

	AuCombatAttacker attacker;

	AuCombatWeapon weapon(WM_Internal);

	AuCombatResult result;

	if(IsInLava())
	{
		// Nothing protects against lava.
		attacker.m_Name = tr("lava");
		weapon.m_CID = CID_Magma;
		weapon.m_w1.m_Effect = WE_Heat;
		result.m_Damage1 = nomDmg * 3;
		CombatApplyDamageLater(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
	}
	else if(IsInSupercooledLiquid())
	{
		// Environment suit protects against supercooled liquid.
		// Some wild animals live in the supercooled liquid of gas giants. That exception is handled in DaAnimal.
		if(!IsProtectedByEnvironmentSuit())
		{
			attacker.m_Name = tr("supercooled liquid");
			weapon.m_CID = CID_SupercooledLiquid;
			weapon.m_w1.m_Effect = WE_Cold;
			result.m_Damage1 = nomDmg;
			CombatApplyDamageLater(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		}
	}
	else
	{
		// Non aquatic creatures need an environment helmet on to go under water.
		if(!LivesInWater() && IsMouthSubmerged() && !DbCommodity::IsEnvironmentHelmet(Helmet()) && !HoldBreath())
		{
			// Lose stamina at one point per second until I start drowning.
			if(m_Stamina)
			{
				// Lose a point of stamina.
				m_Stamina--;	
				SetModified();
			}
			else
			{
				attacker.m_Name = tr("drowning");
				weapon.m_CID = CID_Water;
				weapon.m_w1.m_Effect = WE_Suffocate;
				result.m_Damage1 = nomDmg/5;
				if(!result.m_Damage1)
					result.m_Damage1 = 1;
				CombatApplyDamageLater(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			}
		}
	}
}

bool DaCreature::IsObstacle(const DaAtom* toWho) const
{
	if(IsEthereal())
		return false;
	//if(EnvironmentQueryValid() && toWho->EnvironmentQueryValid() && IsInAtmosphere() != toWho->IsInAtmosphere())
	if(IsInAtmosphere() != toWho->IsInAtmosphere())
		return false;
	const DaItem* item = dynamic_cast<const DaItem*>(toWho);
	if(item)
		return item->IsActiveMissile() && !item->IsArmedBy(Id());	// Creatures block active missiles, unless launched by them.

	// Creatures do not block other creatures.
	if(dynamic_cast<const DaCreature*>(toWho))
		return false;
	return true;
}

QString DaCreature::SpotDescription(quint32 spotterEmpire) const
{
	QString name = tr("%1 %2")
		.arg(GenderName(Gender()))
		.arg(SpeciesName());

	int curHits = CurrentHitPoints();
	int maxHits = MaxHitPoints();
	if(curHits == maxHits)
		name += tr(" %1 hp").arg(maxHits);
	else
		name += tr(" %1/%2 hp").arg(curHits).arg(maxHits);

	if(EmpireId())
	{
		QString empire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(EmpireId(), empire);
		POLITICAL_STANCE stance = gd.EmpirePoliticalStance(spotterEmpire, EmpireId());
		g_Au.UnlockData();
		switch(stance)
		{
		case POL_Friend:
			return tr("Friendly %1 %2").arg(empire).arg(name);
		case POL_Vassal:
			return tr("Overlord %1 %2").arg(empire).arg(name);
		case POL_Neutral:
			return tr("Neutral %1 %2").arg(empire).arg(name);
		case POL_Enemy:
			return tr("Enemy %1 %2").arg(empire).arg(name);
		case POL_Empire:
			return tr("%1 %2").arg(empire).arg(name);
		}
	}
	return name;
}

void DaCreature::LoadClientConsoleSettings()
{
	//Q_ASSERT(g_Au.IsSceneApp());
	if(!HasConsole())
		return;													// Do nothing when there is no console.

	// Get current planet id, if any.
	AuId planetId;
	const DaWorld* planet = FindParentPlanetInEnv();
	if(planet)
		planetId = planet->Id();

	// This console knows how to restore its settings.
	m_Console->LoadClientSettings(planetId);
}

void DaCreature::StoreClientConsoleSettings() const
{
	//Q_ASSERT(g_Au.IsSceneApp());
	if(!HasConsole())
		return;													// Do nothing when there is no console.

	// Get current planet id, if any.
	AuId planetId;
	const DaWorld* planet = FindParentPlanetInEnv();
	if(planet)
		planetId = planet->Id();

	// Store the console settings.
	m_Console->StoreClientSettings(planetId);
}

DaAtom* DaCreature::SightShotgunTarget(AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, float shotgunRange,
		float patternAngle) const
{
	//qDebug() << "DaCreature::SightShotgunTarget" << Id() << Name();

	// Get my look ray in my own space. The matrix will project all other atoms in the environment to be relative to me.
	AuPoint eyeLoc = CameraOffset();

	// Spiral out from the center until we hit a target, any target.
	const int bbs = 16;
	const int bbsPerSpin = 8;
	DaAtom* bestTarget = 0;
	double closestDist2 = 0;
	AuPoint lookVec;
	for(int bb=0; bb<bbs; bb++)
	{
		// Calculate the arc angle outward from the center to the maximum deflection of the shotgun pattern.
		//float arcAngle = (1.145762838f*DEG2RADf) * (bb+1) / bbs;
		float arcAngle = patternAngle * bb / bbs;

		// Calculate a spin angle that will circle the center once for every 10 bb's.
		float spinAngle = (PIf*2) * (bb%bbsPerSpin) / bbsPerSpin;

		// Rotate straight-out look vector up by the arcAngle, which grows from 0 to 1.15 degrees.
		lookVec = AuPoint(0,1,0).RotateX(arcAngle);

		// Spin the tilted look vector around the Y axis to create a spiral search.
		lookVec = lookVec.RotateY(spinAngle);

		//qDebug() << "  arcAngle" << arcAngle*RAD2DEGf << "spinAngle" << spinAngle*RAD2DEGf << "lookVec" << lookVec;

		// Tilt perturbed look vector to current head tilt.
		lookVec = lookVec.RotateX(CurrentHeadTilt());
		if(CurrentGeneralPose() == AuPose::ReceptiveMate || CurrentGeneralPose() == AuPose::MateOnBottom)
		{
			if(HasDNA3())
				lookVec = lookVec.RotateY(PI);
			lookVec = lookVec.RotateX(PI/2);
		}

		// Check for a target for the bb.
		// Only the first bb is projected out to full sight range. Remaining bbs are limited to maximum shotgun range.
		AuPoint bbLoc;
		unsigned int bbPart = 0;
		bool bbShield = false;
		DaAtom* target;
		if(bb)
			target = DaUnit::SightTarget(bbLoc, bbPart, bbShield, eyeLoc, eyeLoc + lookVec * shotgunRange);
		else
			target = DaUnit::SightTarget(bbLoc, bbPart, bbShield, eyeLoc, eyeLoc + lookVec * c_CreatureSightRange);
		if(target)
		{
			double dist2 = bbLoc.Length2();
			if(!bestTarget || dist2 < closestDist2)
			{
				// Closest bb to hit so far.
				bestTarget = target;
				closestDist2 = dist2;
				sightedLoc = bbLoc;
				sightedPart = bbPart;
				sightedShield = bbShield;
			}
		}
	}
	return bestTarget;
}

DaAtom* DaCreature::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
		AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	// Don't sight myself or my subatoms.
	// Don't sight creatures with bay weapons of ships or bases. These call us with a sighting atom of a ship or a city.
	if(!IsSightTarget(sightingAtom))
		return pBest;

	// Determine my location in the space of the sighting atom.
	// If I am far enough away from the sight segment, this atom and my subordinate atoms can be ignored.
	AuPoint myLoc = tModel.Project(AuPoint(0,0));
	if(myLoc.DistToSegment(eyeLoc, sightedLoc) > MaxHitPoints())	// Rough wag at size.
		return pBest;

	// This atom can be sighted by sightingAtom when targeting.
	// Get a list of the rough extents quads. If we don't hit one of those, we don't need to get more detailed.
	AuQuadArray extSurfs;
	AuBox extBox;
	if(RoughExtents(extBox))
	{
		extBox.Transform(tModel);
		extBox.GetQuads(extSurfs);

		// If the eye is inside or the sighted point is inside, checkDetails is set true so we'll do a more detailed
		// check of the actual sight surfaces and skip the intersection tests with the extents.
		bool checkDetails = (extSurfs.TestPointInside(eyeLoc) || extSurfs.TestPointInside(sightedLoc)) ? true : false;
		if(!checkDetails)
		{
			// Eye is not inside and sight point are both inside.
			// Test to see if the current look segment intersects any sides.
			AuPoint intersect, normal;
			for(AuQuadArray::const_iterator it = extSurfs.cbegin(); it != extSurfs.cend() && !checkDetails; it++)
				if((*it).TestSegmentIntersection(intersect, normal, eyeLoc, sightedLoc))
					checkDetails = true;	// At least one side is i'sected
		}

		// If the rough test passed, we need to get more detailed.
		if(checkDetails && SightTargetDetails(eyeLoc, tModel, sightedLoc, sightedPart, sightedShield))
		{
			pBest = (DaAtom*)this;			// Must cast me to non-const.
			if(sightedSurfaceNormal)
				*sightedSurfaceNormal = AuPoint(0,0);
		}
	}

	// Base class performs a recursive search.
	return DaUnit::SightTarget(sightingAtom, eyeLoc, tModel, pBest, sightedLoc, sightedPart, sightedShield, sightedSurfaceNormal);
}

bool DaCreature::IsReadyForMating() const
{
	if(!g_Au.IsMatingOk())
		return false;			// Nerf solo sex.
	if(IsBodyDyingOrDead())
		return false;			// I am dead or almost there. No necrophilia.
	if(IsLockedPose())
		return false;			// Not while at any station.
	if(Gender() == Asexual)
		return false;			// Asexual creatures do not mate sexually.
	if(IsPregnant())
		return false;			// I am already pregnant.
	if(!IsFullGrown())
		return false;			// I am immature. No pedophilia here.
	return Armor() == CID_Invalid && Helmet() == CID_Invalid;	// I can mate if I am naked.
}

bool DaCreature::TryMating(const DaCreature& creature) const
{
	// Check to see if creature is ready for mating.
	if(creature.CurrentGeneralPose() != AuPose::ReceptiveMate)
		return false;		// Creature has not posed their body for mating.
	if(!creature.IsReadyForMating())
		return false;		// Creature is not ready to mate. Could be asexual.
	return TryBecomingReceptiveToMating(creature);
}

bool DaCreature::TryBecomingReceptiveToMating(const DaCreature& creature) const
{
	if(!IsReadyForMating())
		return false;		// I am not ready to mate. Could be asexual.
	return IsSexuallyCompatible(creature);
}

bool DaCreature::IsSexuallyCompatible(const DaCreature& creature) const
{
	// Check for appropriate gender combination.
	switch(creature.Gender())
	{
	case Female:
		if(Gender() != Male)
			return false;				// Wrong gender combination.
		break;
	case Male:
		if(Gender() != Female)
			return false;				// Wrong gender combination.
		break;
	case Asexual:
		return false;					// Wrong gender combination.
	case Hermaphrodite:
		if(Gender() != Hermaphrodite)
			return false;				// Wrong gender combination.
		break;
	}

	bool compatible = true;
	AuDNABase* crdna = creature.DNAnew();
	AuDNABase* mydna = DNAnew();
	if(crdna && mydna)
	{
		if(!crdna->IsSexuallyCompatible(*mydna))
			compatible = false;
	}
	else
		compatible = false;

	if(crdna)
		delete crdna;
	if(mydna)
		delete mydna;
	return compatible;
}

void DaCreature::MountCreature(DaCreature* creature)
{
	if(Is(creature->ParentId()) || creature->Is(ParentId()))
		return;	// One of us is already subordinate to the other.

	// When my body is dying, clear the pending bleeding damage or the next hit causes my body to either a) die or b) change pose.
	if(IsBodyDyingOrDead())
		ClearPendingDamage();

	// Mount the creature facing backward.
	Straddle();
	ChangePos(AuAtomPos(creature->Id(), AuMatrix4x4()));
	InvalidateEnvironment();
	creature->SetModified();

	// Creature positions its riders.
	creature->PositionRiders();
}

void DaCreature::ForceMountCreature(DaCreature* creature)
{
	if(Is(creature->ParentId()) || creature->Is(ParentId()))
		return;	// One of us is already subordinate to the other.

	// When my body is dying, clear the pending bleeding damage or the next hit causes my body to either a) die or b) change pose.
	if(IsBodyDyingOrDead())
		ClearPendingDamage();

	// Mount the creature facing backward.
	Straddle();
	ForcePosStop(AuAtomPos(creature->Id(), AuMatrix4x4()));		// Reparent myself to the creature.
	SyncController(AuSyncPose(AuPose::Straddle, 0, 0, 0));
	creature->SetModified();

	// Creature positions its riders.
	creature->ForcePositionRiders();
}

void DaCreature::MateWithCreature(DaCreature* creature)
{
	if(Is(creature->ParentId()) || creature->Is(ParentId()))
		return;	// One of us is already subordinate to the other.

	// Have sex with the creature facing forward.
	MateOnTop();
	AuMatrix4x4 mountedM;
	mountedM.RotateZ(PI);
	ChangePosAndOrient(AuAtomPos(creature->Id(), mountedM));
	InvalidateEnvironment();

	// Creature positions its riders.
	creature->MateOnBottom();
	creature->PositionRiders();
	creature->SetModified();

	// Pregnancy is only handled on the server.
	if(!g_Au.IsSceneWorker() && !g_Au.IsSceneSolo())
		return;

	// The copulating partners trade any souvenirs they are carrying. This will unfortunately include
	// poison, which might not make sense. We'll assume not too many poisoned people will be having sex.
	LockConditions().InflictCreatureIfUnique(creature);
	UnlockConditions();

	creature->LockConditions().InflictCreatureIfUnique(this);
	creature->UnlockConditions();
}

void DaCreature::PositionRiders()
{
	float bodyThick = TorsoThickness()/2;						// Allow for my body thickness.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Determine proper location of atom.
		AuPoint rideLoc = CGOffset();							// Riders sit at my body pivot point.
		rideLoc += AuPoint(0, -bodyThick, bodyThick);			// Move back and up by my body thickness.

		DaCreature* rider = dynamic_cast<DaCreature*>(*it);
		if(rider)
		{
			// Creature rider positioning. Insertion point of creature bodies when riding an animal is their waist/pivot point.
			rideLoc.z += rider->TorsoThickness()/2;				// Move up by the rider's body thickness.
			if(!rideLoc.FuzzyEquals(rider->Location()))
			{
				// Position the creature atom.
				AuMatrix4x4 t;
				t.Translate(rideLoc);							// Position rider at saddle.
				t.Rotate(rider->Rotation());					// Preserve rider's rotation.
				rider->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				rider->InvalidateEnvironment();
				rider->SetModified();
			}
		}
		else
		{
			// Position the non-creature atom. Probably an item.
			if(!rideLoc.FuzzyEquals((*it)->Location()))
			{
				AuMatrix4x4 t;
				t.Translate(rideLoc);
				(*it)->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				(*it)->InvalidateEnvironment();
				(*it)->SetModified();
			}
		}
	}
}

void DaCreature::ForcePositionRiders()
{
	float bodyThick = TorsoThickness()/2;						// Allow for my body thickness.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Determine proper location of atom.
		AuPoint rideLoc = CGOffset();							// Riders sit at my body pivot point.
		rideLoc += AuPoint(0, -bodyThick, bodyThick);			// Move back and up by my body thickness.

		DaCreature* rider = dynamic_cast<DaCreature*>(*it);
		if(rider)
		{
			// Creature rider positioning. Insertion point of creature bodies when riding an animal is their waist/pivot point.
			rideLoc.z += rider->TorsoThickness()/2;				// Move up by the rider's body thickness.
			if(!rideLoc.FuzzyEquals(rider->Location()))
			{
				// Position the creature atom.
				AuMatrix4x4 t;
				t.Translate(rideLoc);							// Position rider at saddle.
				t.Rotate(rider->Rotation());					// Preserve rider's rotation.
				AuAtomPos pos(Id(), t);
				rider->ChangePosAndOrientSyncNotParent(pos);
				rider->InvalidateEnvironment();
				rider->SetModified();
			}
		}
		else
		{
			// Position the non-creature atom. Probably an item.
			if(!rideLoc.FuzzyEquals((*it)->Location()))
			{
				AuMatrix4x4 t;
				t.Translate(rideLoc);
				AuAtomPos pos(Id(), t);
				(*it)->ChangePosAndOrientSyncNotParent(pos);
				(*it)->InvalidateEnvironment();
				(*it)->SetModified();
			}
		}
	}
}

void DaCreature::DismountCreature()
{
	DaCreature* mount = dynamic_cast<DaCreature*>(Parent());
	if(!mount)
		return;

	bool wasMating = IsMating();

	// I am riding a creature; dismount it.
	AuPoint rideRot = Rotation();
	AuMatrix4x4 toff;
	toff.Rotate(rideRot);
	AuMatrix4x4 t = mount->Transform() * toff;

	// Transfer mount's movement vector to me.
	AuPoint vec = toff.UnProject(mount->Vector());

	// Pose mount.
	mount->Dismounted();

	// Place me subordinate to the mount's parent atom.
	ForcePosMove(AuAtomPos(mount->ParentId(), t), vec);
	double fallDist = FallDistance();
	if(fallDist < 0)
	{
		t.Translate(0,0,-fallDist);
		ForcePosMove(AuAtomPos(mount->ParentId(), t), vec);
	}

	// Pose my body appropriately.
	AuPose pose;
	if(!IsBodyDyingOrDead())
	{
		if(wasMating)
		{
			pose = AuPose::SitOnGround;
			SetGeneralPose(pose);
		}
		else
		{
			pose = AuPose::Stand;
			Stand();											// Live conscious creature dismounts standing up.
		}
	}
	else
		pose = SetDyingPose();									// Unconscious creature dismounts in dying pose.
	SyncController(AuSyncPose(pose, 0, 0, 0));
	SetModified();
}

int DaCreature::MountVehicle(DaVehicle* vehicle)
{
	AuPoint seatLoc;
	AuPose seatPose;
	int seatIndex = vehicle->FindEmptySeat(seatLoc, seatPose);
	if(seatIndex < 0)
		return -1;												// All seats are occupied.

	// No parent on entry happens in two situations: 1) This is a nu atom and the vehicle is its first parent atom, or 2) This
	// atom was handed off to a solar system using AdminGoTo.
	bool hadParent = Parent() ? true : false;

	// Pose body for proper camera offset calc.
	SetGeneralPose(seatPose, false);
	m_BodyLock.lockForRead();
	if(m_Body)
		seatLoc.z += m_Body->CGHeight(true);				// Move seat point up to creature waist location.
	else
		seatLoc.z += m_Animalia->CGHeight();				// Move seat point up to creature waist location.
	m_BodyLock.unlock();
	AuMatrix4x4 t;											// Identity matrix.
	t.Translate(seatLoc);

	// Position my body.
	ChangePosAndOrient(AuAtomPos(vehicle->Id(), t));
	SetVehicleSeatIndex((quint8)seatIndex);					// Remember my current vehicle seat index.
	vehicle->SetUsed();										// Enable the vehicle to decay.
	vehicle->SetModified();									// Signal the vehicle atom as modified.

	// Certain actions are not performed on nu atoms.
	if(hadParent)
	{
		SetAtRest();
		SetModified();
	}
	vehicle->UpdateDriverOnLine();
	InvalidateEnvironment();
	vehicle->InvalidateEnvironment();
	vehicle->TickTockRecursive();
	return seatIndex;
}

int DaCreature::ForceMountVehicle(DaVehicle* vehicle)
{
	AuPoint seatLoc;
	AuPose seatPose;
	int seatIndex = vehicle->FindEmptySeat(seatLoc, seatPose);
	if(seatIndex < 0)
		return -1;												// All seats are occupied.

	// No parent on entry happens in two situations: 1) This is a nu atom and the vehicle is its first parent atom, or 2) This
	// atom was handed off to a solar system using AdminGoTo.
	bool hadParent = Parent() ? true : false;

	// Pose body for proper camera offset calc.
	SetGeneralPose(seatPose, false);
	m_BodyLock.lockForRead();
	if(m_Body)
		seatLoc.z += m_Body->CGHeight(true);				// Move seat point up to creature waist location.
	else
		seatLoc.z += m_Animalia->CGHeight();				// Move seat point up to creature waist location.
	m_BodyLock.unlock();
	AuMatrix4x4 t;											// Identity matrix.
	t.Translate(seatLoc);

	// Position my body.
	ForcePosStop(AuAtomPos(vehicle->Id(), t));
	SetVehicleSeatIndex((quint8)seatIndex);					// Remember my current vehicle seat index.
	SyncController(AuSyncPose(seatPose, 0, 0, seatIndex));
	vehicle->SetUsed();										// Enable the vehicle to decay.
	vehicle->SetModified();									// Signal the vehicle atom as modified.

	// Certain actions are not performed on nu atoms.
	if(hadParent)
	{
		SetAtRest();
		SetModified();
	}
	vehicle->UpdateDriverOnLine();
	InvalidateEnvironment();
	vehicle->InvalidateEnvironment();
	vehicle->TickTockRecursive();
	return seatIndex;
}

// Called on the client.
bool DaCreature::DismountVehicle()
{
	DaVehicle* vehicle = dynamic_cast<DaVehicle*>(Parent());
	if(!vehicle)
		return false;

	// I am riding a vehicle. I must dismount it.
	// Get vehicle extents box, in the space of the vehicle.
	AuBox vehicleBox;
	vehicle->RoughExtents(vehicleBox);

	// Determine my seat location.
	AuPoint seatLoc = vehicle->SeatLoc(VehicleSeatIndex());

	// Calculate the offload location based on my look direction.
	AuPoint rideRot = Rotation();							// My rotation in the vehicle.
	AuPoint lookVec(0,1,0);									// Vector to offload location from seat location.
	lookVec = lookVec.RotateX(rideRot.x).RotateY(rideRot.y).RotateZ(rideRot.z);
	lookVec = lookVec.RotateX(CurrentHeadTilt());
	AuPoint lookLoc = seatLoc + lookVec*10000;

	// Determine which side of the extents box is intersected by the offload segment.
	//qDebug() << "DaCreature::DismountVehicle" << Id() << Name() << "vehicle inWater" << vehicle->IsInVisibleWater()
		//<< "cgUnderWater" << vehicle->IsCGUnderWater();
	AuQuadArray quads;
	vehicleBox.GetQuads(quads);
	bool hitQuad = false;
	AuPoint offloc, normal;
	int quadIndex=0;	// From AuQuad::GetQuads, 0=front, 1=left, 2=back, 3=right, 4=top, 5=bottom.
	for(AuQuadArray::const_iterator qi = quads.cbegin(); qi != quads.cend(); qi++, quadIndex++)
	{
		// Skip sides that are not valid exit directions, based on the type of vehicle.
		if(vehicle->CommodityId() == CID_SpaceRocket)
		{
			// Rocket cannot be exited from front, back or bottom.
			if(quadIndex == 0 || quadIndex == 2 || quadIndex == 5)
				continue;
		}
		else if(vehicle->CommodityId() == CID_Submarine)
		{
			// Submarine cannot be exited from the bottom.
			if(quadIndex == 5)
				continue;
		}
		else
		{
			// Most vehicles can only be exited from the sides, not the top or the bottom.
			if(quadIndex >= 4)
				break;
		}

		hitQuad = (*qi).TestSegmentIntersection(offloc, normal, seatLoc, lookLoc);
		if(hitQuad)
		{
			//qDebug() << "DaCreature::DismountVehicle" << Id() << Name() << "hitQuad" << quadIndex
				//<< (*qi).vll << (*qi).vul << (*qi).vur << (*qi).vlr;
			break;	// Found the exit side quad.
		}
	}

	if(hitQuad)
	{
		// Hit a quad of the extents box. Exit in that direction.
		// Move away from the vehicle slightly.
		//qDebug() << "DaCreature::DismountVehicle" << Id() << Name() << "seatIndex" << VehicleSeatIndex()
			//<< "seatLoc" << seatLoc << "lookVec" << lookVec << "lookLoc" << lookLoc << "offloc" << offloc
			//<< "hitQuad N" << normal << "vehicleQuads" << quads.count() << "quadIndexHit" << quadIndex;
		offloc += normal*1.5;
	}
	else
	{
		// Not likely.
		//qDebug() << "DaCreature::DismountVehicle" << Id() << Name() << "lookVec did not hit extents box, exiting the old way";
		// Calculate the offload location that corresponds to my station.
		offloc = vehicle->OffLoc(VehicleSeatIndex());		// Offload location for my seat, relative to vehicle.
	}

	// I must reparent to the parent of the vehicle for CheckCollision.
	AuAtomPos posSave(Pos());								// Save my transform so I can restore it if I can't get out.
	ChangePosAndOrient(vehicle->Pos());						// Put me where the vehicle is.
	InvalidateEnvironment();

	// Test for collision between vehicle location and offload location.
	AuPoint parOff = vehicle->Transform().Project(offloc);	// Offload location relative to parent.
#if !defined(QT_NO_DEBUG)
	if(parOff.IsNaN())
	{
		qDebug() << "DaCreature::DismountVehicle" << Id() << Name() << "parOff" << parOff << parOff.IsNaN()
			<< "offloc" << offloc << offloc.IsNaN() << "IsNaN ERROR!";
	}
#endif
	DaAtom* pHit = 0;
	AuPoint hitLoc, hitNormal;										// Return parameter from CheckCollision.
	COLLISION_TYPE testMove = CheckCollision(vehicle->Location(), parOff, pHit, hitLoc, hitNormal);
	if(testMove != CT_Miss && testMove != CT_JoinedPeer)
	{
		// The path is blocked from the vehicle location to the offload location.
		// Put me back into the vehicle since I couldn't get out.
		//qDebug() << "DaCreature::DismountVehicle" << Id() << Name() << "exit path blocked" << testMove
			//<< "by" << pHit->Id() << pHit->Name();
		ChangePosAndOrient(posSave);
		SetAtRest();
		InvalidateEnvironment();
		vehicle->UpdateDriverOnLine();
		vehicle->TickTockRecursive();
		return false;
	}

	// Nothing is blocking my path to the offload location.
	//qDebug() << "DaCreature::DismountVehicle" << Id() << Name() << "exit path from vehicle clear, nuParent"
		//<< nuParent->Id() << nuParent->Name();
	Stand();

	// Move me to the offload location that corresponds to my station, relative to nuParent, the vehicle's parent.
	AuMatrix4x4 toff;
	toff.Translate(offloc);									// Offload location, relative to vehicle.
	toff.Rotate(rideRot);									// Offload rotation, relative to vehicle.
	AuMatrix4x4 t = vehicle->Transform() * toff;			// Offload transform, relative to nuParent.
	ChangePosAndOrient(AuAtomPos(vehicle->ParentId(), t));

	// Inherit vehicle movement vector.
	AuPoint vec = toff.UnProjectVector(vehicle->Vector());
	SetVector(vec);
	SetVehicleSeatIndex(0);									// Clear current seat index.
	SetInMotion();
	InvalidateEnvironment();
	SetModified();
	TickTock();

	vehicle->SetParked(false);
	vehicle->UpdateDriverOnLine();
	vehicle->TickTockRecursive();
	vehicle->SetModified();
	return true;
}

bool DaCreature::GrabLadder()
{
	// Check for ladder when I am aboard a spacecraft.
	if(GrabLadderOnMyShip())
		return true;

	// Check for ladder when I am aboard a bd.
	if(GrabLadderOnMyBd())
		return true;

	// Check for exterior ladders on spacecraft and buildings in my environment.
	DaAtomList ships;
	FindEnvironmentAtom()->FindCapsulesRecursive(ships);
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaCapsule* v = dynamic_cast<DaCapsule*>(*si);
		if(!v || !v->IsNormal())
			continue;
		if(v->Is(ParentId()))
			continue;					// No need to recheck my parent vessel.
		if(GrabLadderOnCapsule(v))
			return true;
	}
	return false;
}

bool DaCreature::GrabLadderOnMyBd()
{
	if(!IsInBd())
		return false;

	DaBd* bd = dynamic_cast<DaBd*>(Parent());
	if(!bd)
		return false;

	// Calculate my location in the space of the bd.
	AuPoint myloc = Location();

	// The matrix returned by CanGrabLadder is relative to the bd.
	AuMatrix4x4 mLadder;
	if(!bd->CanGrabLadder(*this, myloc, mLadder))
		return false;

	// Insure there is a clear path to the creature's nu eye position. This prevents grabbing a ladder under
	// a bd and tweaking your head up into the bd.
	AuPose myPose = CurrentGeneralPose();		// Pose before grabbing ladder.
	quint16 stationId = StationId();
	AuMatrix4x4 myT = Transform();			// Transform before grabbing ladder.

	// Calculate my eye location in the space of the bd.
	AuPoint eyePos = myT.Project(CameraOffset());	// Eye position, before grabbing ladder.

	// Calculate my transformation matrix on the ladder and tentatively set my nu pose at the ladder.
	ChangePosAndOrientNotParent(AuAtomPos(bd->Id(), mLadder));
	InvalidateEnvironment();
	SetGeneralPose(AuPose::Ladder, false);

	AuPoint hitLoc, hitNormal;
	if(bd->IsPathBlocked(eyePos, Transform().Project(CameraOffset()), hitLoc, hitNormal, this))
	{
		// Eye path was blocked so restore pose and transform.
		SetGeneralPose(myPose, stationId, false);
		ChangePosAndOrientNotParent(AuAtomPos(bd->Id(), myT));
		InvalidateEnvironment();
		return false;
	}
	else
	{
		// Eye path was not blocked so keep the nu pose.
		m_BodyLock.lockForWrite();
		m_StationId = 0;
		m_tmPoseChange.Set();
		m_BodyLock.unlock();

		SetAtRest();
		InvalidateEnvironment();
		SetModified();
		return true;
	}
}

bool DaCreature::GrabLadderOnMyShip()
{
	if(!IsInSpacecraft())
		return false;

	DaCapsule* ship = dynamic_cast<DaCapsule*>(Parent());
	if(!ship)
		return false;

	// Calculate my location in the space of the ship.
	AuPoint myloc = Location();

	// The matrix returned by CanGrabLadder is relative to the ship.
	AuMatrix4x4 mLadder;
	if(!ship->CanGrabLadder(*this, myloc, mLadder))
		return false;

	// Insure there is a clear path to the creature's nu eye position. This prevents grabbing a ladder under
	// a ship and tweaking your head up into the ship.
	AuPose myPose = CurrentGeneralPose();		// Pose before grabbing ladder.
	quint16 stationId = StationId();
	AuMatrix4x4 myT = Transform();			// Transform before grabbing ladder.

	// Calculate my eye location in the space of the ship.
	AuPoint eyePos = myT.Project(CameraOffset());	// Eye position, before grabbing ladder.

	// Calculate my transformation matrix on the ladder and tentatively set my nu pose at the ladder.
	ChangePosAndOrientNotParent(AuAtomPos(ship->Id(), mLadder));
	InvalidateEnvironment();
	SetGeneralPose(AuPose::Ladder, false);

	AuPoint hitLoc, hitNormal;
	if(ship->IsPathBlocked(eyePos, Transform().Project(CameraOffset()), hitLoc, hitNormal, this))
	{
		// Eye path was blocked so restore pose and transform.
		SetGeneralPose(myPose, stationId, false);
		ChangePosAndOrientNotParent(AuAtomPos(ship->Id(), myT));
		InvalidateEnvironment();
		return false;
	}
	else
	{
		// Eye path was not blocked so keep the nu pose.
		m_BodyLock.lockForWrite();
		m_StationId = 0;
		m_tmPoseChange.Set();
		m_BodyLock.unlock();

		SetAtRest();
		InvalidateEnvironment();
		SetModified();
		return true;
	}
}

bool DaCreature::GrabLadderOnMyShipAt(const AuPoint& ladderLoc)
{
	if(!IsInSpacecraft())
		return false;

	DaCapsule* ship = dynamic_cast<DaCapsule*>(Parent());
	if(!ship)
		return false;

	// The matrix returned by CanGrabLadder is relative to the ship.
	AuMatrix4x4 mLadder;
	if(!ship->CanGrabLadderAt(*this, ladderLoc, mLadder))
		return false;
	//DHH Check distance to ladderLoc.

	// Insure there is a clear path to the creature's nu eye position. This prevents grabbing a ladder under
	// a ship and tweaking your head up into the ship.
	AuPose myPose = CurrentGeneralPose();		// Pose before grabbing ladder.
	quint16 stationId = StationId();
	AuMatrix4x4 myT = Transform();			// Transform before grabbing ladder.

	// Calculate my eye location in the space of the ship.
	AuPoint eyePos = myT.Project(CameraOffset());	// Eye position, before grabbing ladder.

	// Calculate my transformation matrix on the ladder and tentatively set my nu pose at the ladder.
	ChangePosAndOrientNotParent(AuAtomPos(ship->Id(), mLadder));
	InvalidateEnvironment();
	SetGeneralPose(AuPose::Ladder, false);

	AuPoint hitLoc, hitNormal;
	if(ship->IsPathBlocked(eyePos, Transform().Project(CameraOffset()), hitLoc, hitNormal, this))
	{
		// Eye path was blocked so restore pose and transform.
		SetGeneralPose(myPose, stationId, false);
		ChangePosAndOrientNotParent(AuAtomPos(ship->Id(), myT));
		InvalidateEnvironment();
		return false;
	}
	else
	{
		// Eye path was not blocked so keep the nu pose.
		m_BodyLock.lockForWrite();
		m_StationId = 0;
		m_tmPoseChange.Set();
		m_BodyLock.unlock();

		SetAtRest();
		InvalidateEnvironment();
		SetModified();
		return true;
	}
}

bool DaCreature::GrabLadderOnCapsule(DaCapsule* ship)
{
	Q_ASSERT(ship);
	// Calculate my location in the space of the ship.
	AuPoint myloc = EnvLocation();
	myloc = ship->EnvTransform().UnProject(myloc);

	// The matrix returned by CanGrabLadder is relative to the ship.
	AuMatrix4x4 mLadder;
	if(!ship->CanGrabLadder(*this, myloc, mLadder))
		return false;

	// Insure there is a clear path to the creature's nu eye position. This prevents grabbing a ladder under
	// a ship and tweaking your head up into the ship.
	AuPose myPose = CurrentGeneralPose();		// Pose before grabbing ladder.
	quint16 stationId = StationId();
	AuMatrix4x4 myT = Transform();			// Transform before grabbing ladder.
	DaAtom* myPar = Parent();				// Parent atom before grabbing ladder.

	// Calculate my eye location in the space of the ship.
	AuPoint eyePos = EnvTransform().Project(CameraOffset());	// Eye position, before grabbing ladder.
	eyePos = ship->EnvTransform().UnProject(eyePos);			// Eye position in the space of the ship.

	// Calculate my transformation matrix on the ladder and tentatively set my nu pose at the ladder.
	SetGeneralPose(AuPose::Ladder, false);		// Ship will not release me if I am in a ladder pose outside its hull.
	ChangePosAndOrient(AuAtomPos(ship->Id(), mLadder));
	InvalidateEnvironment();

	AuPoint hitLoc, hitNormal;
	if(ship->IsPathBlocked(eyePos, Transform().Project(CameraOffset()), hitLoc, hitNormal, this))
	{
		// Eye path was blocked so restore parent, pose and transform.
		SetGeneralPose(myPose, stationId, false);
		ChangePosAndOrient(AuAtomPos(myPar->Id(), myT));
		InvalidateEnvironment();
		return false;
	}
	else
	{
		// Eye path was not blocked so keep the nu pose and parent atom.
		m_BodyLock.lockForWrite();
		m_StationId = 0;
		m_tmPoseChange.Set();
		m_BodyLock.unlock();

		SetAtRest();
		InvalidateEnvironment();
		SetModified();
		return true;
	}
}

bool DaCreature::IsSightTarget(const DaAtom* toWho) const
{
	// Don't sight myself.
	if(toWho == this)
		return false;

	// Creatures must not be a sight target to spacecraft or cities because they block shots by bay weapons and base weapons.
	if(dynamic_cast<const DaCapsule*>(toWho))
	{
		// Guard towers do want to target creatures.
		const DaBd* bd = dynamic_cast<const DaBd*>(toWho);
		if(!bd || bd->IndustryId() != IID_BaseGuardTower)
			return false;
	}
	return true;
}

AuPose DaCreature::SetDyingPose()
{
	// Randomly pick a facing for the dying pose.
	AuPose pose;
	switch(g_Au.Dice().Roll(1,4))
	{
	case 1:
		pose = AuPose::WritheFaceDown;
		break;
	case 2:
		pose = AuPose::WritheFaceLeft;
		break;
	case 3:
		pose = AuPose::WritheFaceRight;
		break;
	case 4:
	default:
		pose = AuPose::WritheFaceUp;
		break;
	}

	bool poseChg;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChg = m_Body->Pose(pose, false, 1);
	else
		poseChg = m_Animalia->Pose(Id(), pose, false, 1);
	if(poseChg)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	if(poseChg)
	{
		ExitConsole();									// Exit console in case I'm in a game.
		DismountAllRiders();								// Dismount any atoms that are mounted on this creature.
		ForceMotion();
		SetModified();

		// When dying in a sick bay, wake the doctor in the room. He could be sleeping.
		if(IsInCapsule() && CapsuleRoomType() == ROOM_SickBay)
		{
			DaCapsule* ship = FindParentCapsuleInEnv();
			if(ship)
				ship->WakeDoctor(*this);
		}
	}
	return pose;
}

bool DaCreature::SetGeneralPose(const AuPose& pose, bool setModified)
{
	// Returns false if the body is already in the general pose.
	float speedEnc = MovementSpeedMod();
	m_BodyLock.lockForWrite();
	bool posed;
	if(m_Body)
		posed = m_Body->Pose(pose, HasItemInHand(), speedEnc);
	else
	{
		AuPose mouth = m_Animalia->CurrentMouthPose();	// Preserve mouth pose.
		posed = m_Animalia->Pose(Id(), pose, HasItemInHand(), speedEnc);
		if(mouth != AuPose::Init)
			m_Animalia->PoseMouth(Id(), mouth);
	}
	if(posed || m_StationId)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	if(!posed)
		return false;	// Pose did not change.

	// The pose actually changed.
	if(setModified)
	{
		PositionRiders();							// Rider positions can change based on my body pose.
		SetModified();
	}
	return true;	// Pose changed.
}

bool DaCreature::SetGeneralPose(const AuPose& pose, quint16 stationId, bool setModified)
{
	// Returns false if the body is already in the general pose.
	if(stationId && !pose.IsConsole() && !pose.IsTurret() && pose != AuPose::ConsoleSurgeryUnit)
		stationId = 0;	// Non-zero station id is not appropriate for the pose.
	float speedEnc = MovementSpeedMod();
	m_BodyLock.lockForWrite();
	bool posed;
	if(m_Body)
		posed = m_Body->Pose(pose, HasItemInHand(), speedEnc);
	else
	{
		AuPose mouth = m_Animalia->CurrentMouthPose();	// Preserve mouth pose.
		posed = m_Animalia->Pose(Id(), pose, HasItemInHand(), speedEnc);
		if(mouth != AuPose::Init)
			m_Animalia->PoseMouth(Id(), mouth);
	}
	if(posed || m_StationId != stationId)
	{
		m_StationId = stationId;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	if(!posed)
		return false;	// Pose did not change.

	// The pose actually changed.
	if(setModified)
	{
		PositionRiders();							// Rider positions can change based on my body pose.
		SetModified();
	}
	return true;	// Pose changed.
}

void DaCreature::Jump(float power, const AuPoint& jumpVector)
{
	float burdenSpeedMod = MovementSpeedMod();
	if(burdenSpeedMod <= 0)
		return;													// Too heavily loaded to jump at all.

	// Reduce stamina for jumping.
	// Jump strength is reduced linearly if insufficient stamina to jump at full power.
	float jumpForce;
	m_BodyLock.lockForRead();
	int stjump;
	if(m_Body)
		stjump = m_Body->StaminaToJump() * power;			// Modulate stamina needed with power applied.
	else
		stjump = m_Animalia->StaminaToJump() * power;			// Modulate stamina needed with power applied.
	m_BodyLock.unlock();
	if(m_Stamina >= stjump)
	{
		// Creature has enough stamina for full force jump.
		jumpForce = 1;
		m_Stamina -= stjump;
	}
	else
	{
		// Creature can only pull off a partial jump.
		jumpForce = (float)m_Stamina/(float)stjump;
		m_Stamina = 0;
	}
	jumpForce *= power;											// Modulate jump force with power applied.

	// Reduce jump force by burden.
	jumpForce *= burdenSpeedMod;

	// Scale jump force for reasonable power.
	jumpForce *= .75f;

	AuPoint vector = Vector();
	vector.z = 0;	// Eliminate lingering gravity portion of vector.
	vector += jumpVector * jumpForce;
	SetVector(vector);
}

// Returns the vector from my atom origin to the eye camera location of my body model.
AuPoint DaCreature::HeadOffset() const
{
	AuPoint headLoc;
	m_BodyLock.lockForRead();
	if(m_Body)
		headLoc = m_Body->HeadLocation();
	else
		headLoc = m_Animalia->HeadLocation();
	m_BodyLock.unlock();

	if(!IsFullGrown())
		return headLoc * GrowthScale(m_MinutesToFullGrown, GrowthTime());		// Scale offset according to growth factor.

	return headLoc;
}

// Returns the vector from my atom origin to the eye camera location of my body model.
AuPoint DaCreature::CameraOffset() const
{
	// Server does not load head geometry so on the server the CameraOffset results in the base of the skull location instead.
	AuPoint eyeLoc;
	if(dynamic_cast<const DaVehicle*>(Parent()))
	{
		// Vehicles place my body CG above the seat location, raised by my CGHeight to allow for my legs and body.
		m_BodyLock.lockForRead();
		if(m_Body)
			eyeLoc = m_Body->CameraLocation();
		else
			eyeLoc = m_Animalia->CameraLocation();
		m_BodyLock.unlock();
	}
	else
	{
		const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
		if(mount)
		{
			// My origin is at the saddle location when riding a mount.
			m_BodyLock.lockForRead();
			if(m_Body)
				eyeLoc = m_Body->CameraLocation();
			else
				eyeLoc = m_Animalia->CameraLocation();
			m_BodyLock.unlock();

			// When riding a mount, make adjustments to my body position to appear to stay exactly positioned at the saddle.
			// Determine proper location of mount's saddle.
			AuPoint rideLoc = mount->CGOffset();				// Riders sit at mount's body pivot point.
			float bodyThick = mount->TorsoThickness()/2;		// Allow for mount's body thickness.
			rideLoc += AuPoint(0, -bodyThick, bodyThick);		// Move back and up by mount's body thickness.

			// Creature rider positioning. Location of creature bodies when riding an animal is their waist/pivot point.
			rideLoc.z += TorsoThickness()/2;					// Move up by the rider's body thickness.
			eyeLoc += rideLoc - Location();
		}
		else
		{
			// Most creature poses have the atom origin at the head location.
			AuPoint camLoc, headLoc;
			AuPose gp;
			m_BodyLock.lockForRead();
			if(m_Body)
			{
				gp = m_Body->CurrentGeneralPose();
				camLoc = m_Body->CameraLocation();
				headLoc = m_Body->HeadLocation();
			}
			else
			{
				gp = m_Animalia->CurrentGeneralPose();
				camLoc = m_Animalia->CameraLocation();
				headLoc = m_Animalia->HeadLocation();
			}
			m_BodyLock.unlock();

			switch(gp.Pose())
			{
			default:
#if(c_OriginAtHead == 1)
				// Default placement of creature has atom origin at the head location.
				// The eye location cannot be used because the server does not load head geometry.
				// Scale offset according to growth factor.
				eyeLoc = camLoc - headLoc;
#else
				// Default placement of creature has atom origin at CG location.
				eyeLoc = camLoc;
#endif
				break;

			case AuPose::Prone:
			case AuPose::ProneCreep:
				// Prone placement of creature has atom origin at head location.
				eyeLoc = camLoc - headLoc;
				break;

			case AuPose::ConsoleSurgeryUnit:
			case AuPose::Patient_Prone:
			case AuPose::Patient_Sit:
			case AuPose::Patient_SitStationary:
			case AuPose::Patient_Stand:
			case AuPose::Patient_StandStationary:
				eyeLoc = camLoc;
				break;

			case AuPose::BerthBottomFaceDown:
			case AuPose::BerthBottomFaceLeft:
			case AuPose::BerthBottomFaceRight:
			case AuPose::BerthBottomFaceUp:
			case AuPose::BerthTopFaceDown:
			case AuPose::BerthTopFaceLeft:
			case AuPose::BerthTopFaceRight:
			case AuPose::BerthTopFaceUp:
				// In bunk, my body is placed at the center surface of the bunk.
				eyeLoc = camLoc;
				break;

			case AuPose::ConsoleTurretBottom:
			case AuPose::ConsoleTurretTop:
				// My origin is at the turret gun location.
				// Offset to the left of the gun here so my body rotates around the gun like I'm in a chair that's attached.
				eyeLoc = AuPoint(-1.25,0,0);
				break;

			case AuPose::GameTableSeat0_0:
			case AuPose::GameTableSeat1_0:
			case AuPose::GameTableSeat2_0:
			case AuPose::GameTableSeat3_0:
			case AuPose::GameTableSeat0_45:
			case AuPose::GameTableSeat1_45:
			case AuPose::GameTableSeat2_45:
			case AuPose::GameTableSeat3_45:
			case AuPose::ConsoleCommand:
			case AuPose::ConsoleNav:
			case AuPose::ConsoleEngine:
			case AuPose::ConsoleFireControl:
			case AuPose::ConsolePilot:
			case AuPose::ConsolePower:
			case AuPose::ConsoleSensor:
			case AuPose::ConsoleShield:
			case AuPose::ConsoleSurgery:
			case AuPose::ConsoleTransporter:
			case AuPose::Command_Sit:
			case AuPose::Command_SitStationary:
			case AuPose::Command_Stand:
			case AuPose::Command_StandStationary:
			case AuPose::Engine_Sit:
			case AuPose::Engine_SitStationary:
			case AuPose::Engine_Stand:
			case AuPose::Engine_StandStationary:
			case AuPose::FireControl_Sit:
			case AuPose::FireControl_SitStationary:
			case AuPose::FireControl_Stand:
			case AuPose::FireControl_StandStationary:
			case AuPose::Helm_Sit:
			case AuPose::Helm_SitStationary:
			case AuPose::Helm_Stand:
			case AuPose::Helm_StandStationary:
			case AuPose::Medic_Sit:
			case AuPose::Medic_SitStationary:
			case AuPose::Medic_Stand:
			case AuPose::Medic_StandStationary:
			case AuPose::Nav_Sit:
			case AuPose::Nav_SitStationary:
			case AuPose::Nav_Stand:
			case AuPose::Nav_StandStationary:
			case AuPose::Post_Sit:
			case AuPose::Power_Sit:
			case AuPose::Power_SitStationary:
			case AuPose::Power_Stand:
			case AuPose::Power_StandStationary:
			case AuPose::Sensor_Sit:
			case AuPose::Sensor_SitStationary:
			case AuPose::Sensor_Stand:
			case AuPose::Sensor_StandStationary:
			case AuPose::Shield_Sit:
			case AuPose::Shield_SitStationary:
			case AuPose::Shield_Stand:
			case AuPose::Shield_StandStationary:
			case AuPose::Transporter_Sit:
			case AuPose::Transporter_SitStationary:
			case AuPose::Transporter_Stand:
			case AuPose::Transporter_StandStationary:
			case AuPose::Turret_Sit:
			case AuPose::Turret_SitStationary:
			case AuPose::Turret_SitSwivel3D:
			case AuPose::Turret_Stand:
			case AuPose::Turret_StandStationary:
				// My atom origin is at the seat location.
				// Position my head directly above the seat location.
				{
					AuPoint offset(-headLoc);
					offset.z = 0;
					eyeLoc = camLoc - offset;
				}
				break;
			}
		}
	}

	if(!IsFullGrown())
		return eyeLoc * GrowthScale(m_MinutesToFullGrown, GrowthTime());		// Scale offset according to growth factor.

	return eyeLoc;
}

// Return CG height of this atom when "standing".
float DaCreature::CGHeightStanding() const
{
	if(CurrentGeneralPose() == AuPose::Stand)
		return CGHeight();

	quint8 growTime = GrowthTime();
	bool inVehicle = dynamic_cast<const DaVehicle*>(Parent());

	// Pose a body like mine to measure the camera height.
	float cghts;
	m_BodyLock.lockForRead();
	if(m_Body)
	{
		DpBody body(m_Body->Sex(), m_Body->IsLeftHanded(), m_Body->DNA());
		body.Pose(AuPose::Stand, false, 1);	// Time is not really a factor in standing pose.
		cghts = CGHeight(body, m_MinutesToFullGrown, growTime, inVehicle);
	}
	else
	{
		AuAnimalia body(*m_Animalia);
		body.Pose(Id(), AuPose::Stand, false, 1);	// Time is not really a factor in standing pose.
		cghts = CGHeight(body, m_MinutesToFullGrown, growTime, inVehicle);
	}
	m_BodyLock.unlock();
	return cghts;
}

float DaCreature::CGHeight() const
{
	quint8 growTime = GrowthTime();
	bool inVehicle = dynamic_cast<const DaVehicle*>(Parent());

	float h;
	m_BodyLock.lockForRead();
	if(m_Body)
		h = CGHeight(*m_Body, m_MinutesToFullGrown, growTime, inVehicle);
	else
		h = CGHeight(*m_Animalia, m_MinutesToFullGrown, growTime, inVehicle);
	m_BodyLock.unlock();
	return h;
}

float DaCreature::CGHeightFullGrown() const
{
	bool inVehicle = dynamic_cast<const DaVehicle*>(Parent());

	float h;
	m_BodyLock.lockForRead();
	if(m_Body)
		h = CGHeight(*m_Body, 0, 0, inVehicle);
	else
		h = CGHeight(*m_Animalia, 0, 0, inVehicle);
	m_BodyLock.unlock();
	return h;
}

AuPoint DaCreature::CGOffset() const
{
	quint8 growTime = GrowthTime();
	bool inVehicle = dynamic_cast<const DaVehicle*>(Parent());

	AuPoint o;
	m_BodyLock.lockForRead();
	if(m_Body)
		o = CGOffset(*m_Body, m_MinutesToFullGrown, growTime, inVehicle);
	else
		o = CGOffset(*m_Animalia, m_MinutesToFullGrown, growTime, inVehicle);
	m_BodyLock.unlock();
	return o;
}

// Returns the vector from my atom location to the CG of my body model.
// In a typical standing pose, this would be the reverse of the CameraOffset.
// In a straddle pose, the CG is at my body's pivot point so I don't move wierdly when looking up and down on a mount.
//static
AuPoint DaCreature::CGOffset(const DpBody& body, quint8 minutesToFullGrown, quint8 growthTime, bool isAboardVehicle)
{
	// Creatures need this adjustment due to bobbing heads while munching. If their origin is at their eye, their
	// body is constantly in motion, adjusting up and down to the terrain. Munching critters must cause no
	// server load. This offset produces a stationary location while they eat.
	// This induces an offset when changing to/from eating pose that is noticeable, as the body origin changes from its eye to
	// its pivot point and adjusts to the nu terrain altitude there. Directly above/below the eye location minimizes
	// this offset, but the body undulates back and forth wierdly as the creatures head bobs up and down. This looks really
	// strange if I am riding the creature when it starts eating.
	// I suppose the cg offset when eating/not eating is tolerable; it doesn't happen that often. The creatures kind of look
	// like they are pouncing on their food. :P
	// A better solution might be to actually change the origin of the creature by this offset when it changes to|from eating.
	if(body.IsEatingPose())
		return AuPoint(0,0);

	// When aboard a vehicle, my atom location is exactly at the seat location.
	if(isAboardVehicle)
		return AuPoint(0,0);

	switch(body.CurrentGeneralPose().Pose())
	{
	default:
#if(c_OriginAtHead == 1)
		// Default placement of creature has atom origin at the head location.
		// The eye location cannot be used because the server does not load head geometry.
		// Scale offset according to growth factor.
		if(minutesToFullGrown)
			return -body.HeadLocation() * GrowthScale(minutesToFullGrown, growthTime);
		return -body.HeadLocation();
#else
		// Default placement of creature has atom origin at CG location.
		return AuPoint(0,0);
#endif

	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Patient_Prone:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
		return AuPoint(0,0);

	case AuPose::Prone:
	case AuPose::ProneCreep:
		// Put origin at head location so body doesn't stick out over roof edge unexpectedly when sniping.
		if(minutesToFullGrown)
			return -body.HeadLocation() * GrowthScale(minutesToFullGrown, growthTime);
		return -body.HeadLocation();

	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
		// My origin is at the turret gun location.
		// Offset to the left of the gun here so my body rotates around the gun like I'm in a chair that's attached.
		return AuPoint(-1.25,0,0);

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::EyesOnly:
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		// My origin is at the seat location, either in a turret, in a bunk, or riding a mount.
		return AuPoint(0,0);

	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleSurgery:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		// My atom origin is at the seat location.
		// Position my head directly above the seat location.
		{
			AuPoint offset(-body.HeadLocation());
			offset.z = 0;
			if(minutesToFullGrown)
				return offset *= GrowthScale(minutesToFullGrown, growthTime);
			return offset;
		}
	}
	return AuPoint(0,0);
}

AuPoint DaCreature::CGOffset(const AuAnimalia& body, quint8 minutesToFullGrown, quint8 growthTime, bool isAboardVehicle)
{
	// Creatures need this adjustment due to bobbing heads while munching. If their origin is at their eye, their
	// body is constantly in motion, adjusting up and down to the terrain. Munching critters must cause no
	// server load. This offset produces a stationary location while they eat.
	// This induces an offset when changing to/from eating pose that is noticeable, as the body origin changes from its eye to
	// its pivot point and adjusts to the nu terrain altitude there. Directly above/below the eye location minimizes
	// this offset, but the body undulates back and forth wierdly as the creatures head bobs up and down. This looks really
	// strange if I am riding the creature when it starts eating.
	// I suppose the cg offset when eating/not eating is tolerable; it doesn't happen that often. The creatures kind of look
	// like they are pouncing on their food. :P
	// A better solution might be to actually change the origin of the creature by this offset when it changes to|from eating.
	if(body.IsEatingPose())
		return AuPoint(0,0);

	// When aboard a vehicle, my atom location is exactly at the seat location.
	if(isAboardVehicle)
		return AuPoint(0,0);

	switch(body.CurrentGeneralPose().Pose())
	{
	default:
#if(c_OriginAtHead == 1)
		// Default placement of creature has atom origin at the head location.
		// The eye location cannot be used because the server does not load head geometry.
		// Scale offset according to growth factor.
		if(minutesToFullGrown)
			return -body.HeadLocation() * GrowthScale(minutesToFullGrown, growthTime);
		return -body.HeadLocation();
#else
		// Default placement of creature has atom origin at CG location.
		return AuPoint(0,0);
#endif

	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Patient_Prone:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
		return AuPoint(0,0);

	case AuPose::Prone:
	case AuPose::ProneCreep:
		// Put origin at head location so body doesn't stick out over roof edge unexpectedly when sniping.
		if(minutesToFullGrown)
			return -body.HeadLocation() * GrowthScale(minutesToFullGrown, growthTime);
		return -body.HeadLocation();

	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
		// My origin is at the turret gun location.
		// Offset to the left of the gun here so my body rotates around the gun like I'm in a chair that's attached.
		return AuPoint(-1.25,0,0);

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::EyesOnly:
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		// My origin is at the seat location, either in a turret, in a bunk, or riding a mount.
		return AuPoint(0,0);

	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleSurgery:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		// My atom origin is at the seat location.
		// Position my head directly above the seat location.
		{
			AuPoint offset(-body.HeadLocation());
			offset.z = 0;
			if(minutesToFullGrown)
				return offset *= GrowthScale(minutesToFullGrown, growthTime);
			return offset;
		}
	}
	return AuPoint(0,0);
}

void DaCreature::BodyPoseAdjustTransform(AuMatrix4x4& tBody) const
{
	if(HasDNA3())
	{
		switch(CurrentGeneralPose().Pose())
		{
		default:
			break;
		case AuPose::EyesOnly:
			tBody.RotateX(CurrentHeadTilt());
			break;
		case AuPose::BerthBottomFaceLeft:
		case AuPose::BerthTopFaceLeft:
		case AuPose::DeadFaceLeft:
		case AuPose::WritheFaceLeft:
			tBody.RotateY(PI/2.);
			break;
		case AuPose::BerthBottomFaceRight:
		case AuPose::BerthTopFaceRight:
		case AuPose::DeadFaceRight:
		case AuPose::WritheFaceRight:
			tBody.RotateY(-PI/2.);
			break;
		case AuPose::BerthBottomFaceUp:
		case AuPose::BerthTopFaceUp:
		case AuPose::DeadFaceUp:
		case AuPose::WritheFaceUp:
		case AuPose::ReceptiveMate:
		case AuPose::MateOnBottom:
		case AuPose::Patient_Prone:
			tBody.RotateY(PI);
			break;
		}
	}
	else
	{
		switch(CurrentGeneralPose().Pose())
		{
		default:
			break;
		case AuPose::EyesOnly:
			tBody.RotateX(CurrentHeadTilt());
			break;
		}
	}
}

bool DaCreature::TiltHeadTo(float angle)
{
	// Constrain head tilt when in a top or bottom turret.
	if(CurrentGeneralPose() == AuPose::ConsoleTurretBottom)
	{
		// Bottom turret. Prevent head tilting too high.
		if(angle > 0)
			angle = 0;
	}
	else if(CurrentGeneralPose() == AuPose::ConsoleTurretTop)
	{
		// Top turret. Prevent head tilting too low.
		if(angle < 0)
			angle = 0;
	}

	// Return false if there is no change to the head tilt.
	if(angle == CurrentHeadTilt())
		return false;

	// Change the head tilt of my body.
	float speedEnc = MovementSpeedMod();
	m_BodyLock.lockForWrite();

	// Head tilt sometimes changes my CG. Kick myself into motion if it's ok to move.
	bool setInMotion = false;
	if(m_Body)
	{
		m_Body->TiltHeadsTo(angle, 0);
		m_tmHeadChange.Set();
		if(!m_Body->IsLockedPose() &&
				!m_Body->IsLadderPose() &&
				!m_Body->IsEatingPose() &&
				!dynamic_cast<const DaCreature*>(Parent()) && 
				!dynamic_cast<const DaVehicle*>(Parent()))
		{
			m_Body->AnimatePose(HasItemInHand(), speedEnc);	// Update body head position for CG height calc.
			setInMotion = true;
		}
	}
	else
	{
		m_Animalia->TiltHeadsTo(angle, 0);
		m_tmHeadChange.Set();
		if(!m_Animalia->IsLockedPose() &&
				!m_Animalia->IsLadderPose() &&
				!m_Animalia->IsEatingPose() &&
				!dynamic_cast<const DaCreature*>(Parent()) && 
				!dynamic_cast<const DaVehicle*>(Parent()))
		{
			//DHH animalia m_Animalia->AnimatePose(HasItemInHand(), speedEnc);	// Update body head position for CG height calc.
			m_Animalia->AnimatePose(Id(), AuElapsedTimer::Now());	// Update body head position for CG height calc.
			setInMotion = true;
		}
	}
	m_BodyLock.unlock();
	if(setInMotion)
	{
		SetInMotion();
		// This causes tick problems. TickTock();
	}
	return true;
}

AuPoint DaCreature::TargetLocation() const
{
	// Target the torso of this creature.
	// CGOffset is the vector to my body pivot point.
	// m_Body.TargetLocation is the vector from my body pivot point to my body target location.
	AuPoint tl;
	bool ddb;
	m_BodyLock.lockForRead();
	if(m_Body)
	{
		tl = m_Body->TargetLocation();
		ddb = m_Body->IsBerthPose() || m_Body->IsDead() || m_Body->IsDying();
	}
	else
	{
		tl = m_Animalia->TargetLocation();
		ddb = m_Animalia->IsBerthPose() || m_Animalia->IsDead() || m_Animalia->IsDying();
	}
	m_BodyLock.unlock();
	if(ddb)
	{
		AuMatrix4x4 tbody;
		BodyPoseAdjustTransform(tbody);
		return CGOffset() + tbody.Project(tl);
	}
	return CGOffset() + tl;
}

void DaCreature::SetEmpireId(quint32 idEmpire)
{
	if(Allegiance() != idEmpire)
	{
		DaUnit::SetEmpireId(idEmpire);

		// Change the empire of the vehicle I am aboard, if I am the driver.
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(Parent());
		if(vehicle && vehicle->Driver() == this)
			vehicle->SetEmpireId(idEmpire);
	}
	else
	{
		// Change the empire of my mount, if it is an animal. A mount could be a player or a citizen.
		DaAnimal* mount = dynamic_cast<DaAnimal*>(Parent());
		if(mount)
			mount->SetEmpireId(idEmpire);
	}
}

// Returns current weight of burden in lbs.
// Includes weight of riders' burden when mounted.
// As creatures do not carry inventory, this base class only returns the combined burdens of riders, if any.
float DaCreature::Burden() const
{
	float burden = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		const DaCreature* rider = dynamic_cast<const DaCreature*>(*ai);
		if(!rider)
			continue;

		// The burden of invisible agents is not added to the mount.
		if(rider->IsEthereal())
			continue;

		// The actual weight of the rider is not added for game playability. Otherwise people are too slow to carry
		// someone piggyback.
		burden += rider->Burden();
	}
	return burden;
}

bool DaCreature::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(cameraEnvLoc);
	return DistanceToAtom2(sceneAtom, 0) < (200*FeetPerMeter() * 200*FeetPerMeter());
}

bool DaCreature::ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return false;

	// 'subatom' is riding this creature. Pass the constraint request up to my parent atom.
	AuMatrix4x4 m = subatom->Transform() * camera;
	if(!parent->ConstrainCamera(this, m))
		return false;

	// Remove the subatom's transform from the modified result.
	camera = subatom->Transform().Inverse() * m;
	return true;
}

// Add a visual and/or audio effect to this atom.
// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
void DaCreature::AddEffect(AuEffect& effect)
{
	if(IsExoskeletal())
	{
		if(effect.Visual() == VISUAL_HitBludgeon)
			effect.SetVisual(VISUAL_HitBludgeonBug);
		else if(effect.Visual() == VISUAL_HitPierce)
			effect.SetVisual(VISUAL_HitPierceBug);
		else if(effect.Visual() == VISUAL_HitSlash)
			effect.SetVisual(VISUAL_HitSlashBug);
	}
	else
	{
		if(effect.Visual() == VISUAL_HitBludgeon)
			effect.SetVisual(VISUAL_HitBludgeonBlood);
		else if(effect.Visual() == VISUAL_HitPierce)
			effect.SetVisual(VISUAL_HitPierceBlood);
		else if(effect.Visual() == VISUAL_HitSlash)
			effect.SetVisual(VISUAL_HitSlashBlood);
	}

	DaUnit::AddEffect(effect);
}

AuStation::TYPE DaCreature::ConsoleType() const
{
	if(!HasConsole())
		return AuStation::TYPE_Count;
	AuStation::TYPE type;
	m_ConsoleLock.lockForRead();
	if(m_Console)
		type = m_Console->Type();
	else
		type = AuStation::TYPE_Count;
	m_ConsoleLock.unlock();
	return type;
}

// Cleans up render objects of previous item.
void DaCreature::SetItemInHand(AuGLEffects& re, AuSound* al, const AuItem* item)
{
	// Try to refresh current item to pick up changes in light state, etc.
	m_ItemInHandLock.lockForWrite();
	if(item)
	{
		if(m_ItemInHand && m_ItemInHand->CommodityId() == item->CommodityId())
		{
			m_ItemInHand->Refresh(item, &re, al);
			m_ItemInHandLock.unlock();
			return;
		}
	}

	// Prepare to be replaced.
	if(m_ItemInHand)
		m_ItemInHand->DeleteRenderObjects(re, al);
	m_ItemInHandLock.unlock();

	// Replace or remove my item in hand.
	SetItemInHand(item);
}

// Just replaces the item.
void DaCreature::SetItemInHand(const AuItem* item)
{
	// Delete the current item in hand.
	m_ItemInHandLock.lockForWrite();
	if(m_ItemInHand)
		delete m_ItemInHand;

	// Set the nu item in hand, possibly null.
	if(item)
		m_ItemInHand = item->Clone();
	else
		m_ItemInHand = 0;
	m_ItemInHandLock.unlock();
}

QString DaCreature::ConsoleName() const
{
	AuStation::TYPE stationType = AuStation::TYPE_Count;
	DpStation* console = LockConsoleForRead();
	if(console)
		stationType = console->Type();
	UnlockConsole();
	if(IsInSpacecraft())
		return AuStation::NameInSpacecraft(stationType);
	else
		return AuStation::NameInBuilding(stationType);
}

// 'destArc' is n/a when the destination system is not a ringworld.
void DaCreature::HengeTeleport(const AuId& fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels, const AuId& destSysId,
		quint8 destSun, quint8 destArc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "DaCreature::HengeTeleport" << Id() << Name() << "henge teleport to system" << destSysId << "sun" << destSun
			<< "arc" << destArc << "panels" << shadowPanels;

	// Prepare for teleport.
	SetTraveling();											// I hope this includes my riders.
	SetVector(AuPoint(0,0));								// Stop moving!
	SetModified();

	emit HengeTeleportSignal(Id(), fromSysId, fromSun, fromArc, shadowPanels, destSysId, destSun, destArc);
}

// Checks for an item in my dominant hand.
bool DaCreature::HasItemInHand(COMMODITYID cid) const
{
	bool isItem = false;
	m_ItemInHandLock.lockForRead();
	if(m_ItemInHand)
	{
		if(m_ItemInHand->CommodityId() == cid)
			isItem = true;
	}
	m_ItemInHandLock.unlock();
	return isItem;
}

// Checks for a specific item in my dominant hand.
// 'inscription' is only compared if 'inscription' is not blank.
bool DaCreature::HasItemInHandSpecific(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		const QString& inscription) const
{
	bool isItem = false;
	m_ItemInHandLock.lockForRead();
	if(m_ItemInHand)
	{
		if(inscription.isEmpty())
		{
			if(m_ItemInHand->CommodityId() == cid &&
					m_ItemInHand->Quality() == quality &&
					m_ItemInHand->EmpireOfOrigin() == empireOfOrigin)
				isItem = true;
		}
		else
		{
			if(m_ItemInHand->CommodityId() == cid &&
					m_ItemInHand->Quality() == quality &&
					m_ItemInHand->EmpireOfOrigin() == empireOfOrigin &&
					m_ItemInHand->Inscription() == inscription)
				isItem = true;
		}
	}
	m_ItemInHandLock.unlock();
	return isItem;
}

/*
bool DaCreature::CanGrabLadder() const
{
	if(InLadderWell())
	{
		// Check for ladder when I am aboard a spacecraft.
		// Query environment has already determined if I am in range of a ladder on my own ship.
		if(CanGrabLadderOnMyShip())
			return true;

		// Check for ladder when I am aboard a bd.
		// Query environment has already determined if I am in range of a ladder on my own bd.
		if(CanGrabLadderOnMyBd())
			return true;
	}

	// Check for ladders on buildings.
	if(CanGrabLadderOnMyWorld())
		return true;

	// Check for exterior ladders on spacecraft in my environment.
	DaAtomList ships;
	FindEnvironmentAtom()->FindCapsulesRecursive(ships);
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaCapsule* v = dynamic_cast<DaCapsule*>(*si);
		if(!v || !v->IsNormal())
			continue;
		if(v->Is(ParentId()))
			continue;					// No need to recheck my parent vessel.
		if(CanGrabLadderOnCapsule(v))
			return true;
	}
	return false;
}
*/

bool DaCreature::CanGrabLadderOnMyBd() const
{
	DaBd* bd = dynamic_cast<DaBd*>(Parent());
	if(!bd)
		return false;

	// Calculate my location in the space of the bd.
	AuPoint myloc = Location();

	// The matrix returned by CanGrabLadder is relative to the bd.
	AuMatrix4x4 mLadder;
	return bd->CanGrabLadder(*this, myloc, mLadder);
}

bool DaCreature::CanGrabLadderOnMyShip() const
{
	DaCapsule* ship = dynamic_cast<DaCapsule*>(Parent());
	if(!ship)
		return false;

	// Calculate my location in the space of the ship.
	AuPoint myloc = Location();

	// The matrix returned by CanGrabLadder is relative to the ship.
	AuMatrix4x4 mLadder;
	return ship->CanGrabLadder(*this, myloc, mLadder);
}

bool DaCreature::CanGrabLadderOnCapsule(DaCapsule* capsule) const
{
	Q_ASSERT(capsule);
	// Calculate my location in the space of the capsule.
	AuPoint myloc = EnvLocation();
	myloc = capsule->EnvTransform().UnProject(myloc);

	// The matrix returned by CanGrabLadder is relative to the capsule.
	AuMatrix4x4 mLadder;
	return capsule->CanGrabLadder(*this, myloc, mLadder);
}

quint16 DaCreature::AtTransporterWhich() const
{
	if(AuStation::IsTransporter(ConsoleType()))
		return StationId();
	return DaUnit::AtTransporterWhich();
}

bool DaCreature::KeepsAutomaticDoorsOpen() const
{
	if(!IsNormal())
		return false;
	// Don't cause automatic doors to stay open when sitting at a nearby console.
	return !IsLockedPose();
}

bool DaCreature::OpensAutomaticDoors() const
{
	// Don't cause automatic doors to open when moving in a nearby console.
	return !IsLockedPose();
}

AuStation::OCCUPANT_POSITION DaCreature::ConsoleOccupantPosition() const
{
	return AuStation::OccPosForPose(CurrentGeneralPose());
}

bool DaCreature::ConsoleScreenQuad(AuQuad& screen) const
{
	const DaCapsule* cap = dynamic_cast<const DaCapsule*>(FindParentCapsuleInEnv());
	if(!cap)
		return false;
	return cap->StationScreen(StationId(), screen);
}

AuPoint DaCreature::SceneCameraLocation() const
{
	if(IsTurretIntegratedPose())
	{
		const DaSc* sc = dynamic_cast<const DaSc*>(Parent());
		if(sc)
		{
			// Aboard ship with new style turrets.
			float baseRot = Rotation().z;
			float gunRot = CurrentHeadTilt();
			AuMatrix4x4 mTur;
			sc->TurretSightMatrix(StationId(), baseRot, gunRot, mTur);
			AuPoint sightLoc = mTur.Translation();		// Relative to the ship.
			return Transform().UnProject(sightLoc);		// Relative to my body.
		}
	}

	// Not in a turret so use my body's camera location.
	return CameraOffset();
}

AuPose DaCreature::SetDeadPose()
{
	// Pick a dead facing that matches the dying pose, if it was dying before entry to this function anyway.
	AuPose pose;
	switch(CurrentGeneralPose().Pose())
	{
	case AuPose::WritheFaceDown:
		pose = AuPose::DeadFaceDown;
		break;
	case AuPose::WritheFaceLeft:
		pose = AuPose::DeadFaceLeft;
		break;
	case AuPose::WritheFaceRight:
		pose = AuPose::DeadFaceRight;
		break;
	case AuPose::WritheFaceUp:
		pose = AuPose::DeadFaceUp;
		break;
	default:
		// Randomly pick a facing for my suddenly dead pose.
		switch(g_Au.Dice().Roll(1,4))
		{
		default:	// Eliminates compiler warning in release build.
		case 1:
			pose = AuPose::DeadFaceDown;
			break;
		case 2:
			pose = AuPose::DeadFaceLeft;
			break;
		case 3:
			pose = AuPose::DeadFaceRight;
			break;
		case 4:
			pose = AuPose::DeadFaceUp;
			break;
		}
		break;
	}

	bool poseChg;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChg = m_Body->Pose(pose, false, 1);
	else
		poseChg = m_Animalia->Pose(Id(), pose, false, 1);
	if(poseChg)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	if(poseChg)
	{
		ExitConsole();									// Exit console in case I'm in a game.
		DismountAllRiders();								// Dismount any atoms that are mounted on this creature.
		ForceMotion();
		SetModified();
	}
	return pose;
}

double DaCreature::FireRadius() const
{
	return TorsoThickness() * 2;
}

void DaCreature::Thought(const QString& text) const
{
	AuCommThought msg(text, Id(), Name());
	g_Au.SendAtomChat(msg);
}

bool DaCreature::Station(AuStation& station) const
{
	const DaCapsule* capsule = FindParentCapsuleInEnv();
	if(!capsule)
		return false;
	return capsule->Station(*this, station);
}

quint64 DaCreature::MemoryUsage() const
{
	quint64 mem = DaUnit::MemoryUsage();
	mem += sizeof(DaCreature) - sizeof(DaUnit);
	if(m_Body)
		mem += m_Body->MemoryUsage();
	//DHH animalia
	if(m_Console)
		mem += m_Console->MemoryUsage();
	if(m_ItemInHand)
		mem += m_ItemInHand->MemoryUsage();
	if(m_Ordnance)
		mem += m_Ordnance->MemoryUsage();
	return mem;
}

bool DaCreature::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	if(IsBodyDyingOrDead())
		return false;
	return DaUnit::IsGuardTowerTarget(gunnerEmpire, gunnerPosture, gunLoc, gunRange, targetLoc);
}

bool DaCreature::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	if(IsBodyDyingOrDead())
		return false;
	return DaUnit::IsShipTurretTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange,
			gunSysVec, targetSysLoc);
}

double DaCreature::FallSafeSpeed() const
{
	bool flies;
	bool jumps;
	bool walks;
	m_BodyLock.lockForRead();
	if(m_Body)
	{
		flies = m_Body->DNA().CanFly();
		jumps = m_Body->DNA().CanJump();
		walks = m_Body->DNA().LivesOnLand;
	}
	else
	{
		flies = m_Animalia->IsFlyer();
		jumps = m_Animalia->IsLeaper();
		walks = m_Animalia->LivesOnLand();
	}
	m_BodyLock.unlock();

	// Jumpers can withstand a lot of shock due to falls.
	if(jumps)
		return 20 * FeetPerMeter();

	// Flyers can withstand some shock due to falls.
	if(flies)
		return 15 * FeetPerMeter();

	// Walkers and swimmers can withstand a little shock due to falls.
	if(walks)
		return 10 * FeetPerMeter();

	// Diggers and swimmers can withstand very little shock due to falls.
	return 5 * FeetPerMeter();
}

// Find atoms that might cause shields to be raised.
void DaCreature::FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
		SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
		double& targetRange) const
{
	// Shields are not effective against creatures, even those carrying bombs.
	Q_UNUSED(shipId);
	Q_UNUSED(shipDomain);
	Q_UNUSED(shipWorldId);
	Q_UNUSED(shipEmpire);
	Q_UNUSED(shieldPolicy);
	Q_UNUSED(shipSysLoc);
	Q_UNUSED(sensorRange);
	Q_UNUSED(targetAtom);
	Q_UNUSED(targetRange);
}

void DaCreature::EmpireDeleted(quint32 empireId)
{
	if(EmpireId() != empireId)
		return;
	DismountAllRiders();
	Kill();
}

bool DaCreature::IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho)
{
	Q_UNUSED(toWho);
	AuPoint bodyRadii;
	bodyRadii.x = TorsoThickness()/2 + 1;
	bodyRadii.y = bodyRadii.x;
	bodyRadii.z = HeadOffset().z + 1;
	AuPoint cen(CGOffset());
	AuPoint isect1, isect2;
	int isect = oloc.IntersectEllipsoid(cen, bodyRadii, nloc, isect1, isect2);
	if(isect <= 0)
		return false;
	// The path intersects my body target elipsoid.
	hitLoc = isect1;
	hitNormal = isect1 - cen;
	hitNormal.Unit();
	return true;
}

DaCreature* DaCreature::FindCreatureRecursive()
{
	//if(!IsNormal())
		//return 0;
	return this;
}

bool DaCreature::IsMating() const
{
	return CurrentGeneralPose().IsMating();
}

AuPoint DaCreature::ButtonSoundLoc() const
{
	return HeadLocation();
}

quint16 DaCreature::RemoteCameraId() const
{
	quint16 remoteCameraId = 0;
	DpStation* station = LockConsoleForRead();
	if(station)
		remoteCameraId = station->RemoteCameraId();
	UnlockConsole();
	return remoteCameraId;
}

bool DaCreature::DoGetRoomUp() const
{
	return IsLockedPose() || IsClimbingLadder();
}

DaAtom::ATOM_TYPE DaCreature::AtomType() const
{
	return ATOM_Creature;
}

void DaCreature::AnimatePose(bool bendDominantArm, float speedEnc)
{
	m_BodyLock.lockForWrite();
	if(m_Body)
		m_Body->AnimatePose(bendDominantArm, speedEnc);
	else
		m_Animalia->AnimatePose(Id(), AuElapsedTimer::Now());	//DHH corpus
	m_BodyLock.unlock();
}

float DaCreature::CycleSpeed() const
{
	float cs;
	m_BodyLock.lockForRead();
	if(m_Body)
		cs = m_Body->DNA().CycleSpeed();
	else
		cs = m_Animalia->CycleSpeed();
	m_BodyLock.unlock();
	return cs;
}

AuSpecies DaCreature::Species() const
{
	AuSpecies species;
	m_BodyLock.lockForRead();
	if(m_Body)
		species = m_Body->DNA().Species();
	else
		species = m_Animalia->DNA().Species();
	m_BodyLock.unlock();
	return species;
}

QString DaCreature::SpeciesName() const
{
	return Species().NameOrDesc();
}

int DaCreature::StrengthMod() const
{
	int strMod;
	if(HasDNA3())
		strMod = (DNA3().Strength - 7) * 4;
	else
		strMod = 0;	//DHH animalia
	return strMod;
}

float DaCreature::ThrowForce() const
{
	float throwForce;
	if(HasDNA3())
		throwForce = (DNA3().Strength + 1) / 6 * 43.56;     // result in ft lbs per second
	else
		throwForce = 50;	//DHH animalia
	return throwForce;
}

int DaCreature::FightReflex() const
{
	int fr;
	if(HasDNA3())
		fr = DNA3().FightReflex;
	else
		fr = 0;	//DHH animalia
	return fr;
}

int DaCreature::FlightReflex() const
{
	int fr;
	if(HasDNA3())
		fr = DNA3().FlightReflex;
	else
		fr = 0;	//DHH animalia
	return fr;
}

float DaCreature::JumpForce() const
{
	float v;
	if(HasDNA3())
		v = DNA3().JumpForce();
	else
		v = m_Animalia->JumpForce();
	return v;
}

float DaCreature::SightRange() const
{
	if(HasDNA3())
		return DNA3().SightRange();
	else
		return m_Animalia->SightRange();
}

float DaCreature::SmellRange() const
{
	if(HasDNA3())
		return DNA3().SmellRange();
	else
		return m_Animalia->SmellRange();
}

float DaCreature::HearingRange() const
{
	if(HasDNA3())
		return DNA3().HearingRange();
	else
		return m_Animalia->HearingRange();
}

float DaCreature::FieldOfViewAngle() const
{
	if(HasDNA3())
		return DNA3().FieldOfViewAngle();
	else
		return m_Animalia->FieldOfViewAngle();
}

bool DaCreature::IsFlyer() const
{
	if(HasDNA3())
		return DNA3().CanFly();
	else
		return m_Animalia->IsFlyer();
}

bool DaCreature::LivesOnLand() const
{
	if(HasDNA3())
		return DNA3().LivesOnLand;
	else
		return m_Animalia->LivesOnLand();
}

bool DaCreature::LivesInAir() const
{
	if(HasDNA3())
		return DNA3().LivesInAir;
	else
		return m_Animalia->LivesInAir();
}

void DaCreature::SetDNA(const AuDNABase& dna)
{
	// Get settings from current body if possible.
	AuDNABase::CHROMATID chroma;
	HANDEDNESS hand;
	short int hp;
	m_BodyLock.lockForRead();
	if(m_Animalia)
	{
		chroma = m_Animalia->Chromatid();
		hand = m_Animalia->Handedness();
		hp = m_Animalia->CurrentHitPoints();
	}
	else if(m_Body)
	{
		chroma = m_Body->Chromatid();
		hand = m_Body->IsLeftHanded() ? LeftHanded : RightHanded;
		hp = m_Body->CurrentHitPoints();
	}
	else
	{
		chroma = AuDNABase::Alpha;
		hand = RightHanded;
		hp = 0;
	}
	m_BodyLock.unlock();

	try
	{
		const AuDNA& dna4 = dynamic_cast<const AuDNA&>(dna);
		AuPose gp(AuPose::Stand);
		m_BodyLock.lockForWrite();
		if(!m_Animalia)
		{
			if(m_Body)
				gp = m_Body->CurrentGeneralPose();
			m_Animalia = new AuAnimalia(dna4, chroma, hand, GrowthStage());
		}
		else
		{
			gp = m_Animalia->CurrentGeneralPose();
			m_Animalia->SetDNA(dna4, chroma);
		}
		m_Animalia->SetHitPoints(hp);
		if(m_Body)
		{
			delete m_Body;
			m_Body = 0;
		}
		m_tmBodyChange.Set();

		m_Animalia->Pose(Id(), gp, false, 1);
		m_tmPoseChange.Set();
		m_BodyLock.unlock();
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		try
		{
			const DhDNA& dna3 = dynamic_cast<const DhDNA&>(dna);
			m_BodyLock.lockForWrite();
			if(m_Body)
				m_Body->SetDNA(dna3, 1);
			else
				m_Body = new DpBody((chroma == AuDNABase::Alpha) ? Male : Female, hand == LeftHanded, dna3);
			m_Body->SetHitPoints(hp);
			if(m_Animalia)
			{
				delete m_Animalia;
				m_Animalia = 0;
			}
			m_tmBodyChange.Set();
			m_BodyLock.unlock();
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
}

//static
float DaCreature::CGHeight(const AuAnimalia& body, quint8 minutesToFullGrown, quint8 growthTime, bool isAboardVehicle)
{
	// Scale height according to growth factor.
	if(minutesToFullGrown)
		return body.CGHeight() * GrowthScale(minutesToFullGrown, growthTime) -
				CGOffset(body, minutesToFullGrown, growthTime, isAboardVehicle).z;
	return body.CGHeight() - CGOffset(body, minutesToFullGrown, growthTime, isAboardVehicle).z;
}

AuPoint DaCreature::RightHandLoc() const
{
	AuPoint l;
	m_BodyLock.lockForRead();
	if(m_Body)
		l = m_Body->CurrentRightHandLoc();
	else
	{
		AuMatrix4x4 m = m_Animalia->RightHandMatrix();
		l = m.Translation();
	}
	m_BodyLock.unlock();

	if(!IsFullGrown())
		l *= GrowthScale();	// Scale hand loc according to growth factor.

	return l + CGOffset();
}

AuPoint DaCreature::LeftHandLoc() const
{
	AuPoint l;
	m_BodyLock.lockForRead();
	if(m_Body)
		l = m_Body->CurrentLeftHandLoc();
	else
	{
		AuMatrix4x4 m = m_Animalia->LeftHandMatrix();
		l = m.Translation();
	}
	m_BodyLock.unlock();

	if(!IsFullGrown())
		l *= GrowthScale();	// Scale hand loc according to growth factor.

	return l + CGOffset();
}

AuPoint DaCreature::HeadLocation() const
{
	AuPoint l;
	m_BodyLock.lockForRead();
	if(m_Body)
		l = m_Body->HeadLocation();
	else
		l = m_Animalia->HeadLocation();
	m_BodyLock.unlock();
	return l;
}

float DaCreature::BiteRange() const
{
	float biteRange;
	m_BodyLock.lockForRead();
	if(m_Body)
		biteRange = m_Body->BiteRange();
	else
		biteRange = m_Animalia->BiteRange();
	m_BodyLock.unlock();

	if(!IsFullGrown())
		biteRange *= GrowthScale();	// Scale bite range according to growth factor.

	// Body measures bite range from pivot point.
	biteRange -= CGOffset().Length_2D();
	if(biteRange < 6)
		biteRange = 6;

	return biteRange;
}

float DaCreature::BurdenMax() const
{
	float bm;
	m_BodyLock.lockForRead();
	if(m_Body)
	{
		int strength = m_Body->DNA().Strength;
		m_BodyLock.unlock();
		strength += MaxHitPoints()/10 + 1;
		bm = (float)(strength * 25);
	}
	else
	{
		bm = m_Animalia->BurdenMax();
		m_BodyLock.unlock();
	}
	return bm;
}

AuPoint DaCreature::GrowthScaleBody() const
{
	float scale = GrowthScale();
	if(scale >= 1.f)
		return AuPoint(1,1,1);

	bool isHoriz;
	m_BodyLock.lockForRead();
	if(m_Body)
		isHoriz = m_Body->IsPronePose();
	else
		isHoriz = m_Animalia->IsPronePose();
	m_BodyLock.unlock();

	if(isHoriz)
	{
		float scxz = scale * 2.f;
		if(scxz > 1)
			scxz = 1;
		return AuPoint(scxz, scale, scxz);
	}
	else
	{
		float scxy = scale * 2.f;
		if(scxy > 1)
			scxy = 1;
		return AuPoint(scxy, scxy, scale);
	}
}

quint8 DaCreature::GrowthTime() const
{
	quint8 growTime;
	m_BodyLock.lockForRead();
	if(m_Body)
	{
		unsigned int sz = m_Body->DNA().OverallSize;
		growTime = (quint8)((sz + 1) * 64 / 8 - 1);	// Minutes to full growth.
	}
	else
		growTime = m_Animalia->GrowthTime();
	m_BodyLock.unlock();
	return growTime;
}

bool DaCreature::IsEatingPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsEatingPose();
	else
		is = m_Animalia->IsEatingPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsInBerth() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsBerthPose();
	else
		is = m_Animalia->IsBerthPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsMovingPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsMovingPose();
	else
		is = m_Animalia->IsMovingPose();
	m_BodyLock.unlock();
	return is;
}

// True if this creature looks like it could be a rat.
bool DaCreature::LooksLikeVermin() const
{
	if(!IsWild())
		return false;

	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->DNA().LooksLikeVermin();
	else
		is = m_Animalia->LooksLikeVermin();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::PoseAttackUnderGround()
{
	bool poseChanged;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChanged = m_Body->PoseAttackUnderGround();
	else
		poseChanged = m_Animalia->PoseAttackUnderGround(Id());
	if(poseChanged)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();

	return poseChanged;
}

bool DaCreature::PoseAttackUnderWater()
{
	bool poseChanged;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChanged = m_Body->PoseAttackUnderWater();
	else
		poseChanged = m_Animalia->PoseAttackUnderWater(Id());
	if(poseChanged)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	return poseChanged;
}

bool DaCreature::PoseAttackHovering()
{
	bool poseChanged;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChanged = m_Body->PoseAttackHovering();
	else
		poseChanged = m_Animalia->PoseAttackHovering(Id());
	if(poseChanged)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();

	return poseChanged;
}

bool DaCreature::PoseAttackFlying()
{
	bool poseChanged;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChanged = m_Body->PoseAttackFlying();
	else
		poseChanged = m_Animalia->PoseAttackFlying(Id());
	if(poseChanged)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	return poseChanged;
}

bool DaCreature::PoseAttackStanding()
{
	bool poseChanged;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChanged = m_Body->PoseAttackStanding();
	else
		poseChanged = m_Animalia->PoseAttackStanding(Id());
	if(poseChanged)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	return poseChanged;
}

bool DaCreature::PoseBurrow(float speedEnc)
{
	bool poseChanged;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChanged = m_Body->PoseBurrow(speedEnc);
	else
		poseChanged = m_Animalia->PoseBurrow(Id(), speedEnc);
	if(poseChanged)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	return poseChanged;
}

bool DaCreature::PoseEatOffGround()
{
	bool poseChanged;
	m_BodyLock.lockForWrite();
	if(m_Body)
		poseChanged = m_Body->PoseEatOffGround();
	else
		poseChanged = m_Animalia->PoseEatOffGround(Id());
	if(poseChanged)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();
	return poseChanged;
}

bool DaCreature::PoseVisage(const AuVisage& start, const AuVisage& finish)
{
	// n/a for DpBody
	bool poseChanged = false;
	m_BodyLock.lockForWrite();
	if(m_Animalia)
	{
		if(start.MouthPose() != m_Animalia->CurrentMouthPose())
		{
			m_Animalia->PoseMouth(Id(), start.MouthPose());
			poseChanged = true;
		}
	}
	if(poseChanged)
		m_tmPoseChange.Set();
	m_MouthAnimDur = start.DurationMs();
	m_MouthAnimStart.restart();		// Reset at every mouth pose change.
	m_MouthFinish = finish;

	// Remember last stationary finish mouth pose set. That is our new current mouth pose.
	if(finish.MouthPose().IsMouthStationary())
		m_MouthStationary = finish.MouthPose();
	//qDebug() << "DaCreature::PoseMouth" << *this << m_Animalia->CurrentMouthPose() << m_MouthAnimDur << m_MouthFinish;
	m_BodyLock.unlock();
	return poseChanged;
}

AuTime DaCreature::SpecialWeaponUseTime() const
{
	AuTime tm;
	m_BodyLock.lockForRead();
	if(m_Body)
		tm = m_Body->SpecialWeaponUseTime();
	else
		tm = m_Animalia->SpecialWeaponUseTime();
	m_BodyLock.unlock();
	return tm;
}

bool DaCreature::CanJump() const
{
	// Cannot jump from a console or while already jumping or flying.
	if(IsLockedPose() || IsJumping() || IsFlying())
		return false;

	// Cannot jump when moving too fast.
	float maxs;
	m_BodyLock.lockForRead();
	if(m_Body)
		maxs = m_Body->MaxSpeedToJump();
	else
		maxs = m_Animalia->MaxSpeedToJump();
	m_BodyLock.unlock();
	if(fabs(Vector().y) > maxs)
		return false;

	if(IsOnGround() || IsLadderPose())
		return true;										// Jump force can be applied to the ground or the ladder.

	// No friction contact. What about nearness to the ground? It sucks to not be able to jump when you break free of the
	// ground by a smidgen. Fall distance is only valid in atmosphere range of a planet and when aboard a spacecraft.
	// No jumping in design environment. When the ship is solid, they can be in that ship and will be able to jump here.
	if(((IsInAtmosphere() && !InDesignEnvironment()) || IsInSpacecraft()) && FallDistance() < 1)
	{
		//if(Is(AuId::Haxus()))
			//qDebug() << "DaCreature::CanJump" << Id() << Name() << "agl" << AltitudeAGL() << "cg" << CGHeight()
				//<< "fallDist" << FallDistance();
		return true;										// Bottom of feet or body is close to the ground.
	}

	return false;
}

bool DaCreature::IsExoskeletal() const	// based on DNA
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->DNA().IsExoskeletal();
	else
		is = m_Animalia->IsExoskeletal();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsLeaper() const	// based on DNA
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->DNA().CanJump();
	else
		is = m_Animalia->IsLeaper();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::LivesInWater() const	// based on DNA
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->DNA().LivesInWater;
	else
		is = m_Animalia->LivesInWater();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::LivesUnderGround() const	// based on DNA
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->DNA().LivesUnderGround;
	else
		is = m_Animalia->LivesUnderGround();
	m_BodyLock.unlock();
	return is;
}

AuDNABase::ECOSTRATEGY DaCreature::EcoStrategy() const
{
	AuDNABase::ECOSTRATEGY es;
	m_BodyLock.lockForRead();
	if(m_Body)
		es = m_Body->DNA().EcoStrategy();
	else
		es = m_Animalia->EcoStrategy();
	m_BodyLock.unlock();
	return es;
}

ECOROLE DaCreature::EcoRole() const
{
	ECOROLE er;
	m_BodyLock.lockForRead();
	if(m_Body)
		er = m_Body->DNA().EcoRole();
	else
		er = m_Animalia->EcoRole();
	m_BodyLock.unlock();
	return er;
}

bool DaCreature::IsGameTablePose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsGameTablePose();
	else
		is = m_Animalia->IsGameTablePose();
	m_BodyLock.unlock();
	return is;
}

AuDNABase* DaCreature::DNAnew() const
{
	AuDNABase* dna;
	m_BodyLock.lockForRead();
	if(m_Animalia)
		dna = m_Animalia->DNA().Clone();
	else if(m_Body)
		dna = m_Body->DNA().Clone();
	else
		dna = 0;
	m_BodyLock.unlock();
	return dna;
}

/*
AuDNA DaCreature::DNA4() const
{
	m_BodyLock.lockForRead();
	AuDNA dna;
	if(m_Animalia)
		dna = m_Animalia->DNA();
	else
		qWarning() << "error: DaCreature::DNA4 has" << (m_Body ? "DNA3" : "No DNA");
	m_BodyLock.unlock();
	return dna;
}
*/

bool DaCreature::IsCrouched() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsCrouchedPose();
	else
		is = m_Animalia->IsCrouchedPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsBodyDead() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsDead();
	else
		is = m_Animalia->IsDead();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsBodyDying() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsDying();
	else
		is = m_Animalia->IsDying();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsHerbivore() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->DNA().EcoRole() == ECO_Herbivore;
	else
		is = m_Animalia->EcoRole() == ECO_Herbivore;
	m_BodyLock.unlock();
	return is;
}

GENDER DaCreature::Gender() const
{
	GENDER gender;
	m_BodyLock.lockForRead();
	if(m_Body)
		gender = (GENDER)m_Body->Sex();
	else
		gender = m_Animalia->Gender();
	m_BodyLock.unlock();
	return gender;
}

float DaCreature::Density() const
{
	// Environment suit acts like a diving suit in that it controls my buoyancy so I can maintain my altitude. This is
	// especially needed in the sea of gas giants.
	if(DbCommodity::IsEnvironmentSuit(Armor()))
		return DensityOfWater();
	float d;
	m_BodyLock.lockForRead();
	if(m_Body)
		d = m_Body->Density();
	else
		d = m_Animalia->Density();
	m_BodyLock.unlock();
	return d;
}

AuPose DaCreature::CurrentGeneralPose() const
{
	AuPose pose;
	m_BodyLock.lockForRead();
	if(m_Body)
		pose = m_Body->CurrentGeneralPose();
	else
		pose = m_Animalia->CurrentGeneralPose();
	m_BodyLock.unlock();
	return pose;
}

float DaCreature::CurrentHeadTilt() const
{
	float headTilt;
	m_BodyLock.lockForRead();
	if(m_Body)
		headTilt = m_Body->CurrentHeadTilt();
	else
		headTilt = m_Animalia->CurrentHeadTilt();
	m_BodyLock.unlock();
	return headTilt;
}

// Returns current hit points of body, not scaled for growth size.
short int DaCreature::CurrentBodyHitPoints() const
{
	short int hp;
	m_BodyLock.lockForRead();
	if(m_Body)
		hp = m_Body->CurrentHitPoints();
	else
		hp = m_Animalia->CurrentHitPoints();
	m_BodyLock.unlock();
	return hp;
}

void DaCreature::CurrentPoses4(AuPose pose[AuAnimalPart::PS_Count]) const
{
	Q_ASSERT(m_Animalia);
	m_BodyLock.lockForRead();
	for(int p=0; p<AuAnimalPart::PS_Count; p++)
		pose[p] = m_Animalia->CurrentPoses()[p];
	m_BodyLock.unlock();
}

void DaCreature::EatOffGround()
{
	bool didIt;
	m_BodyLock.lockForWrite();
	if(m_Body)
		didIt = m_Body->PoseEatOffGround();
	else
		didIt = m_Animalia->PoseEatOffGround(Id());
	if(didIt)
	{
		m_StationId = 0;
		m_tmPoseChange.Set();
	}
	m_BodyLock.unlock();

	if(didIt)
	{
		// Animal body render origin changes when eating off the ground.
		PositionRiders();
		SetInMotion();
		TickTock();
	}
}

bool DaCreature::IsConsolePose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsConsolePose();
	else
		is = m_Animalia->IsConsolePose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsPostPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsPostPose();
	else
		is = m_Animalia->IsPostPose();
	m_BodyLock.unlock();
	return is;
}

// True if a troop in the current pose is manning a station.
bool DaCreature::IsTroopStationPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsTroopStationPose();
	else
		is = m_Animalia->IsTroopStationPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsLockedPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsLockedPose();
	else
		is = m_Animalia->IsLockedPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsFlying() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsFlyingPose();
	else
		is = m_Animalia->IsFlyingPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsLadderPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsLadderPose();
	else
		is = m_Animalia->IsLadderPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsLeftHanded() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsLeftHanded();
	else
		is = m_Animalia->IsLeftHanded();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsJumping() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->CurrentGeneralPose().IsJumping();
	else
		is = m_Animalia->IsJumpingPose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsProne() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsPronePose();
	else
		is = m_Animalia->IsPronePose();
	m_BodyLock.unlock();
	return is;
}

bool DaCreature::IsSwimming() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsSwimmingPose();
	else
		is = m_Animalia->IsSwimmingPose();
	m_BodyLock.unlock();
	return is;
}

// True if my current body pose is an integrated turret station, for new style AuScDesigns.
bool DaCreature::IsTurretIntegratedPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsTurretIntegratedPose();
	else
		is = m_Animalia->IsTurretIntegratedPose();
	m_BodyLock.unlock();
	return is;
}

// True if my current body pose is operating a turret.
bool DaCreature::IsTurretPose() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->IsTurretPose();
	else
		is = m_Animalia->IsTurretPose();
	m_BodyLock.unlock();
	return is;
}

short int DaCreature::MaxHitPoints() const
{
	short int hp;
	m_BodyLock.lockForRead();
	if(m_Body)
		hp = m_Body->MaxHitPoints();
	else
		hp = m_Animalia->MaxHitPoints();
	m_BodyLock.unlock();

	if(!IsFullGrown())
	{
		// Scale current hit points according to growth factor.
		int growTime = GrowthTime();
		hp = hp * (growTime - (m_MinutesToFullGrown - 1)) / growTime;
		if(!hp)
			return 1;	// Return at least 1 hit point.
	}

	return hp;
}

unsigned short int DaCreature::MaxStamina() const
{
	unsigned short int max;
	m_BodyLock.lockForRead();
	if(m_Body)
		max = m_Body->Stamina();
	else
		max = m_Animalia->Stamina();
	m_BodyLock.unlock();
	return max;
}

bool DaCreature::IsAwaitingSpawn() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->NeedsSpawn();
	else
		is = m_Animalia->NeedsSpawn();
	m_BodyLock.unlock();
	return is;
}

AuPoint DaCreature::JumpVectorFromCurrentPose() const
{
	AuPoint vec;
	m_BodyLock.lockForRead();
	if(m_Body)
		vec = m_Body->JumpVector();
	else
		vec = m_Animalia->JumpVector();
	m_BodyLock.unlock();
	return vec;
}

float DaCreature::MovementSpeed(float power) const
{
	// Movement speed in feet per second.
	float moveSpeed;
	m_BodyLock.lockForRead();
	if(m_Body)
		moveSpeed = m_Body->MovementSpeed();
	else
		moveSpeed = m_Animalia->MovementSpeed();
	m_BodyLock.unlock();

	// Adjust speed for conditions, e.g. paralysis.
	moveSpeed *= ConditionSpeedEffect();

	// Adjust speed with power applied.
	moveSpeed *= power;

	// Reduce movement speed based on burden carried.
	if(IsInGravity() && !IsInGravityWell() && moveSpeed > 0)
		moveSpeed *= MovementSpeedMod();

	return moveSpeed;
}

bool DaCreature::NeedsInit() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->NeedsInit();
	else
		is = m_Animalia->NeedsInit();
	m_BodyLock.unlock();
	return is;
}

float DaCreature::NoiseLevel() const	// 0..1
{
	float noise;
	m_BodyLock.lockForRead();
	if(m_Body)
		noise = m_Body->NoiseLevel();
	else
		noise = m_Animalia->NoiseLevel();
	m_BodyLock.unlock();
	return noise;
}

void DaCreature::ResetHitPoints()
{
	m_BodyLock.lockForWrite();
	if(m_Body)
		m_Body->ResetHitPoints();
	else
		m_Animalia->ResetHitPoints();
	m_tmHPChange.Set();
	m_BodyLock.unlock();

	ClearUnitDamage();
}

void DaCreature::SetHitPoints(short int hitPoints)
{
	// Must scale the hit points according to my scale.
	if(!IsFullGrown() && hitPoints)
	{
		// Scale up to natural value.
		int growTime = GrowthTime();
		hitPoints = (short int)ceilf((float)hitPoints * (float)growTime / (float)(growTime - (m_MinutesToFullGrown - 1)));
		if(!hitPoints)
			hitPoints = 1;	// Don't turn a non-zero number into zero.
	}

	m_BodyLock.lockForWrite();
	if(m_Body)
	{
		if(hitPoints != m_Body->CurrentHitPoints())
		{
			m_Body->SetHitPoints(hitPoints);
			m_tmHPChange.Set();
		}
	}
	else
	{
		if(hitPoints != m_Animalia->CurrentHitPoints())
		{
			m_Animalia->SetHitPoints(hitPoints);
			m_tmHPChange.Set();
		}
	}
	m_BodyLock.unlock();
}

void DaCreature::SetLeftHanded(bool isLefty)
{
	m_BodyLock.lockForWrite();
	if(m_Body)
		m_Body->SetLeftHanded(isLefty);
	else
		m_Animalia->SetLeftHanded(isLefty);
	m_tmBodyChange.Set();
	m_BodyLock.unlock();
}

void DaCreature::SetChromatid(AuDNABase::CHROMATID chroma)
{
	m_BodyLock.lockForWrite();
	if(m_Body)
		m_Body->SetSex(chroma == AuDNABase::Alpha ? Male : Female);
	else
		m_Animalia->SetDNA(m_Animalia->DNA(), chroma);
	m_tmBodyChange.Set();
	m_BodyLock.unlock();
}

// Returns the stationary pose that corresponds to my current body pose.
AuPose DaCreature::StationaryPose() const
{
	AuPose pose;
	m_BodyLock.lockForRead();
	if(m_Body)
		pose = m_Body->CurrentGeneralPose().Stationary();
	else
		pose = m_Animalia->CurrentGeneralPose().Stationary();
	m_BodyLock.unlock();
	return pose;
}

quint8 DaCreature::StaminaToJump() const
{
	quint8 s;
	m_BodyLock.lockForRead();
	if(m_Body)
		s = m_Body->StaminaToJump();
	else
		s = m_Animalia->StaminaToJump();
	m_BodyLock.unlock();
	return s;
}

bool DaCreature::SpecialWeaponReady() const
{
	bool is;
	m_BodyLock.lockForRead();
	if(m_Body)
		is = m_Body->SpecialWeaponReady();
	else
		is = m_Animalia->SpecialWeaponReady();
	m_BodyLock.unlock();
	return is;
}

void DaCreature::UseSpecialWeapon()
{
	m_BodyLock.lockForWrite();
	if(m_Body)
		m_Body->UseSpecialWeapon(AuTime::Now());
	else
		m_Animalia->UseSpecialWeapon(AuTime::Now());
	m_BodyLock.unlock();
}

float DaCreature::TorsoThickness() const
{
	float t;
	m_BodyLock.lockForRead();
	if(m_Body)
		t = m_Body->TorsoThickness();
	else
		t = m_Animalia->TorsoThickness();
	m_BodyLock.unlock();
	if(!IsFullGrown())
		t *= GrowthScale();	// Scale torso thickness according to growth factor.
	return t;
}

float DaCreature::TurnRate() const
{
	float t;
	m_BodyLock.lockForRead();
	if(m_Body)
		t = m_Body->TurnRate();
	else
		t = m_Animalia->TurnRate();
	m_BodyLock.unlock();
	return t;
}

float DaCreature::Volume() const
{
	float v;
	m_BodyLock.lockForRead();
	if(m_Body)
		v = m_Body->Volume();
	else
		v = m_Animalia->Volume();
	m_BodyLock.unlock();

	if(!IsFullGrown())
		v *= GrowthScale();	// Scale volume according to growth factor.

	return v;
}

void DaCreature::SetHairStyle(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave)
{
	m_BodyLock.lockForWrite();
	m_Haircut = cut;
	m_Shave = shave;
	m_tmHairChange.Set();
	m_BodyLock.unlock();
}

AuPose DaCreature::CurrentMouthPose() const
{
	AuPose pose;
	m_BodyLock.lockForRead();
	if(m_Body)
		pose = AuPose::MouthClosed;	// DpBody models do not have animated mouths.
	else
		pose = m_Animalia->CurrentMouthPose();
	m_BodyLock.unlock();
	return pose;
}

HANDEDNESS DaCreature::Handedness() const
{
	HANDEDNESS hand;
	m_BodyLock.lockForRead();
	if(m_Body)
		hand = m_Body->IsLeftHanded() ? LeftHanded : RightHanded;
	else
		hand = m_Animalia->Handedness();
	m_BodyLock.unlock();
	return hand;
}

DaUnit* DaCreature::Rider() const
{
	DaUnit* riderUnit = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		riderUnit = dynamic_cast<DaUnit*>(*ai);
		if(riderUnit)
			break;		// Found a subordinate unit.
	}
	return riderUnit;
}

void DaCreature::Orgasm()
{
	// I am mating and my stamina just reached zero.
	if(Gender() == Male || Gender() == Hermaphrodite)
	{
		// Male orgasm.
		DaCreature* female;
		if(CurrentGeneralPose() == AuPose::MateOnTop)
			female = dynamic_cast<DaCreature*>(Parent());	// My parent atom is on the bottom.
		else
			female = dynamic_cast<DaCreature*>(Rider());	// A subatom is on top.
		if(!female)
			return;

		bool impregnated = !female->IsPregnant() && IsSexuallyCompatible(*female);
		//qDebug() << "DaCreature::Orgasm" << *this << "female" << *female << "impregnated" << impregnated;

		// Male communication.
		AuId avatarDad;
		DaPerson* dad = dynamic_cast<DaPerson*>(this);
		if(dad)
		{
			if(impregnated)
			{
				avatarDad = dad->Id();

				// Player Statistics
				dad->Stats().ChildFathered();
				dad->SetModified();
				AuCommThoughtMessage msgDad(AuCommThoughtMessage::IImpregnatedThem, female->Name(), Id(), Name());
				g_Au.SendAtomChat(msgDad);
			}
			else
			{
				AuCommThoughtMessage msgDad(AuCommThoughtMessage::Boink, Id(), Name());
				g_Au.SendAtomChat(msgDad);
			}
		}
		else
		{
			// Male citizen exclamation.
			AuCommChat myreply(tr("O!"), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(myreply);
		}

		if(!impregnated)
			return;

		// Female communication.
		DaPerson* mom = dynamic_cast<DaPerson*>(female);
		if(mom)
		{
			// Player Statistics
			mom->Stats().ChildCarried();
			AuCommThoughtMessage msgMom(AuCommThoughtMessage::TheyImpregnatedMe, Name(), female->Id(), female->Name());
			g_Au.SendAtomChat(msgMom);
		}

		// Impregnate the female.
		AuDNABase* herDNA = female->DNAnew();
		AuDNABase* hisDNA = DNAnew();
		if(herDNA && hisDNA)
		{
			AuDNABase* babyDNA = hisDNA->Combine(*herDNA, g_Au.Dice());	// Combine our DNA.
			if(babyDNA)
			{
				quint8 babyGender = g_Au.Dice().Roll(1,2,-1);
				LINEAGE_TYPE lineage;
				if(dad && mom)
				{
					// Both parents are avatars.
					if(dad->IsLineageDemiavatar() || mom->IsLineageDemiavatar())
						lineage = LINEAGE_Demiavatar;	// One parent is of mixed lineage.
					else
						lineage = LINEAGE_Avatar;		// Both parents have pure avatar lineage.
				}
				else if(dad || mom)
					lineage = LINEAGE_Demiavatar;		// One parent is an avatar; the other is not.
				else
					lineage = LINEAGE_Citizen;			// Both parents are citizens.

				AuConPregnant* pregnancy = new AuConPregnant(avatarDad, *babyDNA, babyGender, lineage, AuTime::Now());
				female->AddCondition(pregnancy);
				delete babyDNA;
			}
		}
		if(herDNA)
			delete herDNA;
		if(hisDNA)
			delete hisDNA;
	}
	else
	{
		// Female orgasm.
		//qDebug() << "DaCreature::Orgasm" << *this;
		if(IsAIControlled())
		{
			AuCommChat myreply(tr("O!"), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(myreply);
		}
		else
		{
			AuCommThoughtMessage msg(AuCommThoughtMessage::O, Id(), Name());
			g_Au.SendAtomChat(msg);
		}
	}
}

AuDNABase::CHROMATID DaCreature::Chromatid() const
{
	AuDNABase::CHROMATID chroma;
	m_BodyLock.lockForRead();
	if(m_Animalia)
		chroma = m_Animalia->Chromatid();
	else if(m_Body)
		chroma = m_Body->Chromatid();
	else
		chroma = AuDNABase::Alpha;
	m_BodyLock.unlock();
	return chroma;
}

quint32 DaCreature::ExperiencePointsForKilling() const
{
	quint32 maxHp = MaxHitPoints();
	if(maxHp > 9)
		return maxHp / 5;
	return 1;
}

void DaCreature::Dismounted()
{
	if(IsClientControlled())
		SetControlled(AuId());
	if(IsAIControlled())
	{
		if(!CurrentGeneralPose().IsMatingOrWaiting())
			TiltHeadTo(0);
		SetInMotion();
	}
	TickTock();
}

int DaCreature::DNAVersion() const
{
	if(m_Animalia)
		return 4;
	else if(m_Body)
		return 3;
	return 0;	// wut?
}

void DaCreature::SetMinutesToFullGrown(quint8 minutes)
{
	m_MinutesToFullGrown = minutes;
	if(m_Animalia)
		m_Animalia->SetGrowthStage(GrowthStage());
}

void DaCreature::Defibrillated(quint32 byEmpireId)
{
	Q_UNUSED(byEmpireId);
	ClearPendingDamage();
}

