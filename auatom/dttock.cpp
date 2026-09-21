//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttock.cpp,v 1.76 2022/04/15 16:57:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dttock.h"
//
#include "auatommanager.h"
#include "auglobal.h"
#include "autime.h"
#include "daatom.h"

void DtTockTransaction::Report(const AuId& atomId) const
{
	qDebug() << atomId << "    " << m_What << "at" << m_ElapsedMs << "ms";
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void DtTockTransactions::Add(const DtTockTransaction& trans)
{
	// When disabled we do not add transactions.
	if(!(g_Au.m_DebugSwitches & 0x01000000))
		return;
	m_Actions.append(trans);
}

void DtTockTransactions::Report(const AuId& atomId, int msElapsed) const
{
	// When disabled we do not report transactions.
	if(!(g_Au.m_DebugSwitches & 0x01000000))
		return;
	qDebug() << atomId << "SLOW Tock" << msElapsed << "ms";
	for(QList<DtTockTransaction>::const_iterator ai = m_Actions.cbegin(); ai != m_Actions.cend(); ai++)
		(*ai).Report(atomId);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DtTock::DtTock(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_semAlive.acquire(1);	// I live...
	m_UsedMs = 0;
}

DtTock::~DtTock()
{
	//qDebug("DtTock::~DtTock.");
}

void DtTock::run()
{
	AuElapsedTimer atomTime;		// Measures tock time of individual atoms, for tock load balancing.
	//AuTime lastTime;	// For alive messages.
	AuElapsedTimer loopTime;	// For load calculation.
	loopTime.start();
	AuId tockAtomId;	// Id returned by Lock function.
	AuId tockAtomSystemId;
	AuId prevAtomId;
	unsigned int consecutiveAtoms = 0;
	while(Alive())
	{
		// Tock the next assigned atom.
		// Gets the current time from the system clock, down to the millisecond. This must be done after the root mutex is
		// locked to prevent timing inconsistencies with other threads modifying the atoms.
		AuEther* ether;
		DaAtom* tockAtom = g_Au.AtomManagerDispatch()->LockNextTockAtom(prevAtomId, tockAtomId, tockAtomSystemId, &ether);
		if(!tockAtomId.IsNull())
			prevAtomId = tockAtomId;							// Capture atom id if it is not 0.
		if(tockAtom)
		{
			if(tockAtom->OkToTock(tockAtomSystemId))
			{
				// Restart atom tock timer after acquiring the lock.
				atomTime.restart();

				// Tock my assigned atom. First time through, elapsed time is expected to be zero.
				AuTime now = AuTime::Now();
				DtTockTransactions tockTrans;
				const AuScripts* scripts = g_Au.LockScripts();
				tockAtom->Tock(tockAtom->TockTime(), now, *scripts, tockTrans);
				g_Au.UnlockScripts();
				tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
				if(tockAtom->IsDead())
				{
					ether->TrashAtom(tockAtomId);	// The atom died.
					tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
				}
				else
				{
					tockAtom->TickTock();
					tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
				}

				// Use the unlock that updates the tock time statistics for the atom.
				int msElapsed = atomTime.elapsed();
				if(msElapsed < 0)
				{
					// Prevent negative elapsed time. Never have seen this happen.
					//qDebug("negative msElapsed %dms", msElapsed);
					msElapsed = 0;
				}
				else if(msElapsed > 1000000 && msElapsed <= 86400000)
				{
					// Prevent huge elapsed time. Have seen the number 86399999 show up a lot. I suspect it's due to the time daemon
					// updating the system clock. 86400000 happens to be the number of milliseconds in a day.
					//qDebug("huge msElapsed %dms, using %dms instead", msElapsed, 86400000 - msElapsed);
					msElapsed = 86400000 - msElapsed;
				}
				else if(msElapsed > 10000)
				{
					// Report what happened during the Tock call.
					tockTrans.Report(tockAtomId, msElapsed);
				}

				// We will actually report 2x the milliseconds, so we can report 1 when the time was zero.
				// This way the zero-time ones are considered 1/2ms each.
				if(msElapsed)
					msElapsed *= 2;
				else
					msElapsed = 1;
				g_Au.AtomManagerDispatch()->UnlockTockAtom(tockAtomId, msElapsed);
			}
			else
			{
				if(tockAtom->IsDead())
					ether->TrashAtom(tockAtomId);	// The atom died.
				else
					tockAtom->TickTock();

				// Use the unlock that does not update the tock time statistics for the atom.
				g_Au.AtomManagerDispatch()->UnlockTockAtom(tockAtomId, -1);
			}
			m_UseLock.lock();
			m_UsedMs += loopTime.elapsed();
			m_UseLock.unlock();
			consecutiveAtoms++;
			if(consecutiveAtoms > 100)
			{
				// I have processed 100 atoms without stopping.
				// Sleep once in a while so I don't hog the CPU.
				consecutiveAtoms = 0;
				msleep(1);
			}
		}
		else
		{
			// We didn't find the atom or not enough time has passed to be ready to tock it again.
			// Use the unlock that does not update the tock time statistics for the atom.
			g_Au.AtomManagerDispatch()->UnlockTockAtom(tockAtomId, -1);
			m_UseLock.lock();
			m_UsedMs += loopTime.elapsed();
			m_UseLock.unlock();
			consecutiveAtoms = 0;
			msleep(NominalTime()/10);
		}
		loopTime.restart();

		/*
		// Print a message once per minute so we know this thread is still alive.
		if((g_Au.m_LogMessages & LOG_TickTock) && lastTime.time().minute() != now.time().minute())
		{
			QString logmsg = QString("Tock Thread 0x%1 Alive").arg((unsigned long)currentThreadId(), 12, 16, QChar('0'));
			g_Au.Log(logmsg);
		}
		lastTime.Set();
		*/
	}
}

quint64 DtTock::UsedMsReset()
{
	m_UseLock.lock();
	quint64 u = m_UsedMs;
	m_UsedMs = 0;
	m_UseLock.unlock();
	return u;
}
