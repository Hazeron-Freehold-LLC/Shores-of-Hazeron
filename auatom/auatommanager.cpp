//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatommanager.cpp,v 1.43 2022/04/15 16:56:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatommanager.h"
//
#include "auether.h"
#include "auglobal.h"
#include "daatom.h"
#include "dttick.h"
#include "dttock.h"

AuAtomManager::AuAtomManager(QObject* threadParent)
{
	//qDebug("AuAtomManager::AuAtomManager - Starting Tick/Tock threads to manage atom %d in sector %d",
			//m_AtomId.GetID(), m_SectorId.GetID());
	m_Tick = new DtTick(threadParent);
	m_Tick->start(QThread::NormalPriority);
	m_Tock = new DtTock(threadParent);
	m_Tock->start(QThread::NormalPriority);
	//qDebug("AuAtomManager::AuAtomManager - Tick/Tock threads started managing atom %d in sector %d",
			//m_AtomId.GetID(), m_SectorId.GetID());
}

AuAtomManager::~AuAtomManager()
{
	if(m_Tick)
	{
		m_Tick->deleteLater();
		m_Tick->Kill();
		AuElapsedTimer t;
		t.start();
		while(m_Tick->isRunning())
		{
			if(t.elapsed() > 1000)
			{
#if !defined(QT_NO_DEBUG)
				qDebug() << "D Waiting for Tick thread to die." << this;
#endif
				t.restart();
			}
		}
		m_Tick = 0;
	}

	// Stop Tock first so it can cleanly stop modifying atoms.
	if(m_Tock)
	{
		m_Tock->deleteLater();
		m_Tock->Kill();
		AuElapsedTimer t;
		t.start();
		while(m_Tock->isRunning())
		{
			if(t.elapsed() > 1000)
			{
#if !defined(QT_NO_DEBUG)
				qDebug() << "D Waiting for Tock thread to die." << this;
#endif
				t.restart();
			}
		}
		m_Tock = 0;
	}
}

quint64 AuAtomManager::UsedMsReset()
{
	quint64 usedMs = m_Tick->UsedMsReset();
	usedMs += m_Tock->UsedMsReset();
	return usedMs;
}

//static
QColor AuAtomManager::Color(float atomLoad, int etheriumLock, quint32 avgTickCycleMs, unsigned int systemsDeadlocked, bool bdLock,
		bool dataLock, bool itLock, bool scLock, bool trashLock)
{
	quint8 alpha = 255;
	if(bdLock || dataLock || itLock || scLock || trashLock)
		return QColor(50,255,255,alpha);					// Cyan

	if(etheriumLock < 0)
		return QColor(255,50,255,alpha);					// Purple
	else if(etheriumLock > 1000)
		return QColor(128,128,255,alpha);					// Blue

	if(systemsDeadlocked)
		return QColor(255,192,255,alpha);					// Pink

	if(atomLoad < .5 && avgTickCycleMs <= DtTick::NominalTime()*2 + 3)
		return QColor(50,255,50,alpha);						// Green
	else if(atomLoad < .9 && avgTickCycleMs < DtTick::OverworkedTime()*2)
		return QColor(255,153,50,alpha);					// Amber
	else
		return QColor(255,50,50,alpha);						// Red
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuAtomManagerDispatch::AuAtomManagerDispatch(int atomManagerCount, QObject* parent)
{
	Q_ASSERT(atomManagerCount);
	if(g_Au.m_LogMessages & LOG_Debug)
		g_Au.Log(QObject::tr("Atom Managers %L1").arg(atomManagerCount));
	for(int ti=0; ti<atomManagerCount; ti++)
		m_AtomManagers.append(new AuAtomManager(parent));
}

AuAtomManagerDispatch::~AuAtomManagerDispatch()
{
	for(QList<AuAtomManager*>::iterator mi = m_AtomManagers.begin(); mi != m_AtomManagers.end(); mi++)
		delete *mi;
	m_AtomManagers.clear();
}

DaAtom* AuAtomManagerDispatch::LockNextTickAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId)
{
	// Lock assignments until assigned atom unlocked.
	AuEther& ether = g_Au.LockEtheriumDelete();
	if(g_Au.LockNextTicker(prevAtomId, atomId, systemId))
	{
		// Atom lock acquired and it is ready for processing.
		//if(IsSolarSystemAssignedToMe(systemId))
		{
			// The solar system is still assigned to me.
			// Must lock root for read before locking any assigned atom because they are subordinate to root.
			DaAtom* atom = ether.FindAtom(atomId);
			if(atom)
			{
				if(g_Au.m_LogMessages & LOG_TickTock)
					qDebug() << "L AuAtomManagerDispatch::LockNextTickAtom" << *atom << atom->SpotDescription(0);
				return atom;
			}
		}
		// The atom isn't supposed to be here.
		g_Au.ReleaseTicker(atomId);		// This prevents it from being locked and allows UnlockTicker to remove it.
		g_Au.UnlockTicker(atomId);
	}
	atomId = AuId(0);
	systemId = AuId(0);
	return 0;
}

void AuAtomManagerDispatch::UnlockTickAtom(const AuId& atomId, int msElapsed)
{
	// Unlock the specific atom assignment mutex.
	if(!atomId.IsNull())
	{
		if(msElapsed >= 0)
			g_Au.UnlockTicker(atomId, msElapsed);
		else
			g_Au.UnlockTicker(atomId);	// No update of process time.
	}
	g_Au.UnlockEtheriumDelete();
}

DaAtom* AuAtomManagerDispatch::LockNextTockAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId, AuEther** ether)
{
	// Lock assignments until assigned atom unlocked.
	*ether = &g_Au.LockEtheriumDelete();
	if(g_Au.LockNextTocker(prevAtomId, atomId, systemId))
	{
		// Atom lock acquired and it is ready for processing.
		//if(IsSolarSystemAssignedToMe(systemId))
		{
			// The solar system is still assigned to me.
			// Must lock root for read before locking any assigned atom because they are subordinate to root.
			DaAtom* atom = (*ether)->FindAtom(atomId);
			if(atom)
			{
				if(g_Au.m_LogMessages & LOG_TickTock)
					qDebug() << "L AuAtomManagerDispatch::LockNextTockAtom" << *atom << atom->SpotDescription(0);
				return atom;
			}
		}
		// The atom isn't supposed to be here.
		g_Au.ReleaseTocker(atomId);		// This prevents it from being locked and allows UnlockTocker to remove it.
		g_Au.UnlockTocker(atomId);
	}
	atomId = AuId(0);
	systemId = AuId(0);
	return 0;
}

void AuAtomManagerDispatch::UnlockTockAtom(const AuId& atomId, int msElapsed)
{
	// Unlock the specific atom assignment mutex.
	if(!atomId.IsNull())
	{
		if(msElapsed >= 0)
			g_Au.UnlockTocker(atomId, msElapsed);
		else
			g_Au.UnlockTocker(atomId);	// No update of process time.
	}
	g_Au.UnlockEtheriumDelete();
}

quint64 AuAtomManagerDispatch::UsedMsReset()
{
	quint64 usedMs = 0;
	for(QList<AuAtomManager*>::iterator mi = m_AtomManagers.begin(); mi != m_AtomManagers.end(); mi++)
		usedMs += (*mi)->UsedMsReset();
	return usedMs;
}

