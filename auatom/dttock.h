//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttock.h,v 1.13 2022/03/22 21:14:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtTock_H
#define DtTock_H

#include "autime.h"
#include <QColor>
#include <QMutex>
#include <QSemaphore>
#include <QThread>

class AuId;
class DaAtom;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Tock transactions are used to record when each line of code or other milestone is reached during the Tock call. This is used to
// help root out sources of lag, namely a few atoms that are taking over 30 seconds to complete the Tock call.

class DtTockTransaction
{
	quint32								m_ElapsedMs;	// Millisecs since tockTime when constructed.
	QString								m_What;			// What got completed.

public:
	// 'what' is a brief description of some action that just finished during Tock.
	// 'tockTime' is the time Tock was called, literally the 'now' time argument of the Tock call.
	inline DtTockTransaction(const QString& what, const AuTime& tockTime)
	{
		m_ElapsedMs = tockTime.ElapsedMSec();
		m_What = what;
	}

	// Typical usage when called from Tock:
	//  tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	inline DtTockTransaction(const char* file, int line, const AuTime& tockTime)
	{
		m_ElapsedMs = tockTime.ElapsedMSec();
		m_What = QString("%1:line %2").arg(file).arg(line);
	}

	inline quint32 ElapsedMs() const
	{
		return m_ElapsedMs;
	}

	void Report(const AuId& atomId) const;

	inline const QString& What() const
	{
		return m_What;
	}
};

class DtTockTransactions
{
	QList<DtTockTransaction>			m_Actions;

public:
	inline DtTockTransactions()
	{
	}

public:
	void Add(const DtTockTransaction& trans);

	void Report(const AuId& atomId, int msElapsed) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DtTock : public QThread
{
	Q_OBJECT;
	QSemaphore							m_semAlive;				// Used to know when I'm supposed to die.

	QMutex								m_UseLock;
	quint64								m_UsedMs;				// Used milliseconds.

public:
	DtTock(QObject* parent=0);
	~DtTock();

	inline bool Alive() const
	{
		return !m_semAlive.available();
	}

	// 'ms' is in 1/2 ms increments.
	inline static QColor Color(unsigned int ms, quint8 alpha=255)
	{
		if(ms <= NominalTime()*2 + 3)
			return QColor(50,255,50,alpha);						// Green
		else if(ms < OverworkedTime()*2)
			return QColor(255,153,50,alpha);					// Amber
		else
			return QColor(255,50,50,alpha);						// Red
	}

	inline void Kill()
	{
		m_semAlive.release(1);
	}

	static inline unsigned int NominalTime()
	{
		return 100;						// If faster than 100ms service is needed, use DtTick instead. AI pilots like about 100ms.
	}

	static inline unsigned int OverworkedTime()
	{
		return 1000;					// AI pilots will have trouble if this gets too jittery.
	}

	// Retrieves the current used ms value and resets it to 0.
	quint64 UsedMsReset();

protected:
	virtual void run();
	void TockAtom(const AuId& atomId);
};

#endif // DtTock_H
