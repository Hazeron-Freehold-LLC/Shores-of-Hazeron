//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auelapsedtimer.h,v 1.6 2022/05/17 16:56:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuElapsedTimer_H
#define AuElapsedTimer_H

#include "auutil.h"
#include <QElapsedTimer>

class QDataStream;

#define d_MSecPerSec					1000					// Milliseconds per Second
#define d_MSecPerMin					(60 * d_MSecPerSec)		// Milliseconds per Minute
#define d_MSecPerHr						(60 * d_MSecPerMin)		// Milliseconds per Hour
#define d_MSecPerDay					(24 * d_MSecPerHr)		// Milliseconds per Day

// Epoch is loosely defined here. This code was adapted from AuTime.
class AUUTIL_DLL AuElapsedTimer : public QElapsedTimer
{
public:	// Constructors
	inline AuElapsedTimer()
	{
	}

public:	// Operators
	inline bool operator==(const AuElapsedTimer& t) const
	{
		return msecsSinceReference() == t.msecsSinceReference();
	}

	inline bool operator<(const AuElapsedTimer& t) const
	{
		return msecsSinceReference() < t.msecsSinceReference();
	}

	inline bool operator<=(const AuElapsedTimer& t) const
	{
		return msecsSinceReference() <= t.msecsSinceReference();
	}

	inline bool operator>(const AuElapsedTimer& t) const
	{
		return msecsSinceReference() > t.msecsSinceReference();
	}

	inline bool operator>=(const AuElapsedTimer& t) const
	{
		return msecsSinceReference() >= t.msecsSinceReference();
	}

public:	// Methods
	// Returns a number from 0 to 1 which is the fraction of the time into the current period.
	// 'period' is the length of a period in milliseconds
	inline float CurrentCycle(qint64 period) const
	{
		return (float)(fmod((double)CurrentMSecsSinceEpoch(), (double)period) / (double)period);
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the current day.
	inline float CurrentDayCycle() const
	{
		return CurrentCycle(d_MSecPerDay);
	}

	inline qint64 CurrentDaysSinceEpoch() const
	{
		return CurrentMSecsSinceEpoch() / d_MSecPerDay;
	}

	inline qint64 CurrentMinsSinceEpoch() const
	{
		return CurrentMSecsSinceEpoch() / d_MSecPerMin;
	}

	inline qint64 CurrentMSecsSinceEpoch() const
	{
		return msecsSinceReference() + elapsed();
	}

	inline qint64 CurrentPeriod(qint64 periodMSec) const
	{
		return CurrentMSecsSinceEpoch() / periodMSec;
	}

	inline qint64 CurrentPeriod(qint64 periodMSec, qint64 offsetMSec) const
	{
		return (CurrentMSecsSinceEpoch() + offsetMSec) / periodMSec;
	}

	inline static bool CurrentSecIsOdd()
	{
		AuElapsedTimer now;
		now.start();
		return now.SecsSinceEpoch() & 1;
	}

	inline qint64 CurrentSecsSinceEpoch() const
	{
		return CurrentMSecsSinceEpoch() / d_MSecPerSec;
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the period of this time value.
	// 'period' is the length of a period in milliseconds
	inline float Cycle(qint64 period) const
	{
		return (float)(fmod((double)msecsSinceReference(), (double)period) / (double)period);
	}

	inline float Cycle(qint64 period, qint64 offsetMSec) const
	{
		return (float)(fmod((double)(msecsSinceReference() + offsetMSec), (double)period) / (double)period);
	}

	inline qint64 DaysSinceEpoch() const
	{
		return msecsSinceReference() / d_MSecPerDay;
	}

	// Returns the number of days (truncated) from this time to the current time.
	// This differs from ElapsedDay in that the result is based on the difference between the whole days of 't' and Now.
	// ElapsedDay is based on the difference in milliseconds between the two.
	inline qint64 ElapsedDayTicks() const
	{
		return CurrentDaysSinceEpoch() - DaysSinceEpoch();
	}

	// Returns the number of hours (truncated) from this time to the current time.
	inline qint64 ElapsedHour() const
	{
		return HoursTo(Now());
	}

	// Returns the number of seconds (truncated) from this time to the current time.
	inline qint64 ElapsedMin() const
	{
		return MinsTo(Now());
	}

	// Returns the number of seconds (truncated) from this time to the current time.
	// This differs from ElapsedMin in that the result is based on the difference between the whole days of 't' and Now.
	// ElapsedMin is based on the difference in milliseconds between the two.
	inline qint64 ElapsedMinTicks() const
	{
		return CurrentMinsSinceEpoch() - MinsSinceEpoch();
	}

	// Returns the number of milliseconds from this time to the current time.
	inline qint64 ElapsedMSec() const
	{
		return elapsed();
	}

	// Returns the whole second to which this timer has elapsed.
	inline qint64 ElapsedSec() const
	{
		return SecsTo(Now());
		//return (msecsSinceReference() + elapsed())/d_MSecPerSec;
	}

	// Returns the number of seconds (truncated) from my start second to the my elapsed second.
	inline qint64 ElapsedSecTicks() const
	{
		return CurrentSecsSinceEpoch() - SecsSinceEpoch();
	}

	inline qint64 ElapsedSecTicks(const AuElapsedTimer& t) const
	{
		return t.SecsSinceEpoch() - SecsSinceEpoch();
	}

	inline qint64 HoursTo(const AuElapsedTimer& t) const
	{
		return MSecsTo(t) / d_MSecPerHr;
	}

	// Returns true if the period of this time is different from the period of the current time. 
	// 'periodMSec' is the length of each period in milliseconds.
	// 'offsetMSec' is an offset applied to this time and the current time. Typical usage passes the id of an atom here so
	// similar atoms do not make the same decision at the same time.
	// e.g. troops use this to decide when to upgrade their equipment. The offset insures all troops don't upgrade their equipment
	// at the same time.
	inline bool IsNewPeriod(qint64 periodMSec, qint64 offsetMSec) const
	{
		qint64 thisPeriod = Period(periodMSec, offsetMSec);
		qint64 currentPeriod = CurrentPeriod(periodMSec, offsetMSec);
		return thisPeriod != currentPeriod;
	}

	inline bool IsNewPeriod(const AuElapsedTimer& now, qint64 periodMSec, qint64 offsetMSec) const
	{
		qint64 thisPeriod = Period(periodMSec, offsetMSec);
		qint64 nowPeriod = now.Period(periodMSec, offsetMSec);
		return thisPeriod != nowPeriod;
	}

	// Returns true if the period of this time is different from the period of the current time. 
	// 'periodMSec' is the length of each period in milliseconds.
	inline bool IsNewPeriod(qint64 periodMSec) const
	{
		qint64 thisPeriod = Period(periodMSec);
		qint64 currentPeriod = CurrentPeriod(periodMSec);
		return thisPeriod != currentPeriod;
	}

	inline bool IsNewPeriod(const AuElapsedTimer& now, qint64 periodMSec) const
	{
		qint64 thisPeriod = Period(periodMSec);
		qint64 nowPeriod = now.Period(periodMSec);
		return thisPeriod != nowPeriod;
	}

	inline bool IsNull() const
	{
		return !isValid();
	}

	// Returns a number from 0 to 1 which is the fraction of the time into this minute.
	inline float MinCycle() const
	{
		return Cycle(d_MSecPerMin);
	}

	inline qint64 MinsSinceEpoch() const
	{
		return msecsSinceReference() / d_MSecPerMin;
	}

	inline qint64 MinsTo(const AuElapsedTimer& t) const
	{
		return MSecsTo(t) / d_MSecPerMin;
	}

	inline qint64 MSecsTo(const AuElapsedTimer& t) const
	{
		return t.msecsSinceReference() - msecsSinceReference();
	}

	inline static AuElapsedTimer Now()
	{
		AuElapsedTimer now;
		now.start();
		return now;
	}

	inline qint64 Period(qint64 periodMSec) const
	{
		return msecsSinceReference() / periodMSec;
	}

	inline qint64 Period(qint64 periodMSec, qint64 offsetMSec) const
	{
		return (msecsSinceReference() + offsetMSec) / periodMSec;
	}

	// A return value of true means my start time falls within the same universal second as the start time of 't'.
	inline bool SameStartSec(const AuElapsedTimer& t) const
	{
		return SecsSinceEpoch() == t.SecsSinceEpoch();
	}

	// Returns a number from 0 to 1 which is the fraction of the time into this second.
	inline float SecCycle() const
	{
		return Cycle(d_MSecPerSec);
	}

	inline float SecCycle(qint64 offsetMSec) const
	{
		return Cycle(d_MSecPerSec, offsetMSec);
	}

	inline bool SecIsOdd() const
	{
		return SecsSinceEpoch() & 1;
	}

	// Returns the whole second at which my timer was started.
	inline qint64 SecsSinceEpoch() const
	{
		return msecsSinceReference() / d_MSecPerSec; 
	}

	inline qint64 SecsTo(const AuElapsedTimer& t) const
	{
		return MSecsTo(t) / d_MSecPerSec;
	}

	inline void Set()
	{
		restart();
	}

	// Returns the number of seconds (truncated) from my start second to the start second of 't'.
	inline qint64 StartSecTicks(const AuElapsedTimer& t) const
	{
		return t.SecsSinceEpoch() - SecsSinceEpoch();
	}
};

#endif // AuElapsedTimer_H
