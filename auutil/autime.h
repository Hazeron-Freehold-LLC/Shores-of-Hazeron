//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autime.h,v 1.21 2022/04/14 16:58:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTime_H
#define AuTime_H

#include "auutil.h"
#include <QDateTime>

class QDataStream;

#define d_MSecPerSec					1000					// Milliseconds per Second
#define d_MSecPerMin					(60 * d_MSecPerSec)		// Milliseconds per Minute
#define d_MSecPerHr						(60 * d_MSecPerMin)		// Milliseconds per Hour
#define d_MSecPerDay					(24 * d_MSecPerHr)		// Milliseconds per Day

class AUUTIL_DLL AuTime
{
private:
	qint64								m_MSecsSinceEpoch;

public:
	inline AuTime()
	{
		m_MSecsSinceEpoch = 0;
	}

	inline AuTime(qint64 msecsSinceEpoch)
	{
		m_MSecsSinceEpoch = msecsSinceEpoch;
	}

	inline AuTime(const AuTime& t)
	{
		m_MSecsSinceEpoch = t.m_MSecsSinceEpoch;
	}

public:
	inline const AuTime& operator=(const AuTime& t)
	{
		m_MSecsSinceEpoch = t.m_MSecsSinceEpoch;
		return *this;
	}

	inline const AuTime& operator=(const QDateTime& t)
	{
		m_MSecsSinceEpoch = t.toMSecsSinceEpoch();
		return *this;
	}

	inline bool operator==(const AuTime& t) const
	{
		return m_MSecsSinceEpoch == t.m_MSecsSinceEpoch;
	}

	inline bool operator!=(const AuTime& t) const
	{
		return !operator==(t);
	}

	inline bool operator<(const AuTime& t) const
	{
		return m_MSecsSinceEpoch < t.m_MSecsSinceEpoch;
	}

	inline bool operator<=(const AuTime& t) const
	{
		return m_MSecsSinceEpoch <= t.m_MSecsSinceEpoch;
	}

	inline bool operator>(const AuTime& t) const
	{
		return m_MSecsSinceEpoch > t.m_MSecsSinceEpoch;
	}

	inline bool operator>=(const AuTime& t) const
	{
		return m_MSecsSinceEpoch >= t.m_MSecsSinceEpoch;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	inline AuTime AddDays(qint64 days) const
	{
		return AddMSecs(days * d_MSecPerDay);
	}

	inline AuTime AddMins(qint64 mins) const
	{
		return AddMSecs(mins * d_MSecPerMin);
	}

	inline AuTime AddMSecs(qint64 msecs) const
	{
		return AuTime(MSecsSinceEpoch() + msecs);
	}

	inline AuTime AddHours(qint64 hrs) const
	{
		return AddMSecs(hrs * d_MSecPerHr);
	}

	inline AuTime AddSecs(qint64 secs) const
	{
		return AddMSecs(secs * d_MSecPerSec);
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the current period.
	// 'period' is the length of a period in milliseconds
	inline static float CurrentCycle(qint64 period)
	{
		return (float)(fmod((double)CurrentMSecsSinceEpoch(), (double)period) / (double)period);
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the current day.
	inline static float CurrentDayCycle()
	{
		return CurrentCycle(d_MSecPerDay);
	}

	inline static bool CurrentDayIsEven()
	{
		return !CurrentDayIsOdd();
	}

	inline static bool CurrentDayIsOdd()
	{
		return CurrentDaysSinceEpoch() & 1;
	}

	inline static qint64 CurrentDaysSinceEpoch()
	{
		return CurrentMSecsSinceEpoch() / d_MSecPerDay;
	}

	inline static qint64 CurrentDaysTo(const AuTime& t)
	{
		return t.DaysSinceEpoch() - CurrentDaysSinceEpoch();
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the current hour.
	inline static float CurrentHourCycle()
	{
		return CurrentCycle(d_MSecPerHr);
	}

	inline static bool CurrentHourIsEven()
	{
		return !CurrentHourIsOdd();
	}

	inline static bool CurrentHourIsOdd()
	{
		return CurrentHoursSinceEpoch() & 1;
	}

	inline static qint64 CurrentHoursSinceEpoch()
	{
		return CurrentMSecsSinceEpoch() / d_MSecPerHr;
	}

	inline static qint64 CurrentHoursTo(const AuTime& t)
	{
		return t.HoursSinceEpoch() - CurrentHoursSinceEpoch();
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the current minute.
	inline static float CurrentMinCycle()
	{
		return CurrentCycle(d_MSecPerMin);
	}

	inline static bool CurrentMinIsEven()
	{
		return !CurrentMinIsOdd();
	}

	inline static bool CurrentMinIsOdd()
	{
		return CurrentMinsSinceEpoch() & 1;
	}

	inline static qint64 CurrentMinsSinceEpoch()
	{
		return CurrentMSecsSinceEpoch() / d_MSecPerMin;
	}

	inline static qint64 CurrentMinsTo(const AuTime& t)
	{
		return t.MinsSinceEpoch() - CurrentMinsSinceEpoch();
	}

	inline static bool CurrentMSecIsEven()
	{
		return !CurrentMSecIsOdd();
	}

	inline static bool CurrentMSecIsOdd()
	{
		return CurrentMSecsSinceEpoch() & 1;
	}

	inline static qint64 CurrentMSecsSinceEpoch()
	{
		return QDateTime::currentMSecsSinceEpoch();
	}

	inline static qint64 CurrentMSecsTo(const AuTime& t)
	{
		return t.MSecsSinceEpoch() - CurrentMSecsSinceEpoch();
	}

	inline static qint64 CurrentPeriod(qint64 periodMSec)
	{
		return CurrentMSecsSinceEpoch() / periodMSec;
	}

	inline static qint64 CurrentPeriod(qint64 periodMSec, qint64 offsetMSec)
	{
		return (CurrentMSecsSinceEpoch() + offsetMSec) / periodMSec;
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the current second.
	inline static float CurrentSecCycle()
	{
		return CurrentCycle(d_MSecPerSec);
	}

	inline static bool CurrentSecIsEven()
	{
		return !CurrentSecIsOdd();
	}

	inline static bool CurrentSecIsOdd()
	{
		return CurrentSecsSinceEpoch() & 1;
	}

	inline static qint64 CurrentSecsSinceEpoch()
	{
		return CurrentMSecsSinceEpoch() / d_MSecPerSec;
	}

	inline static qint64 CurrentSecsTo(const AuTime& t)
	{
		return t.SecsSinceEpoch() - CurrentSecsSinceEpoch();
	}

	inline static QDateTime CurrentUTC()
	{
		return Now().UTC();
	}

	// Returns a number from 0 to 1 which is the fraction of the time into the period of this time value.
	// 'period' is the length of a period in milliseconds
	inline float Cycle(qint64 period) const
	{
		return (float)(fmod((double)MSecsSinceEpoch(), (double)period) / (double)period);
	}

	inline float Cycle(qint64 period, qint64 offsetMSec) const
	{
		return (float)(fmod((double)(MSecsSinceEpoch() + offsetMSec), (double)period) / (double)period);
	}

	// Returns a number from 0 to 1 which is the fraction of the time into this day.
	inline float DayCycle() const
	{
		return Cycle(d_MSecPerDay);
	}

	inline bool DayIsEven() const
	{
		return !DayIsOdd();
	}

	inline bool DayIsOdd() const
	{
		return DaysSinceEpoch() & 1;
	}

	inline qint64 DaysSinceEpoch() const
	{
		return MSecsSinceEpoch() / d_MSecPerDay;
	}

	inline qint64 DaysTo(const AuTime& t) const
	{
		return MSecsTo(t) / d_MSecPerDay;
	}

	// Returns the number of days (truncated) from this time to the current time.
	inline qint64 ElapsedDay() const
	{
		return DaysTo(Now());
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

	// Returns the number of milliseconds from this time to the current time.
	inline qint64 ElapsedMSec() const
	{
		return MSecsTo(Now());
	}

	// Returns the number of seconds (truncated) from this time to the current time.
	inline qint64 ElapsedSec() const
	{
		return SecsTo(Now());
	}

	// Returns the number of days (truncated) from this time to the current time.
	// This differs from ElapsedDay in that the result is based on the difference between the whole days of 't' and Now.
	// ElapsedDay is based on the difference in milliseconds between the two.
	inline qint64 ElapsedDayTicks() const
	{
		return CurrentDaysSinceEpoch() - DaysSinceEpoch();
	}

	inline qint64 ElapsedDayTicks(const AuTime& t) const
	{
		return t.DaysSinceEpoch() - DaysSinceEpoch();
	}

	// Returns the number of hours (truncated) from this time to the current time.
	// This differs from ElapsedHour in that the result is based on the difference between the whole days of 't' and Now.
	// ElapsedHour is based on the difference in milliseconds between the two.
	inline qint64 ElapsedHourTicks() const
	{
		return CurrentHoursSinceEpoch() - HoursSinceEpoch();
	}

	inline qint64 ElapsedHourTicks(const AuTime& t) const
	{
		return t.HoursSinceEpoch() - HoursSinceEpoch();
	}

	// Returns the number of seconds (truncated) from this time to the current time.
	// This differs from ElapsedMin in that the result is based on the difference between the whole days of 't' and Now.
	// ElapsedMin is based on the difference in milliseconds between the two.
	inline qint64 ElapsedMinTicks() const
	{
		return CurrentMinsSinceEpoch() - MinsSinceEpoch();
	}

	inline qint64 ElapsedMinTicks(const AuTime& t) const
	{
		return t.MinsSinceEpoch() - MinsSinceEpoch();
	}

	// Returns the number of milliseconds from this time to the current time.
	// This differs from ElapsedMSec in that the result is based on the difference between the whole days of 't' and Now.
	// ElapsedMSec is based on the difference in milliseconds between the two.
	inline qint64 ElapsedMSecTicks() const
	{
		return CurrentMSecsSinceEpoch() - MSecsSinceEpoch();
	}

	inline qint64 ElapsedMSecTicks(const AuTime& t) const
	{
		return t.MSecsSinceEpoch() - MSecsSinceEpoch();
	}

	// Returns the number of seconds (truncated) from this time to the current time.
	// This differs from ElapsedSec in that the result is based on the difference between the whole days of 't' and Now.
	// ElapsedSec is based on the difference in milliseconds between the two.
	inline qint64 ElapsedSecTicks() const
	{
		return CurrentSecsSinceEpoch() - SecsSinceEpoch();
	}

	inline qint64 ElapsedSecTicks(const AuTime& t) const
	{
		return t.SecsSinceEpoch() - SecsSinceEpoch();
	}

	// Returns a number from 0 to 1 which is the fraction of the time into this hour.
	inline float HourCycle() const
	{
		return Cycle(d_MSecPerHr);
	}

	inline bool HourIsEven() const
	{
		return !HourIsOdd();
	}

	inline bool HourIsOdd() const
	{
		return HoursSinceEpoch() & 1;
	}

	inline qint64 HoursSinceEpoch() const
	{
		return MSecsSinceEpoch() / d_MSecPerHr;
	}

	inline qint64 HoursTo(const AuTime& t) const
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

	inline bool IsNewPeriod(const AuTime& now, qint64 periodMSec, qint64 offsetMSec) const
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

	inline bool IsNewPeriod(const AuTime& now, qint64 periodMSec) const
	{
		qint64 thisPeriod = Period(periodMSec);
		qint64 nowPeriod = now.Period(periodMSec);
		return thisPeriod != nowPeriod;
	}

	inline bool IsNull() const
	{
		return MSecsSinceEpoch() == 0;
	}

	// Returns a number from 0 to 1 which is the fraction of the time into this minute.
	inline float MinCycle() const
	{
		return Cycle(d_MSecPerMin);
	}

	inline bool MinIsEven() const
	{
		return !MinIsOdd();
	}

	inline bool MinIsOdd() const
	{
		return MinsSinceEpoch() & 1;
	}

	inline qint64 MinsSinceEpoch() const
	{
		return MSecsSinceEpoch() / d_MSecPerMin;
	}

	inline qint64 MinsTo(const AuTime& t) const
	{
		return MSecsTo(t) / d_MSecPerMin;
	}

	inline bool MSecIsEven() const
	{
		return !MSecIsOdd();
	}

	inline bool MSecIsOdd() const
	{
		return MSecsSinceEpoch() & 1;
	}

	inline qint64 MSecsSinceEpoch() const
	{
		return m_MSecsSinceEpoch;
	}

	inline qint64 MSecsTo(const AuTime& t) const
	{
		return t.m_MSecsSinceEpoch - m_MSecsSinceEpoch;
	}

	inline static AuTime Now()
	{
		return AuTime(CurrentMSecsSinceEpoch());
	}

	inline qint64 Period(qint64 periodMSec) const
	{
		return MSecsSinceEpoch() / periodMSec;
	}

	inline qint64 Period(qint64 periodMSec, qint64 offsetMSec) const
	{
		return (MSecsSinceEpoch() + offsetMSec) / periodMSec;
	}

	// Receives only the bottom two bytes of MSecsSinceEpoch and replaces them in the current Now() time, coordinated with the
	// server's time.
	// 16 bits has a magnitude of 65,535. That's enough for just over a minute of time.
	void Rx16(QDataStream& in, qint64 serverTimeOffset = 0);

	// Receives only the bottom four bytes of MSecsSinceEpoch and replaces them in the current Now() time, coordinated with the
	// server's time.
	// 32 bits has a magnitude of 4,294,967,295. That's enough for just over 49 days of time.
	void Rx32(QDataStream& in, qint64 serverTimeOffset = 0);

	// Returns a number from 0 to 1 which is the fraction of the time into this second.
	inline float SecCycle() const
	{
		return Cycle(d_MSecPerSec);
	}

	inline float SecCycle(qint64 offsetMSec) const
	{
		return Cycle(d_MSecPerSec, offsetMSec);
	}

	inline bool SecIsEven() const
	{
		return !SecIsOdd();
	}

	inline bool SecIsOdd() const
	{
		return SecsSinceEpoch() & 1;
	}

	inline qint64 SecsSinceEpoch() const
	{
		return MSecsSinceEpoch() / d_MSecPerSec; 
	}

	inline qint64 SecsTo(const AuTime& t) const
	{
		return MSecsTo(t) / d_MSecPerSec;
	}

	inline void Set()
	{
		SetMSecsSinceEpoch(CurrentMSecsSinceEpoch());
	}

	inline void SetMSecsSinceEpoch(qint64 msecs)
	{
		m_MSecsSinceEpoch = msecs;
	}

	inline void SetNull()
	{
		SetMSecsSinceEpoch(0);
	}

	// Transmits only the bottom two bytes of the MSecsSinceEpoch.
	// 16 bits has a magnitude of 65,535. That's enough for just over a minute of time.
	void Tx16(QDataStream& out) const;

	// Transmits only the bottom four bytes of the MSecsSinceEpoch.
	// 32 bits has a magnitude of 4,294,967,295. That's enough for just over 49 days of time.
	void Tx32(QDataStream& out) const;

	// Returns this AuTime as a QDateTime with UTC timespec.
	inline QDateTime UTC() const
	{
		return QDateTime::fromMSecsSinceEpoch(m_MSecsSinceEpoch, Qt::UTC, 0);
	}
};

Q_DECLARE_METATYPE(AuTime)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline unsigned int qHash(const AuTime& key)
{
	return key.MSecsSinceEpoch();
}

inline QDataStream& operator<<(QDataStream& out, const AuTime& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTime& t)
{
	t << in;
	return in;
}

AUUTIL_DLL QDebug operator<<(QDebug dbg, const AuTime& t);

#endif // AuTime_H
