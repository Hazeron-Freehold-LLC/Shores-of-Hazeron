//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augeologicalfeature.h,v 1.54 2022/04/15 20:45:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGeologicalFeature_H
#define AuGeologicalFeature_H

#include "auatom.h"
#include "auglobell.h"
#include "auid.h"
#include "autime.h"
#include "dbindustry.h"
#include <QList>

class AuDice;
class AuGear;
class AuGeologicalFeatures;
class AuGLEffects;
class AuGLSLScBase;
class AuGLVAO;
class AuRoadMap;
class AuScFaceList;
class DeConstructionProcess;
class DaWorldLL;

class AUATOM_DLL AuGeologicalFeature
{
protected:
	AuGlobeLLF							m_LL;					// Location of this feature.

public: // Constructors
	AuGeologicalFeature(const AuGlobeLLF& ll);
	AuGeologicalFeature();
	virtual ~AuGeologicalFeature();
	AuGeologicalFeature(const AuGeologicalFeature& c);

public:	// Operators
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
	// Sets 'hide' bools to indicate affect on natural resources at 'll'.
	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	// 'pavement' is set if the feature cares, otherwise it is left untouched.
	// 'onRoadOrGrader' is a count of roads or graders influencing 'll', for averaging their effects.
	// 'newest' is set to the create time of the newest geological feature that actually modifies the altitude. Most features
	// return a null time. Only features that can be man made will report a non-null create time.
	// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const = 0;

	// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
	// 'onRoadOrGrader' is a count of roads or graders influencing 'll', for averaging their effects.
	// 'newest' is set to the create time of the newest geological feature that actually modifies the altitude. Most features
	// return a null time. Only features that can be man made will report a non-null create time.
	// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const = 0;

	// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
	// Ignores anything that can be man made, such as roads, landscapers and bomb craters.
	// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const = 0;

	virtual float AlignmentRot(const DaWorldLL& llworld, const AuGlobeLLF& alignLL) const
	{
		Q_UNUSED(llworld);
		Q_UNUSED(alignLL);
		return 0;
	}

	// Returns a duplicate of this situation, allocated on the heap.
	virtual AuGeologicalFeature* Clone() const = 0;

	virtual bool ConstructionEnable(bool enable)
	{
		Q_UNUSED(enable);
		return false;
	}

	virtual bool ConstructionFetch(const DaWorldLL& llworld)
	{
		Q_UNUSED(llworld);
		return false;
	}

	// Returns bit mask. All 0 = did nothing, else b0=under construction, b1=construction complete, b2=gear changed.
	virtual int ConstructionGo(AuGear& gear)
	{
		Q_UNUSED(gear);
		return 0;	// Did nothing. Not under construction.
	}

	// Gets my construction process into 'cp' and returns true if I had one.
	virtual bool ConstructionProcess(DeConstructionProcess& cp) const
	{
		Q_UNUSED(cp);
		return false;
	}

	static AuGeologicalFeature* Create(quint8 type);

	virtual void CreateRoadMap(const DaWorldLL& world, AuRoadMap& roadMap) const
	{
		Q_UNUSED(world);
		Q_UNUSED(roadMap);
	}

	virtual void DeleteDeviceObjects(AuGLEffects& re)
	{
		Q_UNUSED(re);
	}

	virtual INDUSTRYID GeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
	{
		Q_UNUSED(llworld);
		Q_UNUSED(ll);
		Q_UNUSED(radius);
		return IID_None;
	}

	virtual ON_GROUND Ground() const
	{
		return OG_None;	// OG_None says the geo does not override natural terrain ground.
	}

	virtual bool HasPavementModel() const
	{
		return false;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const = 0;

	virtual bool IsBombCrater() const
	{
		return false;
	}

	// Returns true if under construction and npc worker is enabled.
	virtual bool IsConstructionWorkerEnabled() const
	{
		return false;
	}

	virtual bool IsDefoliant() const
	{
		return false;
	}

	// Most geological features cannot be deleted by the user, using a bulldozer.
	virtual bool IsDeletable() const
	{
		return false;
	}

	// Most geological features cannot be deleted by the user, using a bulldozer.
	virtual bool IsDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
	{
		Q_UNUSED(llworld);
		Q_UNUSED(ll);
		Q_UNUSED(radius);
		return false;
	}

	virtual bool IsDeletableCraterContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
	{
		Q_UNUSED(llworld);
		Q_UNUSED(ll);
		Q_UNUSED(radius);
		return false;
	}

	// Holy places are revealed by village elders.
	virtual bool IsHoly() const
	{
		return false;
	}

	virtual bool IsIrrigation() const
	{
		return false;
	}

	// Returns true for things whose construction can be changed in place. e.g. pave a dirt road.
	virtual bool IsReconstructable() const
	{
		return false;
	}

	// Returns true for roads that add anything to the road map in CreateRoadMap.
	virtual bool IsRoad() const
	{
		return false;
	}

	virtual bool IsSlab() const
	{
		return false;
	}

	// Some features emit smoke.
	virtual bool IsSmokey() const
	{
		return false;
	}

	virtual bool IsTransporterTarget() const
	{
		return true;
	}

	virtual bool IsUnderConstruction() const
	{
		return false;
	}

	inline const AuGlobeLLF& LL() const
	{
		return m_LL;
	}

	virtual double LLMSL() const
	{
		return 0;
	}

	virtual const AuGlobeLLF& LL2() const
	{
		return m_LL;
	}

	virtual double LL2MSL() const
	{
		return 0;
	}

	virtual QColor MapColor() const
	{
		return QColor();
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const = 0;

	virtual void PaintOnSurfaceMap(const DaWorldLL& world, QPainter& p, unsigned int width, unsigned int height) const
	{
		Q_UNUSED(world);
		Q_UNUSED(p);
		Q_UNUSED(width);
		Q_UNUSED(height);
	}

	virtual float Radius() const
	{
		return 0;
	}

	// Base class sets m_LL to a random location.
	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual void Render_Shader(AuGLSLScBase* shader, const DaWorldLL& world, const AuMatrix4x4& projection,
			const AuMatrix4x4& model, AuGLEffects& re)
	{
		Q_UNUSED(shader);
		Q_UNUSED(world);
		Q_UNUSED(projection);
		Q_UNUSED(model);
		Q_UNUSED(re);
	}

	virtual void RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
	{
		Q_UNUSED(world);
		Q_UNUSED(projection);
		Q_UNUSED(model);
		Q_UNUSED(re);
		Q_UNUSED(nearLoc);
		Q_UNUSED(hitRoad);
		Q_UNUSED(hitColor);
		Q_UNUSED(roadColor);
	}

	// Shader has already been configured.
	virtual void RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
			const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
	{
		Q_UNUSED(world);
		Q_UNUSED(re);
		Q_UNUSED(nearLoc);
		Q_UNUSED(hitRoad);
		Q_UNUSED(hitColor);
		Q_UNUSED(roadColor);
	}

	virtual void RenderOnSurfaceTexture(const DaWorldLL& world, AuGLEffects& re, float radiansPerFootX,
			float radiansPerFootY) const
	{
		Q_UNUSED(world);
		Q_UNUSED(re);
		Q_UNUSED(radiansPerFootX);
		Q_UNUSED(radiansPerFootY);
	}

	virtual double SmokeRadius() const
	{
		return 0;
	}

	// Tests an existing geological feature for fragmentation by a bomb crater at 'll with 'radius'.
	// Fragments are added to 'fragments'.
	// Roads can be broken into parts his way.
	// Returns true if this feature is to be replaced by the results.
	virtual bool TestBombCraterHit(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
	{
		Q_UNUSED(llworld);
		Q_UNUSED(ll);
		Q_UNUSED(radius);
		return false;
	}

	// 'atLL' is either LL or LL2, for features that care.
	// 'hliq' is the type of surface liquid present on my world: none, water, lava, or supercooled liquid.
	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const
	{
		Q_UNUSED(hliq);
		return QString("");
	}

	// Returns a number that is unique to each derived class.
	virtual quint8 Type() const = 0;

	// Returns an unpaved equivalent to a paved feature that is getting its pavement blown off.
	virtual AuGeologicalFeature* UnpavedEquivalent() const
	{
		return 0;
	}

	virtual float Width() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuGeologicalFeature& geo)
{
	geo >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGeologicalFeature& geo)
{
	geo << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuGeologicalFeatures : public QList<AuGeologicalFeature*>
{
public:	// Operators
	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

public:
	// Affects roads that are overlapped by the new bomb crater.
	void AddBombCrater(const DaWorldLL& llworld, float craterRadius, float debrisHeight, float bottomMSL, const AuGlobeLLF& ll);

	// Adjusts 'altMSL' at 'll' on 'llworld' based on geological features.
	// Sets 'hide' bools to indicate affect on natural resources at 'll'.
	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	// 'pavement' is set if the feature cares, otherwise it is left untouched.
	// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
	double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, AuTime& newest) const;

	// Adjusts 'altMSL' at 'll' on 'llworld' based on geological features.
	// 'pavement' is set if the feature cares, otherwise it is left untouched.
	// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
	double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			AuTime& newest) const;

	// Adjusts 'altMSL' at 'll' on 'llworld' based on geological features.
	// Ignores anything that can be man made, such as roads, landscapers and bomb craters.
	// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
	double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	// Returns 99.f if none is found.
	// 'hitLL' is set to the LL of the geo that is found.
	float AlignmentRot(const DaWorldLL& llworld, const AuGlobeLLF& alignLL, double searchRadius, AuGlobeLLF& hitLL) const;

	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	bool ConstructionEnable(const AuGlobeLLF& ll, bool enable);

	// Fetches to every geo under construction.
	void ConstructionFetch(const DaWorldLL& world);

	// Fetches to the geo at 'll'.
	bool ConstructionFetch(const AuGlobeLLF& ll, const DaWorldLL& world);

	// Applies construction labor to all things under construction.
	// Returns bit mask. All 0 = did nothing, else b0=under construction, b1=construction complete.
	int ConstructionGo();

	// Returns bit mask. All 0 = did nothing, else b0=under construction, b1=construction complete, b2=gear changed.
	int ConstructionGo(const AuGlobeLLF& ll, AuGear& gear);

	unsigned int CountBombCraters() const;

	void CraterPavement(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius);

	void CreateRoadMap(const DaWorldLL& world, AuRoadMap& roadMap) const;

	// Deletes IsDeletable feature whose LL() exactly matches 'geoLL'.
	bool Delete(const AuGlobeLLF& geoLL);

	// Removes all IsDeletable features. Called when a pirate claims a world, to remove old roads, landscaping, bomb craters, etc.
	// as a cleanup service for me. ;^)
	// Returns true if any features are deleted.
	bool DeleteDeletableFeatures();

	bool DeleteGeo(const AuGeologicalFeature* geo);

	// Finds any feature whose LL() exactly matches 'geoLL'.
	const AuGeologicalFeature* Find(const AuGlobeLLF& geoLL) const;

	// Finds IsDeletable feature whose LL() exactly matches 'geoLL'.
	const AuGeologicalFeature* FindDeletable(const AuGlobeLLF& geoLL) const;

	const AuGeologicalFeature* FindDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	AuGeologicalFeature* FindDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius);

	void FindDeletableCratersContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius,
			QList<const AuGeologicalFeature*>& found) const;

	void FindDeletablesContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius,
			QList<const AuGeologicalFeature*>& found) const;

	void FindDeletablesContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius,
			QList<AuGeologicalFeature*>& found);

	// Finds IsReconstructable feature whose LL() exactly matches 'geoLL'.
	const AuGeologicalFeature* FindReconstructable(const AuGlobeLLF& geoLL) const;

	// Returns industry id of any feature containing 'll' that is under construction.
	// Returns industry id of any road containing 'll' that is completed.
	// 'geoLL' is set to the LL of the geo on return.
	INDUSTRYID FindGeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius, AuGlobeLLF& geoLL) const;

	bool FindGeoUnderConstruction(const AuGlobeLLF& geoLL, DeConstructionProcess& cp) const;

	// Returns the prevalent ground at a geo, for sound effects and ground contact movement friction.
	// Returns OG_None if the geo does not override natural terrain ground, or if 'geoLL' is not found.
	ON_GROUND Ground(const AuGlobeLLF& geoLL) const;

	// Returns true if any of my features has a pavement model. i.e. roads.
	bool HasPavementModel() const;

	quint64 MemoryUsage() const;

	void PaintOnSurfaceMap(const DaWorldLL& world, QPainter& p, unsigned int width, unsigned int height) const;

	void Render_Shader(AuGLSLScBase* shader, const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model,
			AuGLEffects& re);

	void RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	void RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc, const AuGlobeLLF& hitRoad,
			const QColor& hitColor, const QColor& roadColor) const;

	void RenderOnSurfaceTexture(const DaWorldLL& world, AuGLEffects& re, float radiansPerFootX, float radiansPerFootY) const;

	// Removes any excess bomb craters beyond 'maxBombCraters', starting with the first one.
	// Returns true if any are removed.
	bool TrimExcessBombCraters(int maxBombCraters);
};

inline QDataStream& operator<<(QDataStream& out, const AuGeologicalFeatures& geoFeatures)
{
	geoFeatures >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGeologicalFeatures& geoFeatures)
{
	geoFeatures << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoValley : public AuGeologicalFeature
{
private:
	float								m_StartMSL;				// Altitude at the bottom of the chasm at the starting ll.
	AuGlobeLLF							m_EndLL;				// LL at end of chasm.
	float								m_Width;				// Width of the chasm.

public:
	AuGeoValley(float width, float startMSL, const AuGlobeLLF& startLL, const AuGlobeLLF& endLL);
	AuGeoValley();
	AuGeoValley(const AuGeoValley& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 1;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Valley from %1 to %2, depth %3m MSL, width %4m").arg(m_LL.ToString()).arg(m_EndLL.ToString())
			.arg(m_StartMSL / FeetPerMeter()).arg(m_Width / FeetPerMeter());
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoCrater : public AuGeologicalFeature
{
protected:
	float								m_BottomMSL;			// Altitude at the bottom of the crater.
	float								m_DebrisHeight;			// Height of debris above local terrain at the rim.
	float								m_Radius;				// Radius of the crater.

public:
	AuGeoCrater(float radius, float debrisHeight, float bottomMSL, const AuGlobeLLF& ll);
	AuGeoCrater();
	AuGeoCrater(const AuGeoCrater& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 2;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Crater at %1, bottom %2m MSL, radius %3m, debris height %4m").arg(m_LL.ToString())
			.arg(m_BottomMSL / FeetPerMeter()).arg(m_Radius / FeetPerMeter()).arg(m_DebrisHeight / FeetPerMeter());
	}

	virtual float Radius() const
	{
		return m_Radius;
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoMesa : public AuGeologicalFeature
{
private:
	float								m_Radius;				// Radius of the mesas.
	float								m_TopMSL;				// Altitude at the top of the mesas.

public:
	AuGeoMesa(float radius, float topMSL, const AuGlobeLLF& ll);
	AuGeoMesa();
	AuGeoMesa(const AuGeoMesa& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 3;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Mesas at %1, top %2m MSL, radius %3m").arg(m_LL.ToString()).arg(m_TopMSL / FeetPerMeter())
			.arg(m_Radius / FeetPerMeter());
	}

	virtual float Radius() const
	{
		return m_Radius;
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoVolcano : public AuGeologicalFeature
{
private:
	float								m_Radius;				// Radius of the volcano caldera at peak MSL.
	float								m_Rise;					// Amount of altitude rise at the lip of the caldera.

public:
	AuGeoVolcano(float radius, float rise, const AuGlobeLLF& ll);
	AuGeoVolcano();
	AuGeoVolcano(const AuGeoVolcano& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

	virtual bool IsSmokey() const
	{
		return true;
	}

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 4;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Volcano at %1, rise %2m, radius %3m").arg(m_LL.ToString()).arg(m_Rise / FeetPerMeter())
			.arg(m_Radius / FeetPerMeter());
	}

	virtual float Radius() const
	{
		return m_Radius;
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual double SmokeRadius() const;

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoRiver : public AuGeologicalFeature
{
private:
	float								m_MouthMSL;				// Altitude at the mouth of the river, at m_LL.
	float								m_Persistence;			// For Perlin noise wiggliness.
	AuGlobeLLF							m_SpringLL;				// LL at spring of river, the beginning of the "water flow".
	float								m_Width;				// Width of the river.
	float								m_XOff;					// For Perlin noise wiggliness.

public:
	AuGeoRiver();
	AuGeoRiver(const AuGeoRiver& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 5;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("River from %1 to %2, mouth depth %3m MSL, width %4m, persistence %5, xoff %6")
			.arg(m_SpringLL.ToString())
			.arg(m_LL.ToString())
			.arg(m_MouthMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter())
			.arg(m_Persistence)
			.arg(m_XOff);
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoChasm : public AuGeologicalFeature
{
private:
	float								m_BottomMSL;			// Altitude at the bottom of the chasm.
	AuGlobeLLF							m_EndLL;				// LL at end of chasm.
	float								m_Persistence;			// For Perlin noise wiggliness.
	float								m_Width;				// Width of the chasm.
	float								m_XOff;					// For Perlin noise wiggliness.

public:
	AuGeoChasm();
	AuGeoChasm(const AuGeoChasm& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 6;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Chasm from %1 to %2, depth %3m MSL, width %4m, persistence %5, xoff %6")
			.arg(m_LL.ToString())
			.arg(m_EndLL.ToString())
			.arg(m_BottomMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter())
			.arg(m_Persistence)
			.arg(m_XOff);
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoHenge : public AuGeologicalFeature
{
private:
	AuTime								m_Active;				// Time when this henge portal became active.
	quint8								m_DestArcSegment;		// Destination ringworld arc segment.
	quint8								m_DestShadowPanels;		// Dest ringworld shadow panels = slabs in the central circle.
	quint8								m_DestSun;				// Destination sun: 0=primary, 1=secondary, 2=tertiary
	AuId								m_DestSysId;			// Destination system in current sector.
	float								m_TopMSL;				// Altitude at the top of the henge.

public:
	AuGeoHenge(const AuId& destSysId, quint8 destSun, quint8 destShadowPanels, quint8 destArcSegment, float topMSL,
			const AuGlobeLLF& ll);
	AuGeoHenge();
	AuGeoHenge(const AuGeoHenge& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	inline float CentralSlabAltitudeMSL() const
	{
		return m_TopMSL;
	}

	inline int CentralSlabCount() const
	{
		return m_DestShadowPanels;
	}

	// Returns the radius of the centerline of the ring of large stones surrounding the center.
	inline float CentralSlabRadius() const
	{
		// Central stones are 6' wide.
		float centerC = m_DestShadowPanels * 2 * 6;	// Circumference of central ring of slabs.
		float centerR = centerC / PIf / 2.f;		// Radius of central ring of slabs.
		return centerR;
	}

	virtual AuGeologicalFeature* Clone() const;

	inline quint8 DestArcSegment() const
	{
		return m_DestArcSegment;
	}

	inline quint8 DestShadowPanels() const
	{
		return m_DestShadowPanels;
	}

	inline quint8 DestSun() const
	{
		return m_DestSun;
	}

	inline const AuId& DestSysId() const
	{
		return m_DestSysId;
	}

	virtual ON_GROUND Ground() const;

	inline bool IsDestination(const AuId& sysId, quint8 sun, quint8 arcSegment) const
	{
		// Bogus shadow panels count cannot be the destination.
		if(!m_DestShadowPanels)
			return false;

		if(sysId != m_DestSysId)
			return false;
		if(sun != m_DestSun)
			return false;
		if(arcSegment != m_DestArcSegment)
			return false;

		return true;
	}

	// Holy places are revealed by village elders.
	virtual bool IsHoly() const
	{
		return true;
	}

	virtual bool IsTransporterTarget() const
	{
		return false;
	}

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 7;
	}

	inline bool IsActive() const
	{
		return !m_Active.IsNull() && m_Active.ElapsedMin() < 26;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Henge at %1, top %2m MSL, slabs %3, radius %4m").arg(m_LL.ToString()).arg(m_TopMSL / FeetPerMeter())
			.arg(m_DestShadowPanels).arg(TerrainRadius()/FeetPerMeter());
	}

	inline int OuterRingColumnCount() const
	{
		float outerR = CentralSlabRadius() + 5*FeetPerMeter();	// Minimum radius of outer ring.
		float outerC = PIf * outerR * 2.f;			// Minimum circumferene of outer ring.
		const float lintelL = 8.f;					// Lintels are 8' long that join tops of columns.
		float columns = ceilf(outerC / lintelL);	// Number of columns needed for integral number of lintels.
		return (int)columns;
	}

	// Returns the radius of the centerline of the ring of columns surrounding the central slabs.
	inline float OuterRingRadius() const
	{
		float outerR = CentralSlabRadius() + 5*FeetPerMeter();	// Minimum radius of outer ring.
		float outerC = PIf * outerR * 2.f;			// Minimum circumferene of outer ring.
		const float lintelL = 8.f;					// Lintels are 8' long that join tops of columns.
		float columns = ceilf(outerC / lintelL);	// Number of columns needed for integral number of lintels.
		outerC = columns * lintelL;					// Circumference of columns.
		outerR = outerC / PIf / 2.f;				// Radius of columns.
		return outerR;
	}

	virtual float Radius() const
	{
		return TerrainRadius();
	}

	// Find a location that is not in a forbidden zone.
	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	inline void SetActive(bool isActive)
	{
		if(isActive)
			m_Active.Set();
		else
			m_Active = AuTime();
	}

	// Returns radius of level terrain of this henge.
	inline float TerrainRadius() const
	{
		return OuterRingRadius() + 10*FeetPerMeter();			// Give us a 10m margin around the outer ring.
	}

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoVent : public AuGeologicalFeature
{
private:
	float								m_Radius;				// Radius of the volcano caldera at peak MSL.
	float								m_Rise;					// Amount of altitude rise at the lip of the caldera.

public:
	AuGeoVent(float radius, float rise, const AuGlobeLLF& ll);
	AuGeoVent();
	AuGeoVent(const AuGeoVent& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

	virtual bool IsSmokey() const
	{
		return true;
	}

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 8;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Vent at %1, rise %2m, radius %3m").arg(m_LL.ToString()).arg(m_Rise / FeetPerMeter())
			.arg(m_Radius / FeetPerMeter());
	}

	virtual float Radius() const
	{
		return m_Radius;
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual double SmokeRadius() const;

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoFissure : public AuGeologicalFeature
{
private:
	float								m_BottomMSL;			// Altitude at the bottom of the chasm.
	AuGlobeLLF							m_EndLL;				// LL at end of chasm.
	float								m_Persistence;			// For Perlin noise wiggliness.
	float								m_Width;				// Width of the chasm.
	float								m_XOff;					// For Perlin noise wiggliness.

public:
	AuGeoFissure();
	AuGeoFissure(const AuGeoFissure& c);

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	static quint8 GeoType()
	{
		return 9;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Fissure from %1 to %2, depth %3m MSL, width %4m, persistence %5, xoff %6")
			.arg(m_LL.ToString())
			.arg(m_EndLL.ToString())
			.arg(m_BottomMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter())
			.arg(m_Persistence)
			.arg(m_XOff);
	}

	virtual void RandomInit(const DaWorldLL& llworld, const AuDice& dice, const AuGeologicalFeatures& existing);

	virtual QString TransporterName(HYDROGRAPHIC_LIQUID hliq) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Creates terrain graded like a road but it does not affect the presence of rocks, grass, shrubs or trees.

class AuGeoRoadGrade : public AuGeologicalFeature
{
protected:
	float								m_EndMSL;				// Altitude at the end of the road, at m_LL.
	DeConstructionProcess*				m_pConstruct;			// Only exists during the construction of this road.
	bool								m_Slab;					// Slab indicator.
	AuGlobeLLF							m_StartLL;				// LL at start of road.
	float								m_StartMSL;				// Altitude at the start of the road, at m_StartLL.
	AuTime								m_TimeCreated;			// Time this feature was created.
	float								m_Width;				// Width of the road.

public:
	// 'cp' can be null to start out fully constructed.
	AuGeoRoadGrade(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width, bool slab,
			const DeConstructionProcess* cp);
	AuGeoRoadGrade();
	AuGeoRoadGrade(const AuGeoRoadGrade& c);

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

public:
	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual float AlignmentRot(const DaWorldLL& llworld, const AuGlobeLLF& alignLL) const;

	AuPoint Center(const DaWorldLL& world) const;

	virtual AuGeologicalFeature* Clone() const;

	virtual bool ConstructionEnable(bool enable);

	virtual bool ConstructionFetch(const DaWorldLL& llworld);

	virtual int ConstructionGo(AuGear& gear);

	virtual bool ConstructionProcess(DeConstructionProcess& cp) const;

	virtual INDUSTRYID GeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	static quint8 GeoType()
	{
		return 10;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual bool IsConstructionWorkerEnabled() const;

	virtual bool IsDeletable() const
	{
		return true;
	}

	virtual bool IsDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	virtual bool IsReconstructable() const
	{
		return true;
	}

	virtual bool IsSlab() const
	{
		return m_Slab;
	}

	virtual bool IsTransporterTarget() const
	{
		return false;
	}

	virtual bool IsUnderConstruction() const
	{
		return m_pConstruct;
	}

	virtual double LLMSL() const
	{
		return m_EndMSL;
	}

	virtual const AuGlobeLLF& LL2() const
	{
		return m_StartLL;
	}

	virtual double LL2MSL() const
	{
		return m_StartMSL;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("%1 bed from %2 at %3m MSL to %4 %5m MSL, width %6m")
			.arg(IsSlab()?QObject::tr("Slab"):QObject::tr("Road"))
			.arg(m_StartLL.ToString())
			.arg(m_StartMSL / FeetPerMeter())
			.arg(m_LL.ToString())
			.arg(m_EndMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter());
	}

	virtual float Radius() const
	{
		return m_Width/2;
	}

	virtual void RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	virtual void RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
			const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}

	virtual float Width() const
	{
		return m_Width;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoRoadDirt : public AuGeoRoadGrade
{
public:
	AuGeoRoadDirt(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width, bool slab,
			const DeConstructionProcess* cp);
	AuGeoRoadDirt();
	AuGeoRoadDirt(const AuGeoRoadDirt& c);

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual AuGeologicalFeature* Clone() const;

	virtual void CreateRoadMap(const DaWorldLL& world, AuRoadMap& roadMap) const;

	virtual INDUSTRYID GeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	static quint8 GeoType()
	{
		return 11;
	}

	virtual ON_GROUND Ground() const;

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual bool IsRoad() const
	{
		return true;
	}

	virtual QColor MapColor() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Dirt %1 from %2 at %3m MSL to %4 %5m MSL, width %6m")
			.arg(IsSlab()?QObject::tr("slab"):QObject::tr("road"))
			.arg(m_StartLL.ToString())
			.arg(m_StartMSL / FeetPerMeter())
			.arg(m_LL.ToString())
			.arg(m_EndMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter());
	}

	virtual void PaintOnSurfaceMap(const DaWorldLL& world, QPainter& p, unsigned int width, unsigned int height) const;

	// Shader has already been configured.
	virtual void RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
			const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	virtual void RenderOnSurfaceTexture(const DaWorldLL& world, AuGLEffects& re, float radiansPerFootX,
			float radiansPerFootY) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}

protected:
	// Adds one or more quads to 'verts' to draw a road on a 2D map.
	static void BuildRoadVerts(const AuPoint& s, const AuPoint& e, float wid2, float radiansPerFootX, float radiansPerFootY,
			AuPointArray& verts);

	void CreateModelQuad(AuScFaceList& faces, const AuPoint& pl, const AuPoint& nl, const AuPoint& nr,
			const AuPoint& pr, const AuPoint& plz, const AuPoint& nlz, const AuPoint& nrz, const AuPoint& prz,
			const QColor& paint) const;

	void CreateModelSectionQuad(AuScFaceList& faces, const AuPoint& pl, const AuPoint& nl, const AuPoint& nr,
			const AuPoint& pr, const AuPoint& plz, const AuPoint& nlz, const AuPoint& nrz, const AuPoint& prz,
			const QColor& paint) const;

	// 'texIndex' 0=asphalt, 1=concrete
	void CreatePavement(const DaWorldLL& world, AuScFaceList& faces, quint8 texIndex) const;

	void CreateRoadModelSection(const DaWorldLL& world, AuScFaceList& faces, const AuPoint& prevLeft,
			const AuPoint& nextLeft, const AuPoint& nextRight, const AuPoint& prevRight, bool prevCap, bool nextCap) const;

	void CreateSlabModelSection(const DaWorldLL& world, AuScFaceList& faces, const AuPoint& prevLeft,
			const AuPoint& nextLeft, const AuPoint& nextRight, const AuPoint& prevRight, bool prevCap, bool nextCap) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoRoadAsphalt : public AuGeoRoadDirt
{
	AuGLVAO*							m_VAO;					// Road surface model.

public:
	AuGeoRoadAsphalt(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width, bool slab,
			const DeConstructionProcess* cp);
	AuGeoRoadAsphalt();
	AuGeoRoadAsphalt(const AuGeoRoadAsphalt& c);
	virtual ~AuGeoRoadAsphalt();

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	virtual AuGeologicalFeature* Clone() const;

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	static quint8 GeoType()
	{
		return 12;
	}

	virtual ON_GROUND Ground() const;

	virtual bool HasPavementModel() const
	{
		return true;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual QColor MapColor() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Asphalt %1 from %2 at %3m MSL to %4 %5m MSL, width %6m")
			.arg(IsSlab()?QObject::tr("slab"):QObject::tr("road"))
			.arg(m_StartLL.ToString())
			.arg(m_StartMSL / FeetPerMeter())
			.arg(m_LL.ToString())
			.arg(m_EndMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter());
	}

	virtual void Render_Shader(AuGLSLScBase* shader, const DaWorldLL& world, const AuMatrix4x4& projection,
			const AuMatrix4x4& model, AuGLEffects& re);

	virtual bool TestBombCraterHit(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}

	virtual AuGeologicalFeature* UnpavedEquivalent() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoRoadConcrete : public AuGeoRoadDirt
{
	AuGLVAO*							m_VAO;					// Road surface model.

public:
	AuGeoRoadConcrete(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width, bool slab,
			const DeConstructionProcess* cp);
	AuGeoRoadConcrete();
	AuGeoRoadConcrete(const AuGeoRoadConcrete& c);
	virtual ~AuGeoRoadConcrete();

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	virtual AuGeologicalFeature* Clone() const;

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	static quint8 GeoType()
	{
		return 13;
	}

	virtual ON_GROUND Ground() const;

	virtual bool HasPavementModel() const
	{
		return true;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual QColor MapColor() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Concrete %1 from %2 at %3m MSL to %4 %5m MSL, width %6m")
			.arg(IsSlab()?QObject::tr("slab"):QObject::tr("road"))
			.arg(m_StartLL.ToString())
			.arg(m_StartMSL / FeetPerMeter())
			.arg(m_LL.ToString())
			.arg(m_EndMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter());
	}

	virtual void Render_Shader(AuGLSLScBase* shader, const DaWorldLL& world, const AuMatrix4x4& projection,
			const AuMatrix4x4& model, AuGLEffects& re);

	virtual bool TestBombCraterHit(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}

	virtual AuGeologicalFeature* UnpavedEquivalent() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeoLandscape : public AuGeologicalFeature
{
protected:
	float								m_EndMSL;				// Altitude at the end of the landscape area, at m_LL.
	bool								m_HideRocks;
	int									m_HideGrass;			// 0=no, 1=yes, 2=mow, 3=lawn
	bool								m_HideShrubs;
	bool								m_HideTrees;
	WORLDTEX_TYPE						m_Pavement;
	DeConstructionProcess*				m_pConstruct;			// Only exists during the construction of this road.
	AuGlobeLLF							m_StartLL;				// LL at start of landscape area.
	float								m_StartMSL;				// Altitude at the start of the landscape area, at m_StartLL.
	AuTime								m_TimeCreated;			// Time this feature was created.
	float								m_Width;				// Width of the landscape area.

public:
	// 'cp' can be null to start out fully constructed.
	AuGeoLandscape(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width, bool hideRocks,
			int hideGrass, bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavementType, const DeConstructionProcess* cp);
	AuGeoLandscape();
	AuGeoLandscape(const AuGeoLandscape& c);

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

	virtual bool ConstructionEnable(bool enable);

	virtual bool ConstructionFetch(const DaWorldLL& llworld);

	virtual int ConstructionGo(AuGear& gear);

	virtual bool ConstructionProcess(DeConstructionProcess& cp) const;

	virtual INDUSTRYID GeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	static quint8 GeoType()
	{
		return 14;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual bool IsConstructionWorkerEnabled() const;

	virtual bool IsDeletable() const
	{
		return true;
	}

	virtual bool IsDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	virtual bool IsDefoliant() const
	{
		return m_HideGrass == 1;
	}

	virtual bool IsIrrigation() const
	{
		return m_HideGrass == 3;
	}

	virtual bool IsReconstructable() const
	{
		return true;
	}

	virtual bool IsTransporterTarget() const
	{
		return false;
	}

	virtual bool IsUnderConstruction() const
	{
		return m_pConstruct;
	}

	virtual double LLMSL() const
	{
		return m_EndMSL;
	}

	virtual const AuGlobeLLF& LL2() const
	{
		return m_StartLL;
	}

	virtual double LL2MSL() const
	{
		return m_StartMSL;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Landscape from %2 at %3m MSL to %4 %5m MSL, width %6m")
			.arg(m_StartLL.ToString())
			.arg(m_StartMSL / FeetPerMeter())
			.arg(m_LL.ToString())
			.arg(m_EndMSL / FeetPerMeter())
			.arg(m_Width / FeetPerMeter());
	}

	virtual float Radius() const
	{
		return m_Width/2;
	}

	virtual void RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	// Shader has already been configured.
	virtual void RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
			const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A bomb crater is just a regular crater that can be deleted using the bulldozer.

class AuGeoBombCrater : public AuGeoCrater
{
protected:
	AuTime								m_TimeCreated;			// Time this feature was created.

public:
	AuGeoBombCrater(float radius, float debrisHeight, float bottomMSL, const AuGlobeLLF& ll);
	AuGeoBombCrater();
	AuGeoBombCrater(const AuGeoBombCrater& c);

public:
	virtual bool operator==(const AuGeologicalFeature& c) const;
	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
			bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
			int& onRoadOrGrader, AuTime& newest) const;

	virtual double AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const;

	virtual AuGeologicalFeature* Clone() const;

	static quint8 GeoType()
	{
		return 16;
	}

	// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
	// Returns the name to fit into these sentences:
	//   "Nearby is %1."
	//   "You should see %1."
	//   "Visitors from afar tell about %1."
	virtual QString IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const;

	virtual bool IsBombCrater() const
	{
		return true;
	}

	virtual bool IsDeletable() const
	{
		return true;
	}

	virtual bool IsDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	virtual bool IsDeletableCraterContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const;

	// Bomb crater is smokey for a while.
	virtual bool IsSmokey() const;

	virtual bool IsTransporterTarget() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return QObject::tr("Bomb Crater at %1, bottom %2m MSL, radius %3m, debris height %4m").arg(m_LL.ToString())
			.arg(m_BottomMSL / FeetPerMeter()).arg(m_Radius / FeetPerMeter()).arg(m_DebrisHeight / FeetPerMeter());
	}

	virtual double SmokeRadius() const;

	virtual void RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	virtual void RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
			const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const;

	// Returns a number that is unique to each derived situation class.
	virtual quint8 Type() const
	{
		return GeoType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuGeologicalFeature_H
