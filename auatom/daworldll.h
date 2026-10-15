//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldll.h,v 1.322 2022/05/17 18:45:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaWorldLL_H
#define DaWorldLL_H

#include "aucommodityexchange.h"
#include "augeologicalfeature.h"
#include "auglobal.h"
#include "austation.h"
#include "daworld.h"
#include "deconstructionprocess.h"

#define PLANT_VAOS						4						// VAO varieties of each plant specie.

#define d_DeleteTerrainDistance2		(1500*FeetPerMeter()*1500*FeetPerMeter())	// Squared distance to discard terrain patches.

class AuFireParticles;
class AuFogOfWarData;
class AuGear;
class AuGLModelMaker;
class AuRoadMap;
class AuScParts;
class AuScReport;
class AuSmokeParticles;
class AuSpecies;
class AuVillage;
class DaArena;
class DaBd;
class DaItem;
class DaNpc;
class DaPerson;
class DaSentient;
class DePlanetFauna;
class DePlanetFlora;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFenceSegment : public AuSegment
{
public:
	AuPoint								m_FenceTop1;			// Top of fence at end 1.
	AuPoint								m_FenceTop2;			// Top of fence at end 2.
	AuId								m_PlatformId;			// Platform id, if applicable.
	AuPoint								m_PostTop1;				// Top of post at end 1.
	AuPoint								m_PostTop2;				// Top of post at end 2.
	bool								m_UnderConstruction;

public:
	AuFenceSegment()
	{
		m_UnderConstruction = false;
	}

	AuFenceSegment(const AuSegment& s) : AuSegment(s)
	{
		m_UnderConstruction = false;
	}

public:
	inline bool IsUnderConstruction() const
	{
		return m_UnderConstruction;
	}

	inline void Transform(const AuMatrix4x4& m)
	{
		AuSegment::Transform(m);
		m_FenceTop1 = m.Project(m_FenceTop1);
		m_FenceTop1 = m.Project(m_FenceTop2);
		m_PostTop1 = m.Project(m_PostTop1);
		m_PostTop1 = m.Project(m_PostTop2);
	}
};

class AuFenceSegmentArray : public QVector<AuFenceSegment>
{
public:
	inline bool ClosestPost(AuPoint& postLoc, const AuPoint& searchLoc, const AuId& onPlatformId) const
	{
		double closestDist2 = -1;
		for(const_iterator si = cbegin(); si != cend(); si++)
		{
			if((*si).m_PlatformId != onPlatformId)
				continue;
			double dist2 = searchLoc.Distance2((*si).m_fLoc1);
			if(dist2 < closestDist2 || closestDist2 < 0)
			{
				closestDist2 = dist2;
				postLoc = (*si).m_fLoc1;
			}
			dist2 = searchLoc.Distance2((*si).m_fLoc2);
			if(dist2 < closestDist2 || closestDist2 < 0)
			{
				closestDist2 = dist2;
				postLoc = (*si).m_fLoc2;
			}
		}
		return closestDist2 >= 0;
	}

	inline void Transform(const AuMatrix4x4& m)
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).Transform(m);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBldHealth
{
public:
	quint32								m_CurHits;
	bool								m_IsBurning;
	quint32								m_MaxHits;

public:
	AuBldHealth(quint32 curHits, quint32 maxHits, bool isBurning)
	{
		m_CurHits = curHits;
		m_IsBurning = isBurning;
		m_MaxHits = maxHits;
	}
};

class AuBldSprite : public AuBldHealth
{
public:
	QString								m_CityName;
	quint8								m_CompletedLevels;
	INDUSTRYID							m_IID;
	bool								m_IsBd;
	quint8								m_LevelsUnderConstruction;
	QColor								m_StanceColor;

public:
	inline AuBldSprite(const QString& cityName, const QColor& stanceColor, INDUSTRYID iid, bool isUnderConstruction,
			quint32 curHits, quint32 maxHits, bool isBurning) :
		AuBldHealth(curHits, maxHits, isBurning)
	{
		m_CityName = cityName;
		if(isUnderConstruction)
		{
			m_CompletedLevels = 0;
			m_LevelsUnderConstruction = 1;
		}
		else
		{
			m_CompletedLevels = 1;
			m_LevelsUnderConstruction = 0;
		}
		m_IID = iid;
		m_IsBd = true;
		m_StanceColor = stanceColor;
	}

	inline AuBldSprite(const AuBldSprite& c) : AuBldHealth(c)
	{
		m_CityName = c.m_CityName;
		m_CompletedLevels = c.m_CompletedLevels;
		m_IID = c.m_IID;
		m_IsBd = c.m_IsBd;
		m_LevelsUnderConstruction = c.m_LevelsUnderConstruction;
		m_StanceColor = c.m_StanceColor;
	}

public:	// Operators
	inline const AuBldSprite& operator=(const AuBldSprite& c)
	{
		AuBldHealth::operator=(c);
		m_CityName = c.m_CityName;
		m_CompletedLevels = c.m_CompletedLevels;
		m_IID = c.m_IID;
		m_IsBd = c.m_IsBd;
		m_LevelsUnderConstruction = c.m_LevelsUnderConstruction;
		m_StanceColor = c.m_StanceColor;
		return *this;
	}

public:	// Methods
	inline bool IsBdUnderConstruction() const
	{
		return m_IsBd && m_LevelsUnderConstruction;
	}
};

class AuRoadSprite
{
public:
	bool								m_IsLandscaper;
	bool								m_IsSlab;
	bool								m_IsUnderConstruction;
	quint8								m_Material;

public:
	inline AuRoadSprite(bool isLandscaper, bool isSlab, quint8 material, bool isUnderConstruction)
	{
		m_IsLandscaper = isLandscaper;
		m_IsSlab = isSlab;
		m_IsUnderConstruction = isUnderConstruction;
		m_Material = material;
	}

	inline AuRoadSprite(const AuRoadSprite& c)
	{
		m_IsLandscaper = c.m_IsLandscaper;
		m_IsSlab = c.m_IsSlab;
		m_IsUnderConstruction = c.m_IsUnderConstruction;
		m_Material = c.m_Material;
	}

public:	// Operators
	inline const AuRoadSprite& operator=(const AuRoadSprite& c)
	{
		m_IsLandscaper = c.m_IsLandscaper;
		m_IsSlab = c.m_IsSlab;
		m_IsUnderConstruction = c.m_IsUnderConstruction;
		m_Material = c.m_Material;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuTriHash
{
public:
	inline size_t operator()(const int& o) const
	{
		return hash<unsigned int>()(o);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Data to store the location of an enclosure, so they can be located without locking/accessing subatoms.

class AuWorldEnclosure
{
	AuId								m_BdId;					// Building that produced this enclosure.
	AuScParts*							m_EnvVoids;				// Environment voids, projected into the space of the host world.
	bool								m_UnderSea;				// True for under sea enclosure.

public:
	// 'envVoid' locations are in the space of the host world.
	AuWorldEnclosure(const AuId& bdId, const AuScParts& envVoids, bool underSea);
	AuWorldEnclosure(const AuWorldEnclosure& c);
	AuWorldEnclosure();
	~AuWorldEnclosure();

public:	// Operators
	const AuWorldEnclosure& operator=(const AuWorldEnclosure& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	inline const AuId& BdId() const
	{
		return m_BdId;
	}

	// 'worldLoc' is in the space of the host world.
	bool Contains(const AuPoint& worldLoc) const;

	inline const AuScParts* EnvironmentVoids() const
	{
		return m_EnvVoids;
	}

	inline AuScParts* EnvironmentVoids()
	{
		return m_EnvVoids;
	}

	inline bool IsUnderSea() const
	{
		return m_UnderSea;
	}

	inline void SetBdId(const AuId& bdId)
	{
		m_BdId = bdId;
	}

	inline void SetUnderSea(bool isUnderSea)
	{
		m_UnderSea = isUnderSea;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuWorldEnclosure& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuWorldEnclosure& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuWorldEnclosureCache : public QList<AuWorldEnclosure>
{
public:	// Constructors
	inline AuWorldEnclosureCache()
	{
	}

	inline AuWorldEnclosureCache(const AuWorldEnclosureCache& c) : QList<AuWorldEnclosure>(c)
	{
	}

public:	// Methods
	bool AnyNotReady() const;

	// Return: 0=no, 1=yes land enclosure, 2=yes under sea enclosure, -1=maybe because an enclosure is here but its design is not
	// ready
	int Contains(const AuPoint& worldLoc) const;

	// Like Contains but 'exceptId' is ignored.
	int ContainsExcept(const AuPoint& worldLoc, const AuId& exceptId, AuId& enclosureId) const;

	AuId ContainsId(const AuPoint& worldLoc) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Data to store the location of irrigated areas, so they can be located without locking/accessing subatoms.

class AuWorldIrrigation
{
	float								m_EndAlt;				// MSL altitude of road segment end
	AuGlobeLLF							m_EndLL;
	float								m_StartAlt;				// MSL altitude of road segment start
	AuGlobeLLF							m_StartLL;
	int									m_Type;					// 1=irrigation, 2=defoliant
	float								m_Width2;				// 1/2 width of this road or slab.

public:
	inline AuWorldIrrigation(int type, const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL,
			float width2)
	{
		m_EndAlt = endMSL;
		m_EndLL = endLL;
		m_StartAlt = startMSL;
		m_StartLL = startLL;
		m_Type = type;
		m_Width2 = width2;
	}

	inline AuWorldIrrigation()
	{
		m_EndAlt = 0;
		m_StartAlt = 0;
		m_Type = 0;
		m_Width2 = 0;
	}

	inline AuWorldIrrigation(const AuWorldIrrigation& c)
	{
		m_EndAlt = c.m_EndAlt;
		m_EndLL = c.m_EndLL;
		m_StartAlt = c.m_StartAlt;
		m_StartLL = c.m_StartLL;
		m_Type = c.m_Type;
		m_Width2 = c.m_Width2;
	}

public:	// Operators
	inline const AuWorldIrrigation& operator=(const AuWorldIrrigation& c)
	{
		m_EndAlt = c.m_EndAlt;
		m_EndLL = c.m_EndLL;
		m_StartAlt = c.m_StartAlt;
		m_StartLL = c.m_StartLL;
		m_Type = c.m_Type;
		m_Width2 = c.m_Width2;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_EndAlt;
		out << m_EndLL;
		out << m_StartAlt;
		out << m_StartLL;
		out << m_Type;
		out << m_Width2;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_EndAlt;
		in >> m_EndLL;
		in >> m_StartAlt;
		in >> m_StartLL;
		in >> m_Type;
		in >> m_Width2;
	}

public:
	inline float EndAltitudeMSL() const
	{
		return m_EndAlt;
	}

	inline const AuGlobeLLF& EndLL() const
	{
		return m_EndLL;
	}

	inline float StartAltitudeMSL() const
	{
		return m_StartAlt;
	}

	inline const AuGlobeLLF& StartLL() const
	{
		return m_StartLL;
	}

	inline int Type() const
	{
		return m_Type;
	}

	inline float Width2() const
	{
		return m_Width2;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuWorldIrrigation& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuWorldIrrigation& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeTerrainAltInfo
{
public:
	float								m_CityAmbience;
	int									m_HideGrass;			// 0=no, 1=yes, 2=mow, 3=lawn
	WORLDTEX_TYPE						m_PavementType;
	float								m_Shine;
	float								m_Snow;
	double								m_TerAltMSL;
	AuPoint								m_TerCoord;
	AuPoint								m_TerNormal;
	QPointF								m_TerTex;
	quint8								m_Veg;

public:
	AuGlobeTerrainAltInfo()
	{
		m_CityAmbience = 0;
		m_HideGrass = 0;
		m_PavementType = WORLDTEX_Count;
		m_Shine = 0;
		m_Snow = 0;
		m_TerAltMSL = 0;
		m_Veg = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeTerrainTriCen : public AuGlobeLLTri
{
public:
	//AuPoint								m_Center;				// Center point relative to the globe.
	AuPoint								m_Loc0;					// Vertex 0 relative to the globe.
	AuPoint								m_Loc1;					// Vertex 1 relative to the globe.
	AuPoint								m_Loc2;					// Vertex 2 relative to the globe.

public:
	AuGlobeTerrainTriCen(const AuPoint& loc0, const AuPoint& loc1, const AuPoint& loc2, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2) : AuGlobeLLTri(ll0, ll1, ll2)
	{
		m_Loc0 = loc0;
		m_Loc1 = loc1;
		m_Loc2 = loc2;
	}

	AuGlobeTerrainTriCen()
	{
	}

	// Construct using base class and a radius.
	AuGlobeTerrainTriCen(const AuGlobeLLTri& c, double globeRadius) : AuGlobeLLTri(c)
	{
		m_Loc0 = m_LL0.GlobeLocation(globeRadius);
		m_Loc1 = m_LL1.GlobeLocation(globeRadius);
		m_Loc2 = m_LL2.GlobeLocation(globeRadius);
	}

	// Construct using base class and precalculated vertex locations. These are expected to correspond to the long lat locations
	// in 'c'.
	AuGlobeTerrainTriCen(const AuGlobeLLTri& c, const AuPoint& loc0, const AuPoint& loc1, const AuPoint& loc2) : AuGlobeLLTri(c)
	{
		m_Loc0 = loc0;
		m_Loc1 = loc1;
		m_Loc2 = loc2;
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeLLTri::operator<<(in);
		in >> m_Loc0;
		in >> m_Loc1;
		in >> m_Loc2;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeLLTri::operator>>(out);
		out << m_Loc0;
		out << m_Loc1;
		out << m_Loc2;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeTerrainTriCen& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeTerrainTriCen& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVert
{
public:
	AuPoint								m_Loc;					// 3D location of this globe vertex

public:
	AuGlobeVert(const AuPoint& loc)
	{
		m_Loc = loc;
	}

	AuGlobeVert()
	{
	}

	void operator<<(QDataStream& in)
	{
		in >> m_Loc;
	}

	void operator>>(QDataStream& out) const
	{
		out << m_Loc;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVert& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVert& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVertTex : public AuGlobeVert
{
public:
	QPointF								m_TexCoord;				// Texture coordinate at this globe vertex

public:
	AuGlobeVertTex(const AuPoint& loc, const QPointF& texCoord) : AuGlobeVert(loc)
	{
		m_TexCoord = texCoord;
	}

	AuGlobeVertTex()
	{
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeVert::operator<<(in);
		in >> m_TexCoord;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeVert::operator>>(out);
		out << m_TexCoord;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVertTex& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVertTex& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVertAltVeg : public AuGlobeVertTex
{
public:
	double								m_Altitude;				// Altitude of this vertex from planet center. MSL alt on ringworld.
	float								m_Snow;					// Snow coverage (0..1)
	quint8								m_Vegetation;			// Vegetation at the vertex. [0..100]

public:
	AuGlobeVertAltVeg(const AuPoint& loc, const QPointF& texCoord, double altitude, float snow, quint8 vegetation) :
		AuGlobeVertTex(loc, texCoord)
	{
		m_Altitude = altitude;
		m_Snow = snow;
		m_Vegetation = vegetation;
	}

	AuGlobeVertAltVeg()
	{
		m_Altitude = 0;
		m_Snow = 0;
		m_Vegetation = 0;
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeVertTex::operator<<(in);
		in >> m_Altitude;
		in >> m_Snow;
		in >> m_Vegetation;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeVertTex::operator>>(out);
		out << m_Altitude;
		out << m_Snow;
		out << m_Vegetation;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVertAltVeg& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVertAltVeg& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVertAltVegN : public AuGlobeVertAltVeg
{
public:
	AuPoint								m_N;					// Vertex surface normal.

public:
	AuGlobeVertAltVegN(const AuPoint& loc, const QPointF& texCoord, double altitude, float snow, quint8 vegetation,
			const AuPoint& normal) : AuGlobeVertAltVeg(loc, texCoord, altitude, snow, vegetation)
	{
		m_N = normal;
	}

	AuGlobeVertAltVegN()
	{
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeVertAltVeg::operator<<(in);
		in >> m_N;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeVertAltVeg::operator>>(out);
		out << m_N;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVertAltVegN& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVertAltVegN& o)
{
	o << in;
	return in;
}


//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVertAltVegNPave : public AuGlobeVertAltVegN
{
public:
	float								m_CityPresence;			// City presence ambient light intensity.
	float								m_PavementCover;		// Pavement coverage 0 or 1.
	float								m_PavementType;			// TERTEX index in the range 0..1

public:
	AuGlobeVertAltVegNPave(const AuPoint& loc, const QPointF& texCoord, double altitude, float snow, quint8 vegetation,
			const AuPoint& normal, float paveCover, float paveType, float cityPresence) :
		AuGlobeVertAltVegN(loc, texCoord, altitude, snow, vegetation, normal)
	{
		m_CityPresence = cityPresence;
		m_PavementCover = paveCover;
		m_PavementType = paveType;
	}

	AuGlobeVertAltVegNPave()
	{
		m_CityPresence = 0;
		m_PavementCover = 0;
		m_PavementType = 0;
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeVertAltVegN::operator<<(in);
		in >> m_CityPresence;
		in >> m_PavementCover;
		in >> m_PavementType;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeVertAltVegN::operator>>(out);
		out << m_CityPresence;
		out << m_PavementCover;
		out << m_PavementType;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVertAltVegNPave& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVertAltVegNPave& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVertTexN : public AuGlobeVertTex
{
public:
	AuPoint								m_N;					// Vertex surface normal.

public:
	AuGlobeVertTexN(const AuPoint& loc, const QPointF& texCoord, const AuPoint& normal) : AuGlobeVertTex(loc, texCoord)
	{
		m_N = normal;
	}

	AuGlobeVertTexN()
	{
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeVertTex::operator<<(in);
		in >> m_N;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeVertTex::operator>>(out);
		out << m_N;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVertTexN& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVertTexN& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVertTexNShine : public AuGlobeVertTexN
{
public:
	float								m_Shininess;			// specular power

public:
	AuGlobeVertTexNShine(const AuPoint& loc, const QPointF& texCoord, const AuPoint& normal, float shininess) :
		AuGlobeVertTexN(loc, texCoord, normal)
	{
		m_Shininess = shininess;
	}

	AuGlobeVertTexNShine()
	{
		m_Shininess = 0;
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeVertTexN::operator<<(in);
		in >> m_Shininess;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeVertTexN::operator>>(out);
		out << m_Shininess;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVertTexNShine& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVertTexNShine& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeVertTexNShineSnow : public AuGlobeVertTexNShine
{
public:
	float								m_Snow;					// Snow coverage (0..1)

public:
	AuGlobeVertTexNShineSnow(const AuPoint& loc, const QPointF& texCoord, const AuPoint& normal, float shininess, float snow) :
		AuGlobeVertTexNShine(loc, texCoord, normal, shininess)
	{
		m_Snow = snow;
	}

	AuGlobeVertTexNShineSnow()
	{
		m_Snow = 0;
	}

	void operator<<(QDataStream& in)
	{
		AuGlobeVertTexNShine::operator<<(in);
		in >> m_Snow;
	}

	void operator>>(QDataStream& out) const
	{
		AuGlobeVertTexNShine::operator>>(out);
		out << m_Snow;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeVertTexNShineSnow& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeVertTexNShineSnow& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

/*
class AuGlobeTri
{
public:
	AuGlobeVert							m_Vert0;
	AuGlobeVert							m_Vert1;
	AuGlobeVert							m_Vert2;
};
*/

class AuGlobeTriTex
{
public:
	AuGlobeVertTex						m_Vert0;
	AuGlobeVertTex						m_Vert1;
	AuGlobeVertTex						m_Vert2;
};

class AuGlobeTriTexN
{
public:
	AuGlobeVertTexN						m_Vert0;
	AuGlobeVertTexN						m_Vert1;
	AuGlobeVertTexN						m_Vert2;
};

class AuGlobeTriTexNShine
{
public:
	AuGlobeVertTexNShine				m_Vert0;
	AuGlobeVertTexNShine				m_Vert1;
	AuGlobeVertTexNShine				m_Vert2;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeTriTexNShineSnow
{
public:
	AuGlobeVertTexNShineSnow			m_Vert0;
	AuGlobeVertTexNShineSnow			m_Vert1;
	AuGlobeVertTexNShineSnow			m_Vert2;

public:
	void operator<<(QDataStream& in)
	{
		in >> m_Vert0;
		in >> m_Vert1;
		in >> m_Vert2;
	}

	void operator>>(QDataStream& out) const
	{
		out << m_Vert0;
		out << m_Vert1;
		out << m_Vert2;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeTriTexNShineSnow& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeTriTexNShineSnow& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeTriAltVeg
{
public:
	AuGlobeVertAltVeg					m_Vert0;
	AuGlobeVertAltVeg					m_Vert1;
	AuGlobeVertAltVeg					m_Vert2;

public:
	void operator<<(QDataStream& in)
	{
		in >> m_Vert0;
		in >> m_Vert1;
		in >> m_Vert2;
	}

	void operator>>(QDataStream& out) const
	{
		out << m_Vert0;
		out << m_Vert1;
		out << m_Vert2;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeTriAltVeg& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeTriAltVeg& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeTriAltVegN
{
public:
	AuGlobeVertAltVegN					m_Vert0;
	AuGlobeVertAltVegN					m_Vert1;
	AuGlobeVertAltVegN					m_Vert2;

public:
	void operator<<(QDataStream& in)
	{
		in >> m_Vert0;
		in >> m_Vert1;
		in >> m_Vert2;
	}

	void operator>>(QDataStream& out) const
	{
		out << m_Vert0;
		out << m_Vert1;
		out << m_Vert2;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeTriAltVegN& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeTriAltVegN& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeTriAltVegNPave
{
public:
	AuGlobeVertAltVegNPave				m_Vert0;
	AuGlobeVertAltVegNPave				m_Vert1;
	AuGlobeVertAltVegNPave				m_Vert2;

public:
	void operator<<(QDataStream& in)
	{
		in >> m_Vert0;
		in >> m_Vert1;
		in >> m_Vert2;
	}

	void operator>>(QDataStream& out) const
	{
		out << m_Vert0;
		out << m_Vert1;
		out << m_Vert2;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGlobeTriAltVegNPave& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGlobeTriAltVegNPave& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGLTriVAO : protected AuGLVAO
{
public:
	AuElapsedTimer						m_LastSelected;		// Last time the VAO was referenced. Discarded when not needed.
	bool								m_Valid;			// The model is still usable while invalidated.

public:
	inline AuGLTriVAO()
	{
		// m_LastSelected remains null until first Selected.
		m_Valid = true;
	}

	inline AuGLTriVAO(const AuGLTriVAO& c) : AuGLVAO(c)
	{
		m_LastSelected = c.m_LastSelected;
		m_Valid = c.m_Valid;
	}

	inline const AuGLTriVAO& operator=(const AuGLTriVAO& c)
	{
		AuGLVAO::operator=(c);

		m_LastSelected = c.m_LastSelected;
		m_Valid = c.m_Valid;

		return *this;
	}

public:
	inline GLuint Buffer(GLuint bufferIndex) const
	{
		return AuGLVAO::Buffer(bufferIndex);
	}

	// Creates the OpenGL VAO. Be sure to call DeleteGL some time.
	inline void CreateGL(AuOpenGLFunctions& GLf, GLuint vertices, GLuint buffers)
	{
		AuGLVAO::CreateGL(GLf, vertices, buffers);
	}

	// Deletes this VAO.
	inline void DeleteGL()
	{
		AuGLVAO::DeleteGL();
	}

	// Selects no vertex array as the current OpenGL vertex array.
	inline void DeselectGL()
	{
		AuGLVAO::DeselectGL();
	}

	// Draws the current OpenGL vertex array, telling it my number of vertices. It's a good idea to call SelectGL before calling
	// this function. However, after calling SelectGL, you probably need to enable one or more vertex attribute array by calling
	// glEnableVertexAttribArray.
	inline void DrawGL() const
	{
		AuGLVAO::DrawGL();
	}

	inline bool IsOld() const
	{
		return m_LastSelected.isValid() && m_LastSelected.elapsed() > d_MSecPerMin;
	}

	inline bool IsValid() const
	{
		return m_Valid;
	}

	inline void SelectGL()
	{
		m_LastSelected.restart();
		AuGLVAO::SelectGL();
	}

	inline void SelectGLNoTime()
	{
		AuGLVAO::SelectGL();
	}

	inline void SetValid(bool valid)
	{
		m_Valid = valid;
	}

	inline GLuint VAO() const
	{
		return AuGLVAO::VAO();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAtGlobeVAO
{
public:
	float								m_MaxAltMSL;			// Highest MSL altitude in the globe vao.
	float								m_MinAltMSL;			// Lowest MSL altitude in the globe vao.
	GLuint								m_TexLookup;			// Terrain palette lookup table texture based on altitude.
	GLuint		 						m_TerElevTex[5];		// Terrain textures by elevation type.
	AuGLVAO								m_Low;					// Low tessellation of the globe.
	QList<AuGlobeTerrainTriCen>			m_TerrainTris;			// Terrain triangles that are the basis for the high tessellation.
	unordered_map<int, AuGLTriVAO, AuTriHash>	m_TriVAOs;		// High tessellation VAOs of globe tris. Key is m_TerrainTris index.
	QSet<int>							m_TriWait;				// Tris requested but VAO not ready.

public:
	inline AuAtGlobeVAO()
	{
		m_MaxAltMSL = 0;
		m_MinAltMSL = 0;
		m_TexLookup = 0;
		for(int tet=0; tet<5; tet++)
			m_TerElevTex[tet] = 0;
	}

	inline AuAtGlobeVAO(const AuAtGlobeVAO& c) : m_TerrainTris(c.m_TerrainTris), m_TriVAOs(c.m_TriVAOs), m_TriWait(c.m_TriWait)
	{
		m_MaxAltMSL = c.m_MaxAltMSL;
		m_MinAltMSL = c.m_MinAltMSL;
		m_TexLookup = c.m_TexLookup;
		m_Low = c.m_Low;
		for(int tet=0; tet<5; tet++)
			m_TerElevTex[tet] = c.m_TerElevTex[tet];
	}

public:
	inline float AltNoiseFactor(float noiseDeviation) const
	{
		float altRange = m_MaxAltMSL - m_MinAltMSL;
		return noiseDeviation / altRange;
	}

	// Deletes tri VAOs that have not been referenced recently, so too many do not accumulate at a given planet.
	void DeleteOldGlobeTriVAOs(const AuId& worldId, AuGLModelMaker& mm);

	// Deletes tri VAOs that have not been referenced recently, so too many do not accumulate at a given planet.
	void DeleteOldRingworldSectionTriVAOs(const AuId& worldId, AuGLModelMaker& mm);

	void DeleteGlobeTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm);

	void DeleteRingworldSectionTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm);

	void InvalidateGlobeTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm);

	void InvalidateRingworldSectionTriVAO(const AuId& worldId, int triIndex, AuGLModelMaker& mm);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuResKey
{
	quint8								m_LatitudeRegion;
	AuGlobeLL							m_LL;
	quint8								m_ResourceZone;
	quint8								m_VAOIndex;
	quint8								m_WindIndex;

public:
	inline AuResKey(quint8 vaoIndex, quint8 resourceZone, const AuGlobeLL& ll, AuFloraDNA::ZONE latitudeRegion, quint8 windIndex) :
		m_LL(ll)
	{
		m_LatitudeRegion = latitudeRegion;
		m_ResourceZone = resourceZone;
		m_VAOIndex = vaoIndex;
		m_WindIndex = windIndex;
	}

public:	// Operators
	inline bool operator<(const AuResKey& c) const
	{
		if(m_ResourceZone < c.m_ResourceZone)
			return true;
		else if(m_ResourceZone > c.m_ResourceZone)
			return false;

		if(m_LatitudeRegion < c.m_LatitudeRegion)
			return true;
		else if(m_LatitudeRegion > c.m_LatitudeRegion)
			return false;

		if(m_VAOIndex < c.m_VAOIndex)
			return true;
		else if(m_VAOIndex > c.m_VAOIndex)
			return false;

		if(m_WindIndex < c.m_WindIndex)
			return true;
		else if(m_WindIndex > c.m_WindIndex)
			return false;

		return m_LL < c.m_LL;
	}

public:	// Methods
	inline AuFloraDNA::ZONE LatitudeRegion() const
	{
		return (AuFloraDNA::ZONE)m_LatitudeRegion;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeFlora
{
public:
	COMMODITYID							m_CID;
	float								m_CityR;				// City ambient lighting red. (0,0,0) is none.
	float								m_CityG;				// City ambient lighting green. (0,0,0) is none.
	float								m_CityB;				// City ambient lighting blue. (0,0,0) is none.
	float								m_FadeAlpha;			// For fading into/out-of scene.
	float								m_GlowR;				// Glow red. (0,0,0) is no glow.
	float								m_GlowG;				// Glow green. (0,0,0) is no glow.
	float								m_GlowB;				// Glow blue. (0,0,0) is no glow.
	quint8								m_ResourceZone;
	quint16								m_VAOIndex;				// Index to a VAO, not the actual VAO.
	quint8								m_WindIndex;			// Wind vector index influencing this plant. 0 to 9
private:
	AuPoint								m_Loc;
	AuMatrix4x4							m_T;					// World transformation matrix.

public:
	inline AuGlobeFlora()
	{
		m_CID = CID_Invalid;
		m_CityR = 0;
		m_CityG = 0;
		m_CityB = 0;
		m_FadeAlpha = 0;
		m_GlowR = 0;
		m_GlowG = 0;
		m_GlowB = 0;
		m_ResourceZone = 0;
		m_VAOIndex = 0;
		m_WindIndex = 0;
	}

	inline AuGlobeFlora(const AuGlobeFlora& c)
	{
		m_CID = c.m_CID;
		m_CityB = c.m_CityB;
		m_CityG = c.m_CityG;
		m_CityR = c.m_CityR;
		m_FadeAlpha = c.m_FadeAlpha;
		m_GlowB = c.m_GlowB;
		m_GlowG = c.m_GlowG;
		m_GlowR = c.m_GlowR;
		m_Loc = c.m_Loc;
		m_ResourceZone = c.m_ResourceZone;
		m_T = c.m_T;
		m_VAOIndex = c.m_VAOIndex;
		m_WindIndex = c.m_WindIndex;
	}

public:	// Operators
	inline const AuGlobeFlora& operator=(const AuGlobeFlora& c)
	{
		m_CID = c.m_CID;
		m_CityB = c.m_CityB;
		m_CityG = c.m_CityG;
		m_CityR = c.m_CityR;
		m_FadeAlpha = c.m_FadeAlpha;
		m_GlowB = c.m_GlowB;
		m_GlowG = c.m_GlowG;
		m_GlowR = c.m_GlowR;
		m_Loc = c.m_Loc;
		m_ResourceZone = c.m_ResourceZone;
		m_T = c.m_T;
		m_VAOIndex = c.m_VAOIndex;
		m_WindIndex = c.m_WindIndex;
		return *this;
	}

public:	// Methods
	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CID;
	}

	inline const AuPoint& Location() const
	{
		return m_Loc;
	}

	// Create a transformation matrix for placement of this plant.
	inline void SetPosition(const AuPoint& loc, const AuPoint& vup, float rotZ, float scaleX, float scaleY, float scaleZ)
	{
		m_Loc = loc;

		// All plant align themselves to 'up'.
		m_T.LoadIdentity();
		m_T.Translate(m_Loc);
		m_T *= AuPoint(0,0,1).GetAlignmentTransform(vup);
		m_T.RotateZ(rotZ);
		if(scaleX != 1 || scaleY != 1 || scaleZ != 1)
			m_T.Scale(scaleX, scaleY, scaleZ);
	}

	inline const AuMatrix4x4& Transform() const
	{
		return m_T;
	}

	// Calculates alpha for fade-in transparency based on time-in-scene.
	// 'elapsedMs' is since last render frame. Pass negative value to fade away.
	inline float UpdateAlpha(float elapsedMs)
	{
		m_FadeAlpha += elapsedMs/1000.f;
		if(m_FadeAlpha > 1.f)
			m_FadeAlpha = 1.f;
		else if(m_FadeAlpha < 0.f)
			m_FadeAlpha = 0.f;
		return m_FadeAlpha;
	}

	inline float ViewAlpha(float elapsedMs, const AuPoint& viewPt, const AuPoint& viewerPos, double viewRangeFt2,
			float behindViewerRange)
	{
		// Calculate distance to the viewer.
		double dist2 = Location().Distance2(viewerPos);
		if(dist2 > behindViewerRange*behindViewerRange)
		{
			// Clip when too far behind the viewer.
			if(viewPt.z > 0)
			{
				// I am behind the viewer.
				return 0.f;										// I am too far behind the vier; disappear from the scene.
			}
			else
			{
				// Check outside field of view.
				AuPoint vp1(viewPt);
				vp1.Unit();
				if(vp1.Dot(AuPoint(0,0,-1)) < g_Au.FieldOfViewXCos())
					return 0.f;
			}
		}

		// Calculate alpha based on presense within range in the scene.
		if(dist2 > viewRangeFt2)
			return UpdateAlpha(-elapsedMs);						// I am too far away from the viewer; fade away.

		// I am in front of the viewer and in range to be visible. Fade in.
		return UpdateAlpha(elapsedMs);
	}
};

class AuGlobeTree : public AuGlobeFlora
{
public:
	float								m_TrunkLength;
	float								m_TrunkRadius;

public:
	inline AuGlobeTree()
	{
		m_TrunkLength = 0;
		m_TrunkRadius = 0;
	}

	inline AuGlobeTree(const AuGlobeTree& c) : AuGlobeFlora(c)
	{
		m_TrunkLength = c.m_TrunkLength;
		m_TrunkRadius = c.m_TrunkRadius;
	}

public:	// Operators
	inline const AuGlobeTree& operator=(const AuGlobeTree& c)
	{
		AuGlobeFlora::operator=(c);
		m_TrunkLength = c.m_TrunkLength;
		m_TrunkRadius = c.m_TrunkRadius;
		return *this;
	}

public:	// Methods
	inline float TrunkLength() const
	{
		return m_TrunkLength;
	}

	inline float TrunkRadius() const
	{
		return m_TrunkRadius;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGlobeRock
{
public:
	COMMODITYID							m_CID;
	float								m_CityR;				// City ambient lighting red. (0,0,0) is none.
	float								m_CityG;				// City ambient lighting green. (0,0,0) is none.
	float								m_CityB;				// City ambient lighting blue. (0,0,0) is none.
	float								m_FadeAlpha;			// For fading into/out-of scene.
	float								m_GlowR;				// Glow red. (0,0,0) is no glow.
	float								m_GlowG;				// Glow green. (0,0,0) is no glow.
	float								m_GlowB;				// Glow blue. (0,0,0) is no glow.
	quint16								m_VAOIndex;				// Index to a VAO, not the actual VAO.
private:
	AuPoint								m_Loc;
	AuMatrix4x4							m_T;					// World transformation matrix.

public:
	inline AuGlobeRock()
	{
		m_CID = CID_Invalid;
		m_CityR = 0;
		m_CityG = 0;
		m_CityB = 0;
		m_FadeAlpha = 0;
		m_GlowR = 0;
		m_GlowG = 0;
		m_GlowB = 0;
		m_VAOIndex = 0;
	}

	inline AuGlobeRock(const AuGlobeRock& c)
	{
		m_CID = c.m_CID;
		m_CityB = c.m_CityB;
		m_CityG = c.m_CityG;
		m_CityR = c.m_CityR;
		m_FadeAlpha = c.m_FadeAlpha;
		m_GlowB = c.m_GlowB;
		m_GlowG = c.m_GlowG;
		m_GlowR = c.m_GlowR;
		m_Loc = c.m_Loc;
		m_T = c.m_T;
		m_VAOIndex = c.m_VAOIndex;
	}

public:	// Operators
	inline const AuGlobeRock& operator=(const AuGlobeRock& c)
	{
		m_CID = c.m_CID;
		m_CityB = c.m_CityB;
		m_CityG = c.m_CityG;
		m_CityR = c.m_CityR;
		m_FadeAlpha = c.m_FadeAlpha;
		m_GlowB = c.m_GlowB;
		m_GlowG = c.m_GlowG;
		m_GlowR = c.m_GlowR;
		m_Loc = c.m_Loc;
		m_T = c.m_T;
		m_VAOIndex = c.m_VAOIndex;
		return *this;
	}

public:	// Methods
	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CID;
	}

	void CreateVAO(AuOpenGLFunctions& GLf, AuGLVAO& vao) const;

	inline const AuPoint& Location() const
	{
		return m_Loc;
	}

	// Create a transformation matrix for placement of this rock.
	inline void SetPosition(const AuPoint& loc, float rotX, float rotY, float rotZ, float scaleX, float scaleY, float scaleZ)
	{
		m_Loc = loc;

		m_T.LoadIdentity();
		m_T.Translate(m_Loc);
		m_T.RotateX(rotX);
		m_T.RotateY(rotY);
		m_T.RotateZ(rotZ);
		//if((*gi).m_CID == CID_Gems)
			//m_T.Scale(scaleX*100, scaleY*100, scaleZ*100);	// For testing, make them really show up.
		//else
			m_T.Scale(scaleX, scaleY, scaleZ);
	}

	inline const AuMatrix4x4& Transform() const
	{
		return m_T;
	}

	// Calculates alpha for fade-in transparency based on time-in-scene.
	// 'elapsedMs' is since last render frame. Pass negative value to fade away.
	inline float UpdateAlpha(float elapsedMs)
	{
		m_FadeAlpha += elapsedMs/1000.f;
		if(m_FadeAlpha > 1.f)
			m_FadeAlpha = 1.f;
		else if(m_FadeAlpha < 0.f)
			m_FadeAlpha = 0.f;
		return m_FadeAlpha;
	}

	inline float ViewAlpha(float elapsedMs, const AuPoint& viewPt, const AuPoint& viewerPos, double viewRangeFt2,
			float behindViewerRange)
	{
		// Calculate distance to the viewer.
		double dist2 = Location().Distance2(viewerPos);
		if(dist2 > behindViewerRange*behindViewerRange)
		{
			// Clip when too far behind the viewer.
			if(viewPt.z > 0)
			{
				// I am behind the viewer.
				return 0.f;										// I am too far behind the vier; disappear from the scene.
			}
			else
			{
				// Check outside field of view.
				AuPoint vp1(viewPt);
				vp1.Unit();
				if(vp1.Dot(AuPoint(0,0,-1)) < g_Au.FieldOfViewXCos())
					return 0.f;
			}
		}

		// Calculate alpha based on presense within range in the scene.
		if(dist2 > viewRangeFt2)
			return UpdateAlpha(-elapsedMs);						// I am too far away from the viewer; fade away.

		// I am in front of the viewer and in range to be visible. Fade in.
		return UpdateAlpha(elapsedMs);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaWorldLL : public DaWorld
{
protected:
	// Unguarded
	mutable quint8						m_BdBits;				// Bits: b0=unused, b1=unused, b2=bd here, b3=bd unknown
																// b4=hq here, b5=hq unknown, b6=base here, b7=base unknown
	mutable qint8						m_GeoConstruction;		// Roads and landscaping. 0=no, 1=yes, -1=unknown
	mutable qint8						m_HengePresence;		// 0=no henges, 1=henge, -1=unknown

protected:
	mutable QReadWriteLock				m_LLLock;
	AuId								m_CaptId;				// Id of officer deployed from this world.
	QString								m_CaptName;				// Name of officer ready/deployed from this world.
	AuCommoditySuppliers				m_CitizenSuppliers;		// Buildings with citizens available, similar to commodity exchange.
	AuGlobeLLF							m_ClaimAtLL;			// The location of the flag pole.
	quint32								m_ClaimEmpire;			// Empire who has staked a claim to this world.
	AuId								m_ClaimPersonId;		// Person who staked the claim.
	QString								m_ClaimPersonName;		// Person who staked the claim.
	AuTime								m_ClaimTime;			// Last time a claim was made.
	AuGeologicalFeatures				m_GeologicalFeatures;	// Craters, chasms, volcanoes, etc.
	AuRoadMap*							m_RoadMap;				// Map of road and building connections. Not stored in db.
	AuElapsedTimer						m_tmGeologyChange;		// m_GeologicalFeatures change
	AuCommoditySuppliers				m_TroopSuppliers;		// Buildings with troops available, similar to commodity exchange.

	// FindTerrainAtPixel cache
	mutable bool						m_FindTerrainAtPixelDetailsOk;	// Result of SightTargetDetails test.
	mutable AuPoint						m_FindTerrainAtPixelEye;		// FindTerrainAtPixel eyeLoc arg cached.
	mutable AuPoint						m_FindTerrainAtPixelLook;		// FindTerrainAtPixel sightedLoc arg cached.
	mutable AuPoint						m_FindTerrainAtPixelResult;		// Resulting sighted location on this world, if any.

	// SightTarget cache
	mutable bool						m_SightDetailsOk;		// Result of SightTargetDetails test.
	mutable AuPoint						m_SightedLoc;			// Resulting sighted location on this world, if any.
	mutable AuPoint						m_SightEye;				// SightTarget eyeLoc arg cached.
	mutable int							m_SightIntersect;		// Result of zone sphere Intersection test.
	mutable AuPoint						m_SightLook;			// SightTarget sightedLoc arg cached.

private:
	mutable QMutex						m_BuildingMapLock;
	mutable QMap<AuGlobeLLF, AuIdList>	m_BuildingMap;		// Key is rounded globe loc, value lists Bd atoms there.

private:
	// Enclosures enable recursive environment queries to answer InEnclosure without locking subatoms.
	// Not all worlds have Enclosures. Enclosure data is not stored.
	mutable QReadWriteLock				m_EnclosureLock;
	mutable AuWorldEnclosureCache*		m_Enclosures;			// Allocated when an enclosure exists that has life support and is
	mutable bool						m_EnclosuresValid;		// not under construction.
	QList<AuWorldIrrigation>*			m_Irrigation;

private:
	// Landing sites, one for every resource zone when requested. Not serialized.
	mutable QReadWriteLock				m_LandingSitesLock;
	mutable AuPoint*					m_LandingSites;			// 0,0 means no landing site found in the zone.

private:
	mutable QReadWriteLock				m_TroopsAtStationsLock;
	mutable unordered_map<AuId, int, AuIdHash>	m_TroopsAtStations;		// Troops at stations, by city id. (Only on scene worker)

private:
	mutable QReadWriteLock				m_VillageLock;
	AuVillage*							m_Village;

private:
	// OpenGL
	AuFireParticles*					m_HengeFire;			// Fire particle system for henge fire pits.

	// Smoke particle generator for volcanos, vents, bomb craters, etc.
	QMap<AuGlobeLLF, AuSmokeParticles*>	m_PSSmoke;				// Smoke particle emitters.

protected:
	QMap<AuResKey, AuGlobeFlora>		m_AquaticShrubs;		// Shrubs near m_FloraViewer location.
	GLuint								m_AsphaltTex;
	GLuint								m_ClaimFlagTex;
	quint32								m_ClaimFlagTexEmpire;	// Empire whose flag the tex is.
	AuGLVAO								m_ClaimPoleVAO;			// Flag pole at claim.
	GLuint								m_ClaimPoleTex;
	GLuint								m_ConcreteTex;
	QHash<int, AuGLVAO>					m_SpriteVAO;			// key is size
	mutable QMap<AuPoint, AuBldSprite>			m_OverlayBuildings;
	AuPoint								m_OverlayBuildingsLoc;
	mutable QMap<AuPoint, AuRoadSprite>			m_OverlayRoads;
	AuPoint								m_OverlayRoadsLoc;
	mutable QMultiHash<COMMODITYID, AuPoint>	m_OverlayRocks;	// Icons visible in rocks overlay in top down view.
	AuPoint								m_OverlayRocksLoc;		// Location of scene atom when the overlay was made.
	mutable QMultiHash<COMMODITYID, AuPoint>	m_OverlayShrubs;// Icons visible in plants overlay in top down view.
	AuPoint								m_OverlayShrubsLoc;		// Location of scene atom when the overlay was made.
	mutable QMultiHash<COMMODITYID, AuPoint>	m_OverlayTrees;	// Icons visible in plants overlay in top down view.
	AuPoint								m_OverlayTreesLoc;		// Location of scene atom when the overlay was made.
	QMap<AuResKey, AuGlobeRock>			m_Rocks;				// Rocks near m_RockViewer location.
	QMap<AuResKey, AuGlobeFlora>		m_Shrubs;				// Shrubs near m_FloraViewer location.
	QMap<AuResKey, AuGlobeTree>			m_Trees;				// Trees near m_FloraViewer location.

public:
	DaWorldLL();
	virtual ~DaWorldLL();

protected:
	DaWorldLL(const DaWorldLL& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// 'previewType' 0=none, 1=empty space, 2=sun, 3=sun and planet, 4=sun, planet and moon, 5=inferno, 6=frigid, 7=gas giant,
	//   8=ringworld, 9=sea bottom
	AuPoint AvatarPreviewLocation(quint8 previewType) const;

	AuIdList BuildingsAt(const AuGlobeLLF& ll) const;

	// Configures 'mBld' for a building to be build at 'buildLoc' on this world so the gravity normal of the world points up, at the
	// center of the building site.
	// 'buildLoc' is the insertion point of the building design, which is rarely at the center of the building site.
	// 'siteCenter' is the site center, in the space of the building design.
	// 'zRotation' is the rotation of the building around its up vector.
	// 'onPlatform' is the platform, if any; may be 0.
	void BuildingConstructionTransform(const AuPoint& buildLoc, double zRotation, const DaBd* onPlatform, const AuPoint& siteCenter,
			AuMatrix4x4& mBld) const;

	// Returns the z rotation needed to face a building to the north, when placed at 'loc'.
	// 'onPlatform' is the platform, if any; may be 0.
	double BuildingConstructionNorthAngle(const AuPoint& loc, const DaBd* onPlatform) const;

	// Capture all facilities on this world. New owner is 'empireId'. Called when a new military base is built.
	void Capture(quint32 empireId, const AuId& capturedById, const QString& capturedByName);

	// Clears the buildings and ambient light maps.
	void ClearMaps();

	void DestroyCities();

	virtual void EmpireDeleted(quint32 empireId);

	// Clears fog of war in a 'radius' centered at 'seaLevelLoc' for 'avatarId' and 'empireId'.
	virtual void Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double radius) = 0;

	// Clears fog of war worldwide for 'avatarId' and 'empireId'.
	virtual void ExploreAll(const AuId& avatarId, quint32 empireId) = 0;

	void FindBdsInsideEnclosure(const AuId& enclosureId, DaAtomList& bds) const;

	void FindBdsOnPlatform(const AuId& platformId, DaAtomList& bds) const;

	bool FindClosestFencePost(AuPoint& postLoc, const AuPoint& searchLoc, const AuId& onPlatformId);

	virtual DaWorldLL* FindParentWorldInEnv();

	virtual const DaWorldLL* FindParentWorldInEnv() const;

	// Search for a platform location that contains pxFind.
	// 'tModel' already includes 'this' atom's transform.
	DaBd* FindPlatformLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp, const AuMatrix4x4& tDeviceOrtho,
			const AuMatrix4x4& tModel, AuPoint& platformLoc) const;

	// Search for a sea level world location that contains pxFind.
	// 'tModel' already includes 'this' atom's transform.
	bool FindSeaLevelLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp, const AuMatrix4x4& tDeviceOrtho,
			const AuMatrix4x4& tModel, AuPoint& terrainLoc) const;

	// Search for a world location that contains pxFind.
	// 'tModel' already includes 'this' atom's transform.
	bool FindTerrainLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp, const AuMatrix4x4& tDeviceOrtho,
			const AuMatrix4x4& tModel, AuPoint& terrainLoc) const;

	// Retrieves a copy of the fog of war data for 'empireId' or 'avatarId'.
	// Returns false if the world has no fog of war data. i.e. ringworld
	virtual bool FogOfWar(const AuId& avatarId, quint32 empireId, AuFogOfWarData& fow) const = 0;

	// Return true if 'll' is explored by 'empireId' or 'avatarId'.
	// Return false if 'll' is obscured by fog of war.
	virtual bool FogOfWarTest(const AuId& avatarId, quint32 empireId, const AuGlobeLL& ll) const = 0;

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Called by a building when it has a citizen available for work, even if restricted to its own empire.
	void CitizenAvailable(const AuId& bdId, quint32 empireId);

	// Called when a building no longer has a citizen available for work.
	void CitizenNotAvailable(const AuId& bdId);
	
	// Query for suppliers when a building in an empire wants to import a worker.
	void CitizenSuppliers(quint32 importEmpireId, AuIdList& suppliers);

	// Query all buildings for supplies of citizens.
	void CitizenSupplierUpdate();

	// Takes a citizen from a non-essential job at a civilian building if possible.
	// Citizens get taken like this to work construction and other essential jobs when nobody else is available.
	// 'tradeConnections' are those available to 'toWorkAtBdId'.
	bool CitizenTakeNonessential(const AuId& toWorkAtBdId, QHash<AuId, int>& tradeConnections);

	DaBd* CreateBuilding(quint32 designId, quint8 quality, INDUSTRYID industryId, COMMODITYID material, const AuGlobeLLF& atLL,
			float zRotation, quint32 empireId, const AuSpecies& residentSpecies);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	// 'geoLL' must exactly match the LL() of the feature to delete. Only deletes deletable features.
	bool DeleteGeologicalFeature(const AuGlobeLLF& geoLL);

	// This is helpful when the client want to know if an empire record is needed.
	virtual quint32 EmpireId() const
	{
		return ClaimedByEmpire();
	}

	// Finds geological features that contain 'myLoc'. Specifically looks for geo features that are under construction and
	// geo roads that are completed. Returns IID_None if none is there.
	// 'geoLL' is set to the LL of the geo on return.
	INDUSTRYID FindGeoIndustryContaining(const AuPoint& loc, AuGlobeLLF& geoLL) const;

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	// 'onSlabInCity' is the id of the city in which the requestor is sitting on a road slab. May be null.
	virtual DaVehicle* FindNewVehicle(COMMODITYID vehicleCID, unsigned int empireId, const AuId& onSlabInCity);

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	// 'onSlabInCity' is the id of the city in which the requestor is sitting on a road slab. May be null.
	virtual DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, unsigned int empireId, const AuId& onSlabInCity);

	// Used by DaBd
	// 'fetcher' is the DaBd that is fetching a vehicle.
	virtual DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher);

	// 'geoLL' is the exact LL of the geo.
	// Returns OG_None if the geo does not override natural terrain ground, or if 'geoLL' is not found.
	ON_GROUND GeoGround(const AuGlobeLLF& geoLL) const;

	// Returns the location above or below 'loc' that is at ground level, using TerrainAltitudeMSL.
	AuPoint GroundLoc(const AuPoint& loc) const;

	// Creates a new animal encounter table.
	// Destroyes current animal encounter table, if any.
	virtual void InitAnimalEncounterTable(const AuDice& dice)
	{
		Q_UNUSED(dice);
	}

	// Called for a building that is being created, to join the city jurisdiction in which it was built, if any.
	void JurisdictionJoin(DaBd* nuBld);

	// Called for a nuPolice station that is being created, to join the city jurisdiction in which it was built, if any.
	void JurisdictionPoliceJoin(DaBd* nuPolice);

	// Updates all jurisdictions.
	void JurisdictionUpdate();

	virtual quint64 MemoryUsage() const;

	// Creates a single building.
	DaBd* PirateBaseBuilding(INDUSTRYID industryId, COMMODITYID material, const AuGlobeLLF& atLL, float zRotation, quint32 empireId,
			const AuSpecies& residentSpecies);

	// Creates the initial HQ for a pirate base on this world. The base manages itself after that.
	void PirateBaseEncounter();

	// Creates initial roads for a pirate base of 'empireId'.
	// Returns true if any roads are created.
	bool PirateBaseRoads(quint32 empireId, int rank, const AuGlobeLLF& baseLL);

	bool PirateBaseRoad(quint32 empireId, const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, double width);

	virtual void ProcessDemandTally(const AuId& idCity, const AuItem& item,
			unsigned int& stock, unsigned int& population, unsigned int& needCount, unsigned int& produceCount) const;

	virtual void QueryTreeTrunkSize(COMMODITYID treeCID, int resourceZone, AuFloraDNA::ZONE latRegion, float& trunkLength,
			float& trunkRadius) const;

	// This function prefers locations on land in daylight.
	virtual AuPoint RandomAvatarLocation() const;

	virtual int ResourceZone(const AuPoint& loc) const
	{
		AuGlobeLLF ll;
		if(!LLOfLoc(ll, loc))
			return 0;
		return ResourceZone(ll.Longitude());
	}

	void RoadConnections(const AuId& bdId, QSet<AuId>& connectedBdsAndCities) const;

	// Finds vehicles that are outside on the ground in a city on this planet that can be used for production processes.
	// Only one of each vehicle type is wanted/needed.
	virtual void TallyVehicles(const AuId& cityId, quint32 empireId, QList<COMMODITYID>& vehicles) const;

	// Called by a building when it has a citizen available for work, even if restricted to its own empire.
	void TroopAvailable(const AuId& bdId, quint32 empireId);

	// Called when a building no longer has a citizen available for work.
	void TroopNotAvailable(const AuId& bdId);
	
	// Query for suppliers when a building in an empire wants to import a worker.
	void TroopSuppliers(quint32 importEmpireId, AuIdList& suppliers);

	// Query all buildings for supplies of citizens.
	void TroopSupplierUpdate();

	// Takes a troop from a non-essential job at a military building if possible.
	// Troops only get taken like this to work construction jobs when nobody else is available.
	// 'tradeConnections' are those available to 'toWorkAtBdId'.
	bool TroopTakeNonessential(const AuId& toWorkAtBdId, QHash<AuId, int>& tradeConnections);

	//--------------------
	// DaWorldLL Functions
public:
	void AddBombCrater(const AuPoint& explodeLoc, float bottomMSL, float craterRadius, float debrisHeight);

	// 'completeConstruction' is false to create a geo landscaper that is under construction.
	// 'completeConstruction' is true to create a geo landscaper that is completely constructed.
	void AddLandscape(const AuPoint& startLoc, const AuPoint& endLoc, float width, bool hideRocks, int hideGrass,
			bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement, bool completeConstruction);

	// 'completeConstruction' is false to create a geo road that is under construction.
	// 'completeConstruction' is true to create a geo road that is completely constructed.
	void AddRoad(const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, float width, quint8 geoType,
			bool completeConstruction);

	void BdAdded();

	void BdRemoved(const AuId& bdId);

	// Uses enclosure cache to return the enclosure that contains 'loc'.
	AuId EnclosureContaining(const AuPoint& loc) const;

	// Creates a new item atom at 'll'.
	DaItem* CreateItem(const AuGlobeLLF& ll, const AuItem& item);

	// 'name' is the name of the ship. It will get a random name if 'name' is blank.
	DaVessel* CreateSpacecraft(const AuGlobeLLF& ll, double altAGL, quint32 designId, quint8 quality, quint32 empireId,
			const QString& name);

	DaVehicle* CreateVehicle(const AuGlobeLLF& ll, COMMODITYID cid, quint8 quality, quint32 empire);

	void DumpGeo() const;

	// Returns true if an enemy unit is found on the ground in the box radius of the building site that blocks construction.
	// Not recursive.
	bool EnemyUnitBlocksConstruction(const AuMatrix4x4& mBld, const AuPoint& siteBoxRadius, quint32 empireId) const;

	bool EnemyUnitBlocksDestruction(quint32 empireId) const;

	// Returns glow added to all flora on this world.
	virtual int FloraGlow() const
	{
		return 0;
	}

	virtual void ActivateHengeContaining(const AuPoint& containingLoc) = 0;

	// Returns true if the portal exists, regardless of whether we created it just now.
	bool HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment);

	// 'centerRadius' is the radius of the inner ring of large stones.
	// 'columnRadius' is the radius of the outer ring of stone columns with lintels.
	// Returns false if there is no henge at 'hengeLL'.
	bool HengeRadii(const AuGlobeLLF& hengeLL, double& centerRadius, double& columnRadius) const;

	// 'sys', 'destSun' and 'destArcSegment' are used to cache the result.
	void HengeCreated(DaSolarSystem* sys, const AuId& mySectorId, const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels,
			quint8 fromArcSegment, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment);

	AuGlobeLLF HengeLL() const;

	void HengeTeleportArrived(DaAtom* arrived, const AuId& fromSysId, quint8 fromSun, quint8 fromArc);

	void IndigenousAttackedBy(const AuId& attackerId);

	// 'isContact' is true to update the village's last contacted time. This prevents it from moving for an hour.
	void IndigenousHelpedBy(const AuId& personId, bool isContact);

	// Starts the indigenous henge ceremony at 'hengeLL'.
	// 'state' is 0=inactive, 1=active
	void IndigenousHengeCeremony(const AuGlobeLLF& hengeLL, int state);

	void IndigenousInjuredBy(const AuId& attackerId);

	void IndigenousKilledBy(const AuId& attackerId);

	QString IndigenousSayClosestCity(const AuGlobeLLF& speakerLL) const;

	QString IndigenousSayGeologicalFeature(const AuGlobeLLF& speakerLL, bool onlyHoly) const;

	QString IndigenousSayLargestCity(const AuGlobeLLF& speakerLL) const;

	QString IndigenousSayNewestCity(const AuGlobeLLF& speakerLL) const;

	QString IndigenousSayOldestCity(const AuGlobeLLF& speakerLL) const;

	QString IndigenousSaySmallestCity(const AuGlobeLLF& speakerLL) const;

	bool IsHengeActive() const;

	// Compares 'll' literally with the geological LL of the henge.
	bool IsHengeActive(const AuGlobeLLF& ll) const;

	// Returns true if 'loc' is in the activation area of a henge, for testing preon location for activation.
	// The activation area is any point within OuterSlabRadius distance of the center of the henge.
	bool IsHengeActivationLocation(const AuPoint& loc) const;

	// Checks to see if loc is within 100m of the henge center point.
	// 'hengeLL' is an output parameter on true return.
	bool IsHengeNear(const AuPoint& loc, AuGlobeLLF& hengeLL) const;

	// Returns true if 'loc' is in the teleport area of an active henge.
	// 'from', 'dest', and 'shadowPanels' variables are all output parameters.
	virtual bool IsHengeTeleportLocation(const AuPoint& loc, AuId& fromSysId, quint8& fromSun, quint8& fromArc,
			quint8& shadowPanels, AuId& destSysId, quint8& destSun, quint8& destArc) const;

	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho);

	// Returns true if this world is ok for a person to respawn who has no place else to respawn.
	// Returns false if !IsSuitableHomePlanet.
	// Returns false if this world is claimed by a different empire than 'empireId'.
	// Returns false if any developments are here that belong to an empire other than 'empireId'.
	bool IsSuitableRespawnPlanet(quint32 empireId) const;

	// Change the material of an existing geo road.
	// Returns:
	//   0 = Success. The road is placed into an "under construction" state requiring the new materials.
	//   1 = cpid is invalid.
	//   2 = Road is under construction.
	//   3 = Road was already built using that construction process.
	//  11 = No road exists at devLL.
	//  21 = Asphalt road cannot be built on inferno world.
	//  22 = City does not exist.
	int ChangeRoadConstruction(const AuGlobeLLF& roadDevLL, CONSTRUCTIONPROCESSID cpid);

	// Returns the amount of altitude change in feet per whole terrain data altitude increment.
	virtual double AltitudeIncrement() const = 0;

	// 'mode': 0=normal, 1=any animal ignoring max pop, 2=aerial animal ignoring max pop
	int AnimalEncounter(const AuGlobeLLF& nearLL, int mode);

	// 'person' is the person triggering the encounter. Their BdId() is not null.
	// Inside certain buildings, the nearest surgery and transporter station are manned by an Npc, if currently unmanned.
	void CitizenEncounter(const DaPerson& person);

	// Creates an indigenous village at 'atLL'. Forces movement of current village if necessary.
	// 'dna' is used only if current indigenous DNA is not established.
	void CreateIndigenousVillage(const AuGlobeLLF& atLL, const AuSpecies& species);

	// Test for encounter with indigenous natives.
	// 'person' is the person triggering the encounter.
	// 'mode': 0=normal, 1=force encounter, 2=force village to move and force encounter
	int IndigenousEncounter(const DaPerson& person, const AuGlobeLLF& nearLL, int mode);

	// Determine foliage density attributed to water and atmosphere.
	// Returns a number from 0 to 100.
	unsigned char AverageFoliage() const;

	// True if military buildings exist that are not the HQ.
	bool BaseHere() const;

	virtual bool BdHere() const;

	bool HQHere() const;

	// True if military buildings exist that are not the HQ or if the state of them is unknown. Does NOT call CalcBasePresence.
	bool MaybeBaseHere() const;

	// True if military HQ exists. Does NOT call CalcHQPresence.
	bool MaybeHQHere() const;

	// Called by when a building has changed, to invalidate the 3D model of the terrain affected by the building area.
	virtual void BuildingChanged(const AuMatrix4x4& bldM, double radius, AuGLModelMaker& mm);

	// True if high mountain terrain is snow capped. This does not imply the presense of high mountain terrain, just that it will
	// be snow capped if it is high enough.
	inline bool CanHaveSnow() const
	{
		if(OrbitZone() != OZONE_Inferno &&						// No snow in Inferno zone.
				Hydrographics() > 15 &&							// Water is required.
			   	MSLAtmosphereDensity() >= .4f)					// Atmosphere density is required.
			return true;
		return false;
	}

	// Test whether animals can exist at a specific terrain level on this
	// planet. Inner and outer worlds restrict animals to the seas and coasts.
	virtual bool CanHaveFaunaAtElev(TER_ELEV_TYPE terType) const = 0;

	inline AuId CaptId() const
	{
		m_LLLock.lockForRead();
		AuId v(m_CaptId);
		m_LLLock.unlock();
		return v;
	}

	inline QString CaptName() const
	{
		m_LLLock.lockForRead();
		QString v(m_CaptName);
		m_LLLock.unlock();
		return v;
	}

	float CityAmbientLight(const AuGlobeLLF& ll, WORLDTEX_TYPE pavementType) const;

	// WARNING: m_LLLock must already be locked for write to call this. Used by geo roads when rendering.
	float BuildingAmbientLight(const AuGlobeLLF& ll) const;

	inline AuGlobeLLF ClaimedAtLL() const
	{
		m_LLLock.lockForRead();
		AuGlobeLLF v(m_ClaimAtLL);
		m_LLLock.unlock();
		return v;
	}

	inline quint32 ClaimedByEmpire() const
	{
		m_LLLock.lockForRead();
		quint32 v = m_ClaimEmpire;
		m_LLLock.unlock();
		return v;
	}

	virtual AuId ClaimedByPersonId() const
	{
		m_LLLock.lockForRead();
		AuId v(m_ClaimPersonId);
		m_LLLock.unlock();
		return v;
	}

	inline QString ClaimedByPersonName() const
	{
		m_LLLock.lockForRead();
		QString v(m_ClaimPersonName);
		m_LLLock.unlock();
		return v;
	}

	inline AuTime ClaimTime() const
	{
		m_LLLock.lockForRead();
		AuTime v(m_ClaimTime);
		m_LLLock.unlock();
		return v;
	}

	void ConstructGeo();

	virtual void CreateAtGlobeSurfaceTriangles(QList<AuGlobeTriAltVegN>& tris, float& maxAltMSL, float& minAltMSL) const = 0;

	// Used when creating the at-globe model.
	virtual void CreateAtSurfaceTriangles(QList<AuGlobeTerrainTriCen>& tricens) const = 0;

	// 'running' becomes false when the thread who called me is killed, so I can stop midstream.
	virtual void CreateAtTerrainTriangles(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLLTri& terTri,
			const bool& running) const = 0;

	virtual void CreateFlora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs,
			QMap<AuResKey, AuGlobeTree>& trees, QMap<AuResKey, AuGlobeFlora>& aquaticgrass,
			QMap<AuResKey, AuGlobeFlora>& aquaticshrubs, const AuPoint& groundLoc, quint32 worldGenSeed, int floraGlow) const = 0;

	virtual void CreateNearGlobeSurfaceTriangles(QList<AuGlobeTriTexNShineSnow>& tris) const = 0;

	// 'range' is the radius around 'groundLoc' to create rock resources.
	// 'visibleOnly' is true to only create the rocks that get rendered. False is used when determining rock resources for a mine
	// or well or drilling rig.
	virtual void CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, quint32 worldGenSeed,
			float range, bool visibleOnly) const = 0;

	static GLuint CreateTerrainTexture(AuGLEffects& re, WORLDTEX_TYPE worldTexType, int tileSize);

	DaDesignBase* DesignEnvironment(const AuGlobeLLF& devLL, int panelIndex);

	// Adds hangared spacecraft to the list that are subordinate to this world. Not recursive.
	void FindHangaredSpaceships(DaAtomList& spaceships) const;

	// Returns true if any enclosure exists on this world that has life support and is not under construction.
	bool HasEnclosure() const;

	// Returns true if any enclosure exists on this world that has an invalid design cache, or if the enclosure cache is not ready,
	// as when loading from the database.
	bool HasEnclosureNotReady() const;

	// Sets my enclosure cache into a not ready state so nobody will be hurt by my environment until the enclosure cache is updated.
	// This is necessary when this world is loaded from the database and it begins loading its buildings.
	void SetEnclosureNotReady();

	// Caches result into m_HengePresence when it is not known.
	bool HasHenge() const;

	bool HasGeoConstruction() const;

	// Returns true if any building exists on this world that is inside 'enclosureId'.
	bool HasMilitaryInEnclosure(const AuId& enclosureId) const;

	// Returns true if any building exists on this world that is on 'platformId'.
	bool HasMilitaryOnPlatform(const AuId& platformId) const;

	// Checks if 'll' at 'altMSL' is within the enclosed area of an enclosure.
	// Return: 0=no, 1=yes land enclosure, 2=yes under sea enclosure, -1=maybe because a enclosure is here but its design is not
	// ready
	inline int InEnclosure(const AuGlobeLLF& ll, double altMSL) const
	{
		return InEnclosure(AuGlobeLL(ll.Longitude(), ll.Latitude()), altMSL);
	}

	// Checks if 'll' at 'altMSL' is within the enclosed area of an enclosure.
	// Return: 0=no, 1=yes land enclosure, 2=yes under sea enclosure, -1=maybe
	int InEnclosure(const AuGlobeLL& ll, double altMSL) const;

	// Like InEnclosure but 'exceptId' enclosure is ignored.
	inline int InEnclosureExcept(const AuGlobeLLF& ll, double altMSL, const AuId& exceptId, AuId& enclosureId) const
	{
		return InEnclosureExcept(AuGlobeLL(ll.Longitude(), ll.Latitude()), altMSL, exceptId, enclosureId);
	}

	// Like InEnclosure but 'exceptId' enclosure is ignored.
	int InEnclosureExcept(const AuGlobeLL& ll, double altMSL, const AuId& exceptId, AuId& enclosureId) const;

	void InvalidateGeoConstruction()
	{
		m_GeoConstruction = -1;
	}

	inline void InvalidateHengePresence()
	{
		m_HengePresence = -1;
	}

	// Returns 0=no, 1='ll' is within an irrigation area, 2='ll' is within a defoliated area.
	inline int IsIrrigated(const AuGlobeLLF& ll) const
	{
		return IsIrrigated(AuGlobeLL(ll.Longitude(), ll.Latitude()));
	}

	// Returns 0=no, 1='ll' is within an irrigation area, 2='ll' is within a defoliated area.
	int IsIrrigated(const AuGlobeLL& ll) const;

	// Not precise. Uses a rough site radius test.
	DaBd* FindBdContaining(const AuPoint& loc, FINDDEV filter) const;
	DaBd* FindBdContaining(INDUSTRYID iid, const AuPoint& loc, FINDDEV filter) const;
	DaBd* FindMilitaryContaining(const AuPoint& loc, FINDDEV filter) const;

	DaBd* FindBdContaining(const AuPoint& loc, FINDDEV filter, const DaBd& onPlatform) const;
	DaBd* FindBdContaining(INDUSTRYID iid, const AuPoint& loc, FINDDEV filter, const DaBd& onPlatform) const;
	DaBd* FindMilitaryContaining(const AuPoint& loc, FINDDEV filter, const DaBd& onPlatform) const;

	// 'siteGround' is a return parameter based on the site mesh that contains 'loc'.
	// 'hideGrass' is a return parameter based on the site mesh that contains 'loc'. It is used by the caller to determine the
	// sound effect of walking at the location.
	DaBd* FindBdContaining(const AuPoint& loc, FINDDEV filter, ON_GROUND& siteGround, int& hideGrass) const;

	// Not precise. Uses a rough site radius test.
	DaBd* FindBdContaining(const AuPoint& loc, double radius, FINDDEV filter) const;

	// Looks for existing buildings that contain a proposed structure 'report' at 'mBld'.
	DaBd* FindBdContaining(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter) const;

	// Looks for existing buildings that contain a proposed structure 'report' at 'mBld' on 'platform'.
	DaBd* FindBdContaining(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter, const DaBd& onPlatform) const;

	// Fills 'bds' with all buildings whose site contain 'loc'.
	void FindBdsContaining(const AuPoint& loc, FINDDEV filter, DaAtomList& bds) const;

	// Looks for existing building site that overlaps the site of a proposed structure 'report' at 'mBld'.
	// Returns the bd that is overlapped, if any.
	DaBd* FindBdSiteOverlap(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter) const;
	DaBd* FindBdSiteOverlap(const AuBdReport& report, const AuMatrix4x4& mBld, FINDDEV filter, const DaBd& onPlatform) const;

	virtual int GroundLevelEquatorDivisions() const = 0;

	virtual int SeaLevelEquatorDivisions() const = 0;

	// Animals when CanHaveFauna is true, one entry for each zone.
	virtual DePlanetFauna* LockFaunaForWrite() = 0;

	virtual void UnlockFauna() = 0;

	virtual bool FetchConstructionMaterials(DeConstructionProcess& cp, const DaBd* forBuilding);

	// elecSubQ - Output parameter. Quality of result when substituting electricity for sunlight. 0 prevents substitution.
	// waterSubQ - Output parameter. Quality of result when substituting water for hydrographics. 0 prevents substitution.
	virtual bool FetchManufacturingMaterials(DeManufacturingProcess &mp, const AuPoint& loc, bool isDark, INDUSTRYID iid,
			bool isUndersea, bool inEnclosure, quint8& elecSubQ, quint8& waterSubQ);

	const DaBd* FindClosestCity(const AuGlobeLLF& searchLL) const;

	const DaBd* FindLargestCity() const;

	const DaBd* FindNewestCity() const;

	const DaBd* FindOldestCity() const;

	const DaBd* FindSmallestCity() const;

	// Locate a troop to load onto a vehicle.
	DaNpc* FindTroopReadyToLeave(const DaVehicle& toLoadVehicle) const;

	void GeoConstructionEnable(const AuGlobeLLF& geoLL, bool enable);

	void GeoConstructionFetch(const AuGlobeLLF& geoLL);

	// Returns true if 'gear' changes.
	bool GeoConstructionGo(const AuGlobeLLF& geoLL, AuGear& gear);

	inline void GeologyChanged()
	{
		m_LLLock.lockForWrite();
		m_tmGeologyChange.Set();
		m_LLLock.unlock();
	}

	inline AuElapsedTimer GeologyChangeTime() const
	{
		m_LLLock.lockForRead();
		AuElapsedTimer v(m_tmGeologyChange);
		m_LLLock.unlock();
		return v;
	}

	// Finds the geological feature at 'geoLL' that is under construction. Gets its construction process into 'cp'.
	// Returns true on success, false if it is not found or if it is not under construction.
	bool GeoUnderConstruction(const AuGlobeLLF& geoLL, DeConstructionProcess& cp) const;

	inline AuGeologicalFeatures& LockGeologicalFeaturesForWrite()
	{
		m_LLLock.lockForWrite();
		return m_GeologicalFeatures;
	}

	inline const AuGeologicalFeatures& LockGeologicalFeatures() const
	{
		m_LLLock.lockForRead();
		return m_GeologicalFeatures;
	}

	inline void UnlockGeologicalFeatures() const
	{
		m_LLLock.unlock();
	}

	// Returns the base geological altitude of terrain at 'll'.
	// The altitude is not adjusted for city developments or geological features.
	virtual double GeologicalTerrainAltitudeMSL(const AuGlobeLL& ll) const = 0;

	// Returns the location on the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLLF& ll) const = 0;

	// Returns the location at a specific altitute above the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLLF& ll, float altAGL) const = 0;

	// Returns the location on the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLL& ll) const = 0;

	// Returns the location at a specific altitute above the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLL& ll, double altAGL) const = 0;

	// Returns the location on the surface of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLLF& ll) const = 0;

	// Returns the location at a specific altitute above the surface of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLLF& ll, float altAGL) const = 0;

	// Returns the location on the surface of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLL& ll) const = 0;

	// Returns the location at a specific altitute above the surface of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLL& ll, double altAGL) const = 0;

	AuFloraDNA GroundCoverDNA(const AuGlobeLLF& ll) const;

	// Returns true when 'loc' is inside a pressure tent.
	bool InsidePressureTent(const AuPoint& loc) const;

	void InvalidateEnclosureCache();

	// Call when something has changed that affects the appearance of the globe. e.g. City lights go on/off.
	virtual void InvalidateNearGlobeTexture() = 0;

	virtual bool IsControl(const DaAtom* sceneAtom) const;

	// Returns true if the build location at 'buildLL' and 'buildAltMSL' falls within 'radius' of hitting a zone where construction
	// is not permitted.
	virtual bool IsForbiddenZone(const AuGlobeLLF& buildLL, double buildAltMSL, double radius) const = 0;

	// Checks surface at 'landingLoc' within 'radius' for obstructions, ignoring 'ignoreAtom'.
	bool IsLandingAreaObstructed(const AuPoint& landingLoc, double radius, const DaAtom* ignoreAtom) const;

	// Tests a spherical area at 'loc' for a unit to spawn to insure it is not obstructed within 'radius'.
	// Returns true if the area is obstructed.
	bool IsSpawnAreaObstructed(const AuPoint& loc, double radius) const;

	bool IsSpawnAreaObstructed(const AuPoint& loc, quint32 designId, quint8 designQ, double roughRadius,
			DaVessel** replaceableShip) const;

	AuFloraDNA::ZONE LatitudeRegion(float latitude) const;

	// Calculates 'll' from 'loc', which is in the space of this world.
	// Returns false if 'loc' is outside the bounds of the world.
	virtual bool LLOfLoc(AuGlobeLL& ll, const AuPoint& loc) const = 0;

	// Calculates 'll' from 'loc', which is in the space of this world.
	// Returns false if 'loc' is outside the bounds of the world.
	virtual bool LLOfLoc(AuGlobeLLF& ll, const AuPoint& loc) const = 0;

	// Shape for AuGlobeLL linear surface calculations.
	virtual LLSHAPE LLShape() const = 0;

	// Returns true if this world has a village with a non-zero population.
	bool HasVillage() const;

	inline const AuVillage* LockVillage() const
	{
		m_VillageLock.lockForRead();
		return m_Village;
	}

	// This guy does his own locking/unlocking.
	void SetVillage(const AuVillage& village);

	inline void UnlockVillage() const
	{
		m_VillageLock.unlock();
	}

	AuGlobeLLF VillageLL() const;

	qint8 VillageReputation(const AuId& personId) const;

	void VillageResetReputation(const AuId& personId);

	void VillageSetContacted(const AuId& personId, qint8 initialReputation);

	// Location returned is at core level on a gas giant, sea level elsewhere.
	virtual AuPoint GlobeLocOfLL(const AuGlobeLL& ll) const = 0;
	virtual AuPoint GlobeLocOfLL(const AuGlobeLLF& ll) const = 0;

	// Location returned is at sea level.
	virtual AuPoint LocOfLL(const AuGlobeLL& ll) const = 0;
	virtual AuPoint LocOfLL(const AuGlobeLLF& ll) const = 0;

	// Location returned is at 'altitude' above/below sea level.
	virtual AuPoint LocOfLL(const AuGlobeLL& ll, double altitude) const = 0;
	virtual AuPoint LocOfLL(const AuGlobeLLF& ll, double altitude) const = 0;

	// Returns the positive latitude angle of the line between polar and temperate latitude zones.
	// Latitudes > PolarLatitude are polar.
	// Returns PIf/2 if the world has no polar region.
	float PolarLatitude() const;

	// Lists all possible commodities that a particular mine can extract.
	virtual void QueryMineResources(const DaBd* mine, QList<COMMODITYID>& rockCIDs) const = 0;

	// Queries for information about the terrain at a specific longitude and latitude.
	virtual void QueryTerrain(AuTerrainInfo& ter, const AuGlobeLL& ll) const = 0;

	inline void QueryTerrain(AuTerrainInfo& ter, const AuGlobeLLF& llf) const
	{
		AuGlobeLL ll(llf.Longitude(), llf.Latitude());
		QueryTerrain(ter, ll);
	}

	// Lists all possible commodities that a particular well can extract.
	virtual void QueryWellResources(const DaBd* well, QList<COMMODITYID>& wellCIDs) const = 0;

	// If none is found, AuGlobeLLF() is returned.
	// Under water locations are returned if they are in undersea cities.
	// Altitude is determined using TerrainAltitude, which considers/locks geological features.
	AuGlobeLLF RandomCitizenLLNear(const AuDice& dice, const AuGlobeLLF& nearLL, double minDist, double maxDist) const;

	// This function creates the m_LandingSites array when needed.
	// Returns the landing site that has the most daylight.
	AuPoint RandomLandingSite(const AuPoint& suggestion) const;

	// This function locates a random land location at least 'minAltMSL' altitude.
	// Altitude is based on TerrainAltitude, which considers/locks geological features.
	AuGlobeLLF RandomLandLL(const AuDice& dice, double minAltMSL) const;

	// If none is found, nearLL is returned.
	// Altitude is based on TerrainAltitude, which considers/locks geological features.
	AuGlobeLLF RandomLandLLNear(const AuDice& dice, const AuGlobeLLF& nearLL, double minAltMSL, double minDist, double maxDist) const;

	// 'radius' is the distance the random land location must be from overlapping any forbidden zones, see IsForbiddenZone.
	// Altitude is based on TerrainAltitude, which considers/locks geological features.
	AuGlobeLLF RandomLandLLNotForbidden(const AuDice& dice, double minAltMSL, double radius) const;

	// This function locates a random land location at least 'minAltMSL' altitude.
	// Altitude is based on GeologicalTerrainAltitude, which does not consider geological features.
	AuGlobeLLF RandomLandLocation(const AuDice& dice, double minAltMSL) const;

	// 'radius' is the distance the random land location must be from overlapping any forbidden zones, see IsForbiddenZone.
	// Altitude is based on GeologicalTerrainAltitude, which does not consider geological features.
	AuGlobeLLF RandomLandLocationNotForbidden(const AuDice& dice, double minAltMSL, double radius) const;

	virtual AuGlobeLLF RandomLL(const AuDice& dice) const = 0;

	// Called when a piece of food wants to decay that is in someone's scene in a city that has a rat infestation.
	// 'loc' is the location of the food.
	void RatEncounter(const AuPoint& loc, const AuSpecies& ratSpecies);

	// 'plantType' is 0=ground cover, 1=shrubber, 2=trees
	// 'floraZone' is 0=polar, 1=temperate, 2=tropical
	void RegenPlantDNA(int resourceZone, int plantType, int floraZone);

	void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& nearLoc,
			const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor);

	// Renders things on the world that use the Sc shader. e.g. claim flag, road pavement.
	virtual void RenderScWorld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const DaPerson* sceneAtom) = 0;

	// Renders things on the world that use the Sc shader. e.g. claim flag, road pavement.
	// Shader is already selected and configured.
	virtual void RenderSc_Shader(AuGLSLScBase* shader, const AuMatrix4x4& projection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, const DaPerson* sceneAtom);

	void RenderHengeFires(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	void RenderSmoke(const AuGlobeLLF& smokeLL, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
				const AuPoint& planetLoc, float size, const QPointF& wind, float ambientR, float ambientG, float ambientB);

	// Send a report to chain-of-command of all cities when the local ship's officer is available.
	void ReportOfficerAvailable();

	// 'mBld' is the orientation matrix of the proposed or existing building.
	// 'siteCen' is the ground level center point of the building site.
	// 'siteBoxRadius' is the square radius to search, oriented to the terrain using 'mBld'.
	// 'shrubs', 'rocks', 'trees' and 'waterCoverage' are return values.
	// 'maxICareAbout' is the number of rocks, shrubs and trees to count. We stop counting them after that for speed. The hidden
	//  state of those resources, per TerrainAltitudeMSLPave, is ignored if 'maxICareAbout' is 4.
	virtual void ResourceDistributionAt(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			unsigned int& rocks, unsigned int& shrubs, unsigned int& trees, float& waterCoverage,
			unsigned int maxICareAbout) const = 0;

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(float longitude) const = 0;

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(double longitude) const = 0;

	// Returns 0,0 if no landing site is found in the zone.
	virtual AuPoint ResourceZoneLandingSite(int zone) const = 0;

	virtual double RotationAngleOffset() const = 0;

	virtual void SceneRenderParameters(const DaAtom* sceneAtom, float& ambientR, float& ambientG, float& ambientB,
			float& fogDensity, float& fogR, float& fogG, float& fogB, bool viewIsTopDown, bool& sunsBlocked) const;

	virtual void SetRotationAngleOffset(double rotAngleOffset) = 0;

	virtual void GeologicalFeatureChanged(const AuGeologicalFeature* feat, AuGLModelMaker& mm) = 0;

	// Rounds a location to the nearest globe longitude latitude vertex.
	// Nearest vertex is the one that would result from tessellating the globe using CreateSurfaceTriangles, passing it 'eqdivs'.
	virtual AuGlobeLL RoundedGlobeLoc(const AuGlobeLL& locLL, int eqdivs) const = 0;

	// Rounds a location to the nearest globe longitude latitude vertex.
	// Nearest vertex is the one that would result from tessellating the globe using CreateSurfaceTriangles, passing it 'eqdivs'.
	virtual AuGlobeLLF RoundedGlobeLoc(const AuGlobeLLF& locLL, int eqdivs) const = 0;

	void SelectDefaultMineProcess(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			DeManufacturingProcess* proc, const AuKnownPatents& patents) const;

	void SelectDefaultWellProcess(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			DeManufacturingProcess* proc, const AuKnownPatents& patents) const;

	inline void SetCapt(const AuId& captId, const QString& captName)
	{
		m_LLLock.lockForWrite();
		m_CaptId = captId;
		m_CaptName = captName;
		m_LLLock.unlock();
	}

	// Sets the indigenous DNA of this world using 'person's DNA, if it has not already been set.
	// Returns true if the indigenous DNA of this world matches 'person'.
	bool SetClaimedBy(const DaPerson& person, const AuGlobeLLF& atLL);

	inline void SetClaim(quint32 empireId, const AuGlobeLLF& atLL, const AuId& personId, const QString& personName,
			const QDateTime& when)
	{
		m_LLLock.lockForWrite();
		m_ClaimAtLL = atLL;
		m_ClaimEmpire = empireId;
		m_ClaimPersonId = personId;
		m_ClaimPersonName = personName;
		m_ClaimTime = when;
		m_LLLock.unlock();
		SetModified();
	}

	// Releases the claim. Caller should verify its ok.
	inline void SetClaimQuit(const AuId& personId, const QString& personName)
	{
		m_LLLock.lockForWrite();
		m_ClaimEmpire = 0;
		m_ClaimPersonId = personId;
		m_ClaimPersonName = personName;
		m_ClaimTime.Set();
		m_LLLock.unlock();
		SetModified();
	}

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	// Returns true if 'loc' is blocked by any city shield generator that is currently active.
	bool SubatomShieldsBlockTransporters(quint32 empireId, const AuPoint& loc) const;

	AuFloraDNA ShrubDNA(const AuGlobeLLF& ll, COMMODITYID shrubCID) const;

	// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
	// 'sightedLoc' and 'sightedPart' are not changed if false is returned.
	virtual bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
			unsigned int& sightedPart, bool& sightedShield) const = 0;

	// Creates a nu worker at 'spawnLL' and equips it from the inventory of the 'bd' community.
	DaNpc* SpawnCitizenWorker(DaBd* bd, const AuGlobeLLF& spawnLL);

	// Spawns a spacecraft in orbit, if possible.
	// Returns:
	//   0 = ok
	//   1 = blocked by unanalyzed ship
	//   2 = Cannot spawn another station in orbit or replace an existing station.
	//   3 = Cannot spawn another nu ship in orbit or replace an existing nu ship.
	//   4 = Orbit spawn location could not be found.
	//   5 = No station at which to spawn.
	virtual int SpawnSc(const AuScReport& shipDesign, quint32 empireId, quint8 quality, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire, const AuId& cityOrBdId) = 0;

	// Spawns a spacecraft on the ground, if possible.
	// Returns:
	//   0 = ok
	//   1 = no slab found
	//   2 = no unobstructed slab found
	//   3 = spawn location not on the world
	int SpawnSc(DaBd* bd, const AuScReport& shipDesign, quint8 quality,
			const AuId& ownerId, const QString& ownerName, quint32 ownerEmpire);

	// Returns the commodity id of significant rock outcroppings at 'll'.
	// This is used for spotting and foraging. 'll' is rounded to the nearest actual rock grid location and that is the rock that
	// is returned, using RockAt.
	// Returns CID_Invalid if no significant rocks are present.
	virtual COMMODITYID SpotRockAt(const AuGlobeLL& ll, quint32 worldGenSeed) const = 0;

	// Returns the commodity id of significant shrubs at 'll'.
	// This is used for spotting and foraging. 'll' is rounded to the nearest actual shrub grid location and that is the shrub that
	// is returned, using ShrubAt.
	// 'aquaticPlant' is set true if the plant is such.
	// Returns CID_Invalid if no significant shrubs are present.
	virtual COMMODITYID SpotShrubAt(const AuGlobeLL& ll, quint32 worldGenSeed, bool& aquaticPlant) const = 0;

	// Returns the commodity id of significant trees outcroppings at 'll'.
	// This is used for spotting and foraging. 'll' is rounded to the nearest actual tree grid location and that is the tree that
	// is returned, using TreeAt.
	// Returns CID_Invalid if no significant trees are present.
	virtual COMMODITYID SpotTreeAt(const AuGlobeLL& ll, quint32 worldGenSeed) const = 0;

	// Returns the altitude of the surface at ll relative to MSL.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual double SurfaceAltitudeMSL(const AuGlobeLL& ll) const = 0;

	// Returns the altitude of the surface at ll relative to MSL.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual double SurfaceAltitudeMSL(const AuGlobeLLF& ll) const = 0;

	// Only valid on the client.
	virtual const QImage* SurfaceImage() const = 0;

	// Returns height of the surface image in feet.
	virtual double SurfaceImageHeight() const = 0;

	void SurrenderClaim(quint32 empireId);

	virtual QString TargetDesc(const AuPoint& spottedLocation, quint32 spotterEmpire, AuId& targetId,
			POLITICAL_STANCE& stanceToTarget) const;

	// Returns the altitude of the terrain at ll relative to MSL.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	double TerrainAltitudeMSL(const AuGlobeLL& ll) const;

	// Returns the altitude of the terrain at ll relative to MSL.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	double TerrainAltitudeMSL(const AuGlobeLLF& ll) const
	{
		return TerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
	}

	// Returns the altitude of the terrain at ll relative to MSL.
	// Sets 'hide' bools to indicate affect on natural resources by developments at 'll'.
	// Sets 'pavementType' to the texture type that covers the ground at the location. WORLDTEX_Count is returned if no pavement.
	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	// 'farmShrub' or 'orchardTree' will be set accordingly if the location falls within the area of a farm or orchard that is
	//   growing something, otherwise set to CID_Invalid. When they are on platforms, 'farmShrubMSL' and 'orchardTreeMSL' are set
	//   to the platform height. Otherwise, they are set to 0, which is not a valid MSL altitude for the deck of a platform.
	double TerrainAltitudeMSLPave(const AuGlobeLL& ll, bool& hideRocks, int& hideGrass, bool& hideShrubs, bool& hideTrees,
			WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, double& farmShrubMSL, COMMODITYID& orchardTree,
			double& orchardTreeMSL) const;

	virtual TER_ELEV_TYPE TerrainElevationType(float altitudeMSL) const;

	// Returns a vector that is perpendicular to the terrain at 'll'.
	AuPoint TerrainNormal(const AuGlobeLL& ll) const
	{
		return TerrainNormal(GetTerrainLocation(ll));
	}

protected:
	// 'loc' MUST BE a globe surface location as returned by GetTerrainLocation.
	// Returns a vector that is perpendicular to the terrain at 'loc'.
	AuPoint TerrainNormal(const AuPoint& loc) const;

	// 'loc' MUST BE a globe surface location as returned by GetTerrainLocation.
	// Returns a vector that is perpendicular to the terrain at 'loc', compared to locations at 'radius' distance away.
	AuPoint TerrainNormal(const AuPoint& loc, double radius) const;

public:
	void SendBdFacilities();

	// Returns the labor needed to prepare a site for 'iid' from 'roadStart' to 'roadEnd'.
	// 'paveMaterial' is set to the number of units of materials needed to pave the area.
	quint32 SitePrepLabor(INDUSTRYID iid, const AuPoint& roadStart, const AuPoint& roadEnd, double roadWidth,
			quint32& paveMaterial) const;

	// Tests a location on this globe to see if its ok to build a building or start a road there.
	// 'bld' is the building design report for buildings, 0 for roads
	// 'buildMaterial' is the primary construction material.
	// 'fBuildLoc' is the build location.
	// 'roadWid2' is 1/2 the width of roads and landscapers. n/a when 'bld' is not null.
	// 'buildRotZ' is the Z rotation of the building, n/a for roads.
	// 'onPlatform' is not-zero when the build location is on top of that platform.
	// 'exDev' is an output parameter when an existing development is responsible for the return result.
	// 'startLoc' is non-zero when testing the second end of a road segment. It is assumed to have already passed this test.
	// 'cityName' is set to the name of the jurisdiction that contains the proposed bd. May be blank; will be blank for military.
	// See comments in the code for return values.
	int TestBdLocation(INDUSTRYID lIID, quint32 empireOfBuilder, const AuBdReport* bld, COMMODITYID buildMaterial,
			const AuPoint& buildLoc, double roadWid2, float buildRotZ, const DaBd* onPlatform, DaBd*& exDev,
			const AuPoint* startLoc, QString& cityName) const;
	int TestBdPreviewLocation(const AuBdReport* bld, const AuPoint& buildLoc, float buildRotZ, DaBd*& exDev) const;

	// Returns a pixmap name that can be used for a cursor, based on the return code from TestBdLocation.
	static QString TestBdResultPixmap(int testBdResult);

	// Returns brief text based on the return code from TestBdLocation.
	static QString TestBdResultText(int testBdResult);

	// Test for standing liquid on the building site. Returns true if present.
	virtual bool TestForLiquid(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius) const = 0;

	// Test for visible rocks on the building site.
	virtual bool TestRockResource(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			COMMODITYID idRes) const = 0;

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	AuFloraDNA TreeDNA(const AuGlobeLLF& ll, COMMODITYID treeCID) const;

	// Returns an altitude above which there are no plants. This is the target altitude when making terrain for the tree line of
	// mountains. It is also used in the CanHaveFlora test.
	// The altitude returned does not depend on the presense of trees or plants. It is solely based on the weather layer altitudes.
	virtual float TreeLineMSL() const = 0;

	// Returns the positive latitude angle of the line between tropical and temperate latitude zones.
	// Latitudes <= TropicalLatitude are tropical.
	// Returns PIf/2 if the entire world is tropical.
	float TropicalLatitude() const;

	// Rebuilds the Enclosures data for this world based on current bds.
	// Returns true if the number of Enclosures changes.
	virtual bool UpdateEnclosureCache() const;

	// Rebuilds the irrigation data for this world based on current geo landscapers.
	void UpdateIrrigationCache();

	// Returns the precipitation conditions present.
	// 'll' is the location relative to this world.
	// 'altitudeMSL' is the MSL altitude of the desired weather reading.
	// 'relHumidity' is an output parameter set to the relative humidity.
	// Relative humidity is the ratio of vapor pressure to saturation pressure. <=1 is not precipitating. >1 is precipitating.
	// 0-1 = No precip
	// 1-1.25 = Light precip
	// 1.25-1.5 = Moderate precip
	// 1.5-1.75 = Heavy precip
	// 1.75+ = Very Heavy precip
	// 'oat' is an output parameter indicating outside air temperature in celsius.
	// 'bar' is an output parameter indicating outside air pressure in bars.
	virtual void WeatherAt(const AuGlobeLLF& ll, float altitudeMSL, float& relHumidity, float& oat, float& bar) const = 0;

	// 'longLat' is longitude latitude angles, in radians
	virtual QPointF WindAt(const AuGlobeLLF& longLat) const = 0;

	// Returns the undeveloped altitude of terrain at 'll'.
	// The altitude is adjusted for geological features.
	// The altitude is not adjusted for developments that might exist there.
	virtual double UndevelopedVirginTerrainAltitudeMSL(const AuGlobeLL& ll) const = 0;

	// Returns the undeveloped altitude of terrain at 'll'.
	// The altitude is adjusted for geological features.
	// The altitude is adjusted for geo roads and landscaping but not other developments that might exist there.
	// 'newest' is set to the newest time the terrain was modified by any man-made geological feature, if any.
	virtual double VirginTerrainAltitudeMSL(const AuGlobeLL& ll, WORLDTEX_TYPE& pavementType, AuTime& newest) const = 0;

	// Returns the undeveloped altitude of terrain at 'll'.
	// The altitude is adjusted for geological features and ringworld structural elements.
	// The altitude is adjusted for geo roads and landscaping but not other developments that might exist there.
	// Sets 'hide' bools to indicate affect on natural resources by developments at 'll'.
	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	// Sets 'pavementType' to the texture type that covers the ground at the location. WORLDTEX_Count is returned if no pavement.
	// Pavement exists in some areas of the ringworld structural elements (i.e. control center, energy cones).
	// 'newest' is set to the newest time the terrain was modified by any man-made geological feature, if any.
	virtual double VirginTerrainAltitudeMSL(const AuGlobeLL& ll, bool& hideRocks, int& hideGrass, bool& hideShrubs, bool& hideTrees,
			WORLDTEX_TYPE& pavementType, AuTime& newest) const = 0;

	// 'numToRelease' of 0xffff results in all workers at 'atBdId' being released from every building.
	void WorkersReleased(const AuId& atBdId, quint16 numToRelease);

protected:
	// Returns an iterator to a map location rounded near 'll'.
	// Always returns a valid iterator which may point to an empty list.
	// Builds data in mutable m_BuildingMap when needed.
	QMap<AuGlobeLLF, AuIdList>::const_iterator BuildingMapLocation(const AuGlobeLLF& ll) const;

	// Updates m_BdBits
	void CalcBdPresence() const;

	// Updates m_BdBits
	void CalcBasePresence() const;

	// Updates m_BdBits
	void CalcHQPresence() const;

	AuGlobeLLF ChooseEncounterLocation(const AuGlobeLLF& nearLL) const;

	AuGlobeLLF ChooseVillageLocation(const AuGlobeLLF& nearLL) const;

	// This was cobbled from a shader.
	// 'P' is the test location, relative to this globe.
	// 'sunLoc' is the sun location, relative to this globe.
	// 'sunAttenuation' is the attenuation of the sun's light
	// 'sunRadius' is the radius of the solar disc, in feet
	float ComputeSunLight(const AuPoint& P, const AuPoint& sunLoc, float sunAttenuation, float sunRadius) const;

	// This was cobbled from a shader.
	// 'P' is the test location, relative to this globe.
	// 'sunLoc' is the sun location, relative to this globe.
	// 'sunAttenuation' is the attenuation of the sun's light
	// 'sunRadius' is the radius of the solar disc, in feet
	// 'shadowCount' is the number of ringworld shadow panels around the sun.
	// 'shadowAngle' is the current angle of rotation around the sun of the ringworld shadow panels.
	// 'shadowXVec' is the plane X vector of the sun.
	// 'shadowYVec' is the plane Y vector of the sun.
	float ComputeSunLight(const AuPoint& P, const AuPoint& sunLoc, float sunAttenuation, float sunRadius, int shadowCount,
			float shadowAngle, const AuPoint& shadowXVec, const AuPoint& shadowYVec) const;

	void CreateClaimVAOs(AuGLEffects& re);

	// Creates a new indigenous native atom with transform 'm'.
	DaNpc* CreateIndigenous(NPC_ROLE role, GROWTHSTAGE stage, const AuSpecies& species, const AuMatrix4x4& m,
			const AuZoneResource& res);

	// Creates a new item atom with transform 'm'.
	DaItem* CreateItem(const AuItem& item, const AuMatrix4x4& m);

	// 'villagers' contains existing villager atoms that are not dying or dead.
	DaNpc* CreateOrMoveIndigenous(NPC_ROLE role, GROWTHSTAGE, const AuSpecies& species, const AuMatrix4x4& m,
			const AuZoneResource& res, DaAtomList& villagers);

	void DeleteExcessBombCraters(unsigned int maxCraters);

	// Calls FenceAdjust on all buildings.
	void FenceAdjust();

	void InvalidateTerrainInfoSprites() const;

	void RenderTerrainInfo(const AuMatrix4x4& model, AuAtomRenderInfo& ari, const DaPerson* scenePerson);

	// Default sprite texture coordinate behavior flips the texture in Y.
	// 'flipSprite2' is true when this behavior must be inverted for 'texture2', used for displaying flags.
	void RenderTerrainInfoSprite(AuGLEffects& re, const AuPoint& txpt, GLuint texture1, int texSize1, GLuint texture2, int texSize2,
			GLuint texQ, double unitsPerPixel, bool sprite2Flip=false);

	void ResetWeaponRadarRanges();

	void UpdateBuildingOverlaySprites(const AuPoint& worldLoc, quint32 sceneEmpireId);

	void UpdateRoadOverlaySprites(const AuPoint& worldLoc);

	// Recreates m_RoadMap.
	void UpdateRoadMap();

	// FindTerrainAtPixel cache
	bool CachedFindTerrainAtPixel(AuPoint& sightedLoc) const;
	bool IsCachedFindTerrainAtPixel(const AuPoint& eyeLoc, const AuPoint& sightedLoc) const;
	void SetCachedFindTerrainAtPixel(const AuPoint& eyeLoc, const AuPoint& sightLook, bool detailsOk,
			const AuPoint& sightedLoc) const;

	// SightTarget cache
	int CachedSightTarget(bool& detailsOk, AuPoint& sightedLoc) const;
	bool IsCachedSightTarget(const AuPoint& eyeLoc, const AuPoint& sightedLoc) const;
	void SetCachedSightTarget(const AuPoint& eyeLoc, const AuPoint& sightLook, int isect, bool detailsOk,
			const AuPoint& sightedLoc) const;

	virtual bool SightSeaSurface(const AuPoint& loc1, const AuMatrix4x4& model, AuPoint& hitLoc) const = 0;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // DaWorldLL_H
