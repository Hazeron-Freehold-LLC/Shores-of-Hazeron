//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttick.cpp,v 1.91 2022/04/22 19:08:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dttick.h"
//
#include "auatommanager.h"
#include "auglobal.h"
#include "autime.h"
#include "daatom.h"

DtTick::DtTick(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_semAlive.acquire(1);	// I live...
	m_UsedMs = 0;
}

DtTick::~DtTick()
{
	//qDebug("DtTick::~DtTick.");
}

void DtTick::run()
{
	AuElapsedTimer atomTime;	// Measures tick time of individual atoms.
	//AuTime lastTime;	// For alive messages.
	AuElapsedTimer loopTime;	// For load calculation.
	loopTime.start();
	AuId tickAtomId;	// Id returned by Lock function.
	AuId tickAtomSystemId;
	AuId prevAtomId;
	unsigned int consecutiveAtoms = 0;
	while(Alive())
	{
		// Tick the next assigned atom.
		// Gets the current time from the system clock, down to the millisecond. This must be done after the root mutex is
		// locked to prevent timing inconsistencies with other threads modifying the atoms.
		DaAtom* tickAtom = g_Au.AtomManagerDispatch()->LockNextTickAtom(prevAtomId, tickAtomId, tickAtomSystemId);
		if(!tickAtomId.IsNull())
			prevAtomId = tickAtomId;							// Capture atom id if it is not 0.
		if(tickAtom)
		{
			if(tickAtom->OkToTick(tickAtomSystemId))
			{
				// Restart atom tick timer after acquiring the lock.
				atomTime.restart();

				// Tick my assigned atom.
				// The TickTime is the last time the atom was Ticked. Only ever null once, when the atom is first created.
				if(g_Au.m_LogMessages & LOG_Physics)
				{
					int ms = tickAtom->TickTime().elapsed();
					if(ms > 500)
						qDebug() << "DtTick::run" << *tickAtom << "last ticked" << ms << "ms ago, too long for movement.";
				}
				tickAtom->Tick(tickAtom->TickTime(), atomTime);

				// Use the unlock that updates the tick time statistics for the atom.
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

				// We will actually report 2x the milliseconds, so we can report 1 when the time was zero.
				// This way the zero-time ones are considered 1/2ms each.
				if(msElapsed)
					msElapsed *= 2;
				else
					msElapsed = 1;
				g_Au.AtomManagerDispatch()->UnlockTickAtom(tickAtomId, msElapsed);
			}
			else
			{
				// Use the unlock that does not update the tick time statistics for the atom.
				g_Au.AtomManagerDispatch()->UnlockTickAtom(tickAtomId, -1);
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
				usleep(100);
			}
		}
		else
		{
			// We didn't find the atom or not enough time has passed to be ready to tick it again.
			// Use the unlock that does not update the tick time statistics for the atom.
			g_Au.AtomManagerDispatch()->UnlockTickAtom(tickAtomId, -1);
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
			QString logmsg = QString("Tick Thread 0x%1 Alive").arg((unsigned long)currentThreadId(), 12, 16, QChar('0'));
			g_Au.Log(logmsg);
		}
		lastTime.Set();
		*/
	}
}

quint64 DtTick::UsedMsReset()
{
	m_UseLock.lock();
	quint64 u = m_UsedMs;
	m_UsedMs = 0;
	m_UseLock.unlock();
	return u;
}
