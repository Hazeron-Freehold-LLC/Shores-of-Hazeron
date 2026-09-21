//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttick.h,v 1.19 2022/02/19 17:38:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtTick_H
#define DtTick_H

#include <QColor>
#include <QMutex>
#include <QSemaphore>
#include <QThread>

class AuId;
class DaAtom;

class DtTick : public QThread
{
	Q_OBJECT;
	QSemaphore							m_semAlive;				// Used to know when I'm supposed to die.

	QMutex								m_UseLock;
	quint64								m_UsedMs;				// Used milliseconds.

public:
	DtTick(QObject* parent=0);
	~DtTick();

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
		return 1000/30;	// 30 ticks per second is smooth enough for game play
	}

	static inline unsigned int OverworkedTime()
	{
		return 1000/10;	// 10 ticks per second feels choppy
	}

	// Retrieves the current used ms value and resets it to 0.
	quint64 UsedMsReset();

protected:
	virtual void run();
	void TickAtom(const AuId& atomId);
};

#endif // DtTick_H
