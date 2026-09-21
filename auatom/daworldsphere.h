//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldsphere.h,v 1.35 2022/02/19 17:35:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaWorldSphere_H
#define DaWorldSphere_H

#include "daworldll.h"

class DaWorldRing;

class AUATOM_DLL DaWorldSphere : public DaWorldLL
{
	mutable QReadWriteLock				m_SphereLock;
	mutable double						m_AxialTilt;			// Gen'd, not stored.
	double								m_RotationAngleOffset;	// Used by designer preview.

public:	// Constructors
	DaWorldSphere();
	virtual ~DaWorldSphere();

protected:
	DaWorldSphere(const DaWorldSphere& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	virtual void ActivateHengeContaining(const AuPoint& containingLoc);

	// Returns the amount of altitude change in feet per whole terrain data altitude increment.
	virtual double AltitudeIncrement() const
	{
		return SeaLevelRadius()/100.;	// Based on size of this world.
	}

	double AxialTilt() const;

	virtual void CreateFlatGlobeVAOTriangle(QList<AuGlobeTriAltVeg>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2) const = 0;

	// 'moons' is the total number of satellites.
	// 'firstOrbit' is the orbit radius of the first available orbit.
	// 'hasTitan' is true if one satellite is a titan.
	void CreateMoons(int moons, double firstOrbit, bool hasTitan, const AuDice& dice, HABITABILITY hab, ORBITZONE orbitZone);
	void CreateMoonsForScan(int moons, double firstOrbit, bool hasTitan, const AuDice& dice, HABITABILITY hab, ORBITZONE orbitZone);

	// Like CreateSurfaceTriangles except the poles have more triangles for making a flat projection of the surface.
	virtual void CreateSurfaceTrianglesFlat(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const = 0;

	static inline int EquatorDivisions100km(quint16 radius100km) 
	{
		Q_ASSERT(radius100km >= 5);
		Q_ASSERT(radius100km <= 128);
		// Equator divisions can be calculated from circumference. This formula gives us the basis for calculating the same size
		// divisions on a DaWorldRingSection.
		//
		//   EqDiv = C / PI
		//
		// Formulae and units of measure.
		//   R: Radius in 100km units. See Radius100km().
		//   C: Circumference in 100km units.
		//   C = PI * R * 2
		//
		// Boiling it down...
		//
		//   EqDiv = C / PI
		//   EqDiv = PI * R * 2 / PI
		//   EqDiv = R * 2
		return 2 * radius100km;
	}

	virtual DaWorldRing* FindRing(const AuPoint& searchLoc) const;

	// Returns a 2D coordinate for the longitude latitude location appropriate for creating the surface map of this world.
	// Return coordinate is in the range -1,-1 to 1,1.
	inline AuPoint GetFlatGlobeSurfaceLocation(const AuGlobeLL& ll) const
	{
		double x = (ll.Longitude() / (PI*2)) * 2;
		double y = (ll.Latitude() / PI) * 2;
		return AuPoint(x,y);
	}

	// Returns the location on the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLLF& ll) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + SurfaceAltitudeMSL(ll));
	}

	virtual AuPoint GetSurfaceLocation(const AuGlobeLLF& ll, float altAGL) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + SurfaceAltitudeMSL(ll) + altAGL);
	}

	// Returns the location on the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLL& ll) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + SurfaceAltitudeMSL(ll));
	}

	virtual AuPoint GetSurfaceLocation(const AuGlobeLL& ll, double altAGL) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + SurfaceAltitudeMSL(ll) + altAGL);
	}

	// Returns the location on the terrain of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLLF& ll) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + TerrainAltitudeMSL(ll));
	}

	virtual AuPoint GetTerrainLocation(const AuGlobeLLF& ll, float altAGL) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + TerrainAltitudeMSL(ll) + altAGL);
	}

	// Returns the location on the terrain of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLL& ll) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + TerrainAltitudeMSL(ll));
	}

	virtual AuPoint GetTerrainLocation(const AuGlobeLL& ll, double altAGL) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + TerrainAltitudeMSL(ll) + altAGL);
	}

	// Returns true if the build location at 'buildLL' and 'buildAltMSL' falls within 'radius' of hitting a zone where construction
	// is not permitted.
	virtual bool IsForbiddenZone(const AuGlobeLLF& /*buildLL*/, double /*buildAltMSL*/, double /*radius*/) const
	{
		// No places are off limits on a globe.
		return false;
	}

	// Calculates 'll' from 'loc', which is in the space of this world.
	// Returns false if 'loc' is outside the bounds of the world.
	virtual bool LLOfLoc(AuGlobeLL& ll, const AuPoint& loc) const
	{
		ll.SetFromGlobeLocation(loc);
		return true;
	}

	// Calculates 'll' from 'loc', which is in the space of this world.
	// Returns false if 'loc' is outside the bounds of the world.
	virtual bool LLOfLoc(AuGlobeLLF& ll, const AuPoint& loc) const
	{
		ll.SetFromGlobeLocation(loc);
		return true;
	}

	// Shape for AuGlobeLL linear surface calculations.
	virtual LLSHAPE LLShape() const
	{
		return LL_Globe;
	}

	virtual AuPoint GlobeLocOfLL(const AuGlobeLL& ll) const
	{
		return ll.GlobeLocation(GlobeRadius());
	}

	virtual AuPoint GlobeLocOfLL(const AuGlobeLLF& ll) const
	{
		return ll.GlobeLocation(GlobeRadius());
	}

	// Location returned is at sea level.
	virtual AuPoint LocOfLL(const AuGlobeLL& ll) const
	{
		return ll.GlobeLocation(SeaLevelRadius());
	}

	// Location returned is at sea level.
	virtual AuPoint LocOfLL(const AuGlobeLLF& ll) const
	{
		return ll.GlobeLocation(SeaLevelRadius());
	}

	// Location returned is at 'altitude' above/below sea level.
	virtual AuPoint LocOfLL(const AuGlobeLL& ll, double altitude) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + altitude);
	}

	// Location returned is at 'altitude' above/below sea level.
	virtual AuPoint LocOfLL(const AuGlobeLLF& ll, double altitude) const
	{
		return ll.GlobeLocation(SeaLevelRadius() + altitude);
	}

	virtual quint64 MemoryUsage() const;

	// Returns the radius in 100s of km.
	virtual quint16 Radius100km() const = 0;

	virtual AuGlobeLLF RandomLL(const AuDice& dice) const;

	virtual double RotationAngleOffset() const
	{
		m_SphereLock.lockForRead();
		double v = m_RotationAngleOffset;
		m_SphereLock.unlock();
		return v;
	}

	// Typical worlds range from .1 to 3 times normal speed.
	virtual double RotationSpeedFactor() const
	{
		// This produces 50:50 results above or below 1.
		if(Id().GetID()&1)
			return 1. + 2. * (Id().GetID() % 1001) / 1000.;		// Result range is 1 to 3
		else
			return .1 + .9 * (Id().GetID() % 1001) / 1000.;		// Result range is .1 to 1
	}

	// Rounds a location to the nearest globe longitude latitude vertex.
	// Nearest vertex is the one that would result from tessellating the globe using CreateSurfaceTriangles, passing it 'eqdivs'.
	virtual AuGlobeLL RoundedGlobeLoc(const AuGlobeLL& locLL, int eqdivs) const;

	// Rounds a location to the nearest globe longitude latitude vertex.
	// Nearest vertex is the one that would result from tessellating the globe using CreateSurfaceTriangles, passing it 'eqdivs'.
	virtual AuGlobeLLF RoundedGlobeLoc(const AuGlobeLLF& locLL, int eqdivs) const;

	// Returns the sea level radius in feet.
	virtual double SeaLevelRadius() const = 0;

	virtual void SetRotationAngleOffset(double rotAngleOffset)
	{
		m_SphereLock.lockForWrite();
		m_RotationAngleOffset = rotAngleOffset;
		m_SphereLock.unlock();
	}

	virtual bool SightSeaSurface(const AuPoint& loc1, const AuMatrix4x4& model, AuPoint& hitLoc) const;

	// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
	// 'sightedLoc' and 'sightedPart' are not changed if false is returned.
	virtual bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
			unsigned int& sightedPart, bool& sightedShield) const;

	// Spawns a spacecraft in orbit, if possible.
	virtual int SpawnSc(const AuScReport& shipDesign, quint32 empireId, quint8 quality, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire, const AuId& cityOrBdId);

	// Returns the altitude of the surface at ll relative to MSL.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual double SurfaceAltitudeMSL(const AuGlobeLL& ll) const;

	// Returns the altitude of the surface at ll relative to MSL.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual double SurfaceAltitudeMSL(const AuGlobeLLF& ll) const
	{
		return SurfaceAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
	}

	virtual bool TestRockResource(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			COMMODITYID idRes) const;

	// Returns the volume of a sphere in cubic feet.
	// V = 4/3 * PI * r^3
	static inline double Volume100km(quint16 radius100km)
	{
		double radius = ::Radius100km(radius100km);
		return 4./3. * PI * radius * radius * radius;
	}

	virtual quint32 WorldPopulationMax() const;

protected:
	double CalcAngleOfRotation() const;

	double CalcAngleOfRotation(qint64 msecsSinceEpoch) const;
};

#endif // DaWorldSphere_H
