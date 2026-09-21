//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausituation.h,v 1.13 2022/02/19 17:32:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSituation_H
#define AuSituation_H

#include "audef.h"
#include "auspecies.h"
#include "autime.h"
#include "dhdna.h"
#include <QList>

class DaCitizen;

class AuSituation
{
protected:
	AuTime								m_CreateTime;			// Time this situation was created.

public:
	AuSituation(const AuTime& createTime);
	AuSituation();
	virtual ~AuSituation();
	AuSituation(const AuSituation& c);

	// Returns a duplicate of this situation, allocated on the heap.
	virtual AuSituation* Clone() const = 0;

	static AuSituation* CreateSituation(quint8 situationType);

	const AuTime& CreateTime() const
	{
		return m_CreateTime;
	}

	// Called for each nu citizen that emerges in/from the afflicted city.
	// The base class default does nothing.
	virtual void InflictCitizen(DaCitizen* citizen) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const = 0;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
	// A non-empty sentence will start with a space, so they will concatenate well.
	// The base class returns an empty string.
	//virtual QString SayNeed(const DaCity* city) const;	//DHH DaCity

	// Returns the quality of the situation.
	// The base class default returns 0.
	virtual quint8 Quality() const;

	// This function is called during the city report, which is about every 13 minutes.
	// Noteworthy information is added to the city's event log.
	// Returns false when this situation should be removed from the city.
	//virtual bool Report(DaCity* city) = 0;	//DHH DaCity

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const = 0;
};

inline QDataStream& operator<<(QDataStream& out, const AuSituation& situation)
{
	situation >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSituation& situation)
{
	situation << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSituationList : public QList<AuSituation*>
{
public:
	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	quint64 MemoryUsage() const;

	// Returns zero or more sentences to briefly describe the needs of the city to remedy these situations.
	// A non-empty return string will start with a space.
	// An empty string is returned if no needs are stated.
	/*DHH DaCity
	inline QString SayNeeds(const DaCity* city) const
	{
		QString needs;
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			needs += (*si)->SayNeed(city);
		return needs;
	}
	*/

	// Finds the first situation of a type found in this list.
	// Returns 0 if none found.
	inline AuSituation* Situation(quint8 situationType) const
	{
		for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
			if((*ci)->Type() == situationType)
				return *ci;
		return 0;
	}

	// Returns true if a situation of the type is contained in this list.
	// Returns false if none found.
	inline bool Contains(quint8 situationType) const
	{
		for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
			if((*ci)->Type() == situationType)
				return true;
		return false;
	}

	inline void InflictCitizen(DaCitizen* citizen) const
	{
		for(const_iterator ci = cbegin(); ci != cend(); ci++)
			(*ci)->InflictCitizen(citizen);
	}

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

	// This function is called during the city report, which is about every 13 minutes.
	// Noteworthy information is added to the city's event log.
	/*DHH DaCity
	inline void Report(DaCity* city)
	{
		iterator ci = begin();
		while(ci != end())
		{
			if((*ci)->Report(city))
				ci++;
			else
			{
				// Time to remove the situation.
				delete *ci;
				ci = erase(ci);
			}
		}
	}
	*/
};

inline QDataStream& operator<<(QDataStream& out, const AuSituationList& situationList)
{
	situationList >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSituationList& situationList)
{
	situationList << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSitPoison : public AuSituation
{
private:
	quint32							m_Antidotes;				// Number of antidotes used.
	quint8							m_Strength;					// Strength of this poisoning.
	quint8							m_Quality;

public:
	AuSitPoison(quint8 strength, quint8 quality, const AuTime& createTime);
	AuSitPoison();
	AuSitPoison(const AuSitPoison& c);

	virtual AuSituation* Clone() const;

	// Called for each nu citizen that emerges in/from the afflicted city.
	virtual void InflictCitizen(DaCitizen* citizen) const;

	// Duration of this situation.
	inline int DurationSeconds() const
	{
		// Calculate so that strength 1, quality 1 = 1 universe day, which is about 51 minutes.
		return (m_Strength+1) * (m_Quality+1) * 86400 / UDaysPerRealDay();
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
	// A non-empty sentence will start with a space, so they will concatenate well.
	// The base class returns an empty string.
	//virtual QString SayNeed(const DaCity* city) const;	//DHH DaCity

	// Returns the quality of the situation.
	virtual quint8 Quality() const;

	// Returns the current strength level of this poison.
	inline int Toxicity() const
	{
		int secsDuration = DurationSeconds();
		int secsAlive = m_CreateTime.ElapsedSec();
		if(secsAlive >= secsDuration)
			return 0;
		
		// The strength starts at its peak and drops off to 0 at the end.
		if(!m_Strength)
			return 0;
		return (m_Strength * (secsDuration - secsAlive) / secsDuration + 9) / 10;
	}

	// This function is called during the city report, which is about every 13 minutes.
	// Noteworthy information is added to the city's event log.
	// Returns false when this situation should be removed from the city.
	//virtual bool Report(DaCity* city);	//DHH DaCity

	static quint8 SituationType()
	{
		return 1;
	}

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSitDisease : public AuSituation
{
private:
	quint32							m_Antitoxins;				// Number of antitoxins used.
	quint8							m_Contagion;				// Contagion level.
	quint8							m_Strength;					// Strength of this disease.
	quint8							m_Quality;					// Quality of antitoxin needed to cure.

public:
	AuSitDisease(quint8 strength, quint8 contagion, quint8 quality, const AuTime& createTime);
	AuSitDisease();
	AuSitDisease(const AuSitDisease& c);

	virtual AuSituation* Clone() const;

	// Called for each nu citizen that emerges in/from the afflicted city.
	virtual void InflictCitizen(DaCitizen* citizen) const;

	// Duration of this situation.
	inline int DurationSeconds() const
	{
		// Calculate so that strength 1, quality 1 = 1 universe day, which is about 51 minutes.
		return (m_Strength+1) * (m_Quality+1) * 86400 / UDaysPerRealDay();
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
	// A non-empty sentence will start with a space, so they will concatenate well.
	// The base class returns an empty string.
	//virtual QString SayNeed(const DaCity* city) const;	//DHH DaCity

	// Returns the quality of the situation.
	virtual quint8 Quality() const;

	// This function is called during the city report, which is about every 13 minutes.
	// Noteworthy information is added to the city's event log.
	// Returns false when this situation should be removed from the city.
	//virtual bool Report(DaCity* city);	//DHH DaCity

	static quint8 SituationType()
	{
		return 2;
	}

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const;

	// Returns the current virulence level of this disease.
	inline int Virulence() const
	{
		int secsDuration = DurationSeconds();
		int secsAlive = m_CreateTime.ElapsedSec();
		if(secsAlive >= secsDuration)
			return 0;

		// The virulence rises to its peak in the first 25% of the disease life then drops to 0 at end.
		if(!m_Strength)
			return 0;
		int peakSecs = secsDuration/4;
		if(secsAlive <= peakSecs && peakSecs)
			return (m_Strength * secsAlive / peakSecs + 9) / 10;				// Slope up to peak strength at 25% of life,
		else
			return (m_Strength - m_Strength * (secsAlive - peakSecs) / (secsDuration - peakSecs) + 9) / 10;	// down from peak to 0.
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSitRatsDNA : public AuSituation
{
private:
	DhDNA							m_DNA;						// DNA of rats.
	quint32							m_Pharmaceuticals;			// Number of pharmaceuticals used.
	quint8							m_Quality;					// Quality of pharmaceuticals needed.
	quint8							m_Voracity;					// Voracity level of rats.

public:
	AuSitRatsDNA(const DhDNA& dna, quint8 voracity, quint8 quality, const AuTime& createTime);
	AuSitRatsDNA();
	AuSitRatsDNA(const AuSitRatsDNA& c);

	virtual AuSituation* Clone() const;

	inline const DhDNA& DNA() const
	{
		return m_DNA;
	}

	// Duration of this situation.
	inline int DurationSeconds() const
	{
		// Calculate so that strength 1 = 1 universe day, which is about 51 minutes.
		return (m_Voracity+1) * (m_Quality+1) * 86400 / UDaysPerRealDay();
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
	// A non-empty sentence will start with a space, so they will concatenate well.
	// The base class returns an empty string.
	//virtual QString SayNeed(const DaCity* city) const;	//DHH DaCity

	// Returns the quality of the situation.
	virtual quint8 Quality() const;

	// This function is called during the city report, which is about every 13 minutes.
	// Noteworthy information is added to the city's event log.
	// Returns false when this situation should be removed from the city.
	//virtual bool Report(DaCity* city);	//DHH DaCity

	static quint8 SituationType()
	{
		return 3;
	}

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSitRatsSpecies : public AuSituation
{
private:
	AuSpecies						m_Species;					// Species of rats.
	quint32							m_Pharmaceuticals;			// Number of pharmaceuticals used.
	quint8							m_Quality;					// Quality of pharmaceuticals needed.
	quint8							m_Voracity;					// Voracity level of rats.

public:
	AuSitRatsSpecies(const AuSpecies& species, quint8 voracity, quint8 quality, const AuTime& createTime);
	AuSitRatsSpecies();
	AuSitRatsSpecies(const AuSitRatsSpecies& c);

	virtual AuSituation* Clone() const;

	// Duration of this situation.
	inline int DurationSeconds() const
	{
		// Calculate so that strength 1 = 1 universe day, which is about 51 minutes.
		return (m_Voracity+1) * (m_Quality+1) * 86400 / UDaysPerRealDay();
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
	// A non-empty sentence will start with a space, so they will concatenate well.
	// The base class returns an empty string.
	//virtual QString SayNeed(const DaCity* city) const;	//DHH DaCity

	// Returns the quality of the situation.
	virtual quint8 Quality() const;

	// This function is called during the city report, which is about every 13 minutes.
	// Noteworthy information is added to the city's event log.
	// Returns false when this situation should be removed from the city.
	//virtual bool Report(DaCity* city);	//DHH DaCity

	static quint8 SituationType()
	{
		return 5;
	}

	inline const AuSpecies& Species() const
	{
		return m_Species;
	}

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSitBounty : public AuSituation
{
private:
	QString								m_Reason;
	double								m_Reward;

public:
	AuSitBounty(const QString& reason, double reward, const AuTime& createTime);
	AuSitBounty();
	AuSitBounty(const AuSitBounty& c);

	virtual AuSituation* Clone() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	inline const QString& Reason() const
	{
		return m_Reason;
	}

	inline double Reward() const
	{
		return m_Reward;
	}

	// This function is called during the city report, which is about every 13 minutes.
	// Noteworthy information is added to the city's event log.
	// Returns false when this situation should be removed from the city.
	//virtual bool Report(DaCity* city);	//DHH DaCity

	static quint8 SituationType()
	{
		return 4;
	}

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuSituation_H
