//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augeologicalfeature.cpp,v 1.88 2022/02/19 17:40:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augeologicalfeature.h"
//
#include "audice.h"
#include "augear.h"
#include "auglobal.h"
#include "aunoise.h"
#include "auroadmap.h"
#include "auweather.h"
#include "auzoneresource.h"
#include "dasolarsystem.h"
#include "daworldll.h"
#include "dbindustry.h"
#include "deconstructionprocess.h"
#include <QDebug>

AuGeologicalFeature::AuGeologicalFeature(const AuGlobeLLF& ll)
{
	m_LL = ll;
}

AuGeologicalFeature::AuGeologicalFeature()
{
}

AuGeologicalFeature::~AuGeologicalFeature()
{
}

AuGeologicalFeature::AuGeologicalFeature(const AuGeologicalFeature& c)
{
	m_LL = c.m_LL;
}

bool AuGeologicalFeature::operator==(const AuGeologicalFeature& c) const
{
	if(m_LL != c.m_LL)
		return false;
	return true;
}

void AuGeologicalFeature::operator<<(QDataStream& in)
{
	in >> m_LL;
}

void AuGeologicalFeature::operator>>(QDataStream& out) const
{
	out << m_LL;
}

AuGeologicalFeature* AuGeologicalFeature::Create(quint8 type)
{
	if(type == AuGeoBombCrater::GeoType())
		return new AuGeoBombCrater;
	else if(type == AuGeoChasm::GeoType())
		return new AuGeoChasm;
	else if(type == AuGeoCrater::GeoType())
		return new AuGeoCrater;
	else if(type == AuGeoFissure::GeoType())
		return new AuGeoFissure;
	else if(type == AuGeoHenge::GeoType())
		return new AuGeoHenge;
	else if(type == AuGeoLandscape::GeoType())
		return new AuGeoLandscape;
	else if(type == AuGeoMesa::GeoType())
		return new AuGeoMesa;
	else if(type == AuGeoRiver::GeoType())
		return new AuGeoRiver;
	else if(type == AuGeoRoadAsphalt::GeoType())
		return new AuGeoRoadAsphalt;
	else if(type == AuGeoRoadConcrete::GeoType())
		return new AuGeoRoadConcrete;
	else if(type == AuGeoRoadDirt::GeoType())
		return new AuGeoRoadDirt;
	else if(type == AuGeoRoadGrade::GeoType())
		return new AuGeoRoadGrade;
	else if(type == AuGeoValley::GeoType())
		return new AuGeoValley;
	else if(type == AuGeoVent::GeoType())
		return new AuGeoVent;
	else if(type == AuGeoVolcano::GeoType())
		return new AuGeoVolcano;
	return 0;
}

void AuGeologicalFeature::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	// Choose a random location that is at least 1m above water.
	// Most geological features are not very interesting under water.
	AuDice dice(Cdice);
	int sanity = 1000;
	do
	{
		m_LL = llworld.RandomLandLocation(dice, FeetPerMeter());
		if(existing.Find(m_LL))
		{
			dice.Random();
			m_LL = AuGlobeLLF();
		}
	}
	while(m_LL == AuGlobeLLF() && sanity--);
}

float AuGeologicalFeature::Width() const
{
	return 0;
}

quint64 AuGeologicalFeature::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeologicalFeature);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

float AuGeologicalFeatures::AlignmentRot(const DaWorldLL& llworld, const AuGlobeLLF& alignLL, double searchRadius,
		AuGlobeLLF& hitLL) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		INDUSTRYID geoiid = (*fi)->GeoIndustryContaining(llworld, alignLL, searchRadius);
		if(geoiid == IID_Road)
		{
			hitLL = (*fi)->LL();
			return (*fi)->AlignmentRot(llworld, alignLL);
		}
	}
	return 99.f;
}

unsigned int AuGeologicalFeatures::CountBombCraters() const
{
	unsigned int craters = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsBombCrater())
			craters++;
	return craters;
}

void AuGeologicalFeatures::CreateRoadMap(const DaWorldLL& world, AuRoadMap& roadMap) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		(*fi)->CreateRoadMap(world, roadMap);
}

bool AuGeologicalFeatures::ConstructionEnable(const AuGlobeLLF& ll, bool enable)
{
	for(iterator fi = begin(); fi != end(); fi++)
		if((*fi)->LL() == ll && (*fi)->ConstructionEnable(enable))
			return true;
	return false;
}

void AuGeologicalFeatures::ConstructionFetch(const DaWorldLL& world)
{
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->ConstructionFetch(world);
}

bool AuGeologicalFeatures::ConstructionFetch(const AuGlobeLLF& ll, const DaWorldLL& world)
{
	for(iterator fi = begin(); fi != end(); fi++)
		if((*fi)->LL() == ll && (*fi)->ConstructionFetch(world))
			return true;
	return false;
}

int AuGeologicalFeatures::ConstructionGo()
{
	int bitResult = 0;
	AuGear gear;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(!(*fi)->IsConstructionWorkerEnabled())
			continue;
		bitResult |= (*fi)->ConstructionGo(gear);	// Turn on result bits.
	}
	return bitResult;
}

int AuGeologicalFeatures::ConstructionGo(const AuGlobeLLF& ll, AuGear& gear)
{
	for(iterator fi = begin(); fi != end(); fi++)
		if((*fi)->LL() == ll)
			return (*fi)->ConstructionGo(gear);
	return 0;	// Did not find the geo at ll.
}

bool AuGeologicalFeatures::FindGeoUnderConstruction(const AuGlobeLLF& geoLL, DeConstructionProcess& cp) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		if((*fi)->LL() != geoLL)
			continue;
		if((*fi)->ConstructionProcess(cp))
			return true;
	}
	return false;
}

ON_GROUND AuGeologicalFeatures::Ground(const AuGlobeLLF& geoLL) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->LL() == geoLL)
			return (*fi)->Ground();
	return OG_None;
}

INDUSTRYID AuGeologicalFeatures::FindGeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius,
		AuGlobeLLF& geoLL) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		INDUSTRYID geoiid = (*fi)->GeoIndustryContaining(llworld, ll, radius);
		if(geoiid != IID_None)
		{
			geoLL = (*fi)->LL();
			return geoiid;
		}
	}
	return IID_None;
}

const AuGeologicalFeature* AuGeologicalFeatures::Find(const AuGlobeLLF& geoLL) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->LL() == geoLL)
			return *fi;
	return 0;
}

const AuGeologicalFeature* AuGeologicalFeatures::FindDeletable(const AuGlobeLLF& geoLL) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsDeletable() && (*fi)->LL() == geoLL)
			return *fi;
	return 0;
}

const AuGeologicalFeature* AuGeologicalFeatures::FindReconstructable(const AuGlobeLLF& geoLL) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsReconstructable() && (*fi)->LL() == geoLL)
			return *fi;
	return 0;
}

void AuGeologicalFeatures::FindDeletableCratersContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll,
		double radius, QList<const AuGeologicalFeature*>& found) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsDeletableCraterContaining(llworld, ll, radius))
			found.append(*fi);
}

void AuGeologicalFeatures::FindDeletablesContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll,
		double radius, QList<const AuGeologicalFeature*>& found) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsDeletableContaining(llworld, ll, radius))
			found.append(*fi);
}

void AuGeologicalFeatures::FindDeletablesContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll,
		double radius, QList<AuGeologicalFeature*>& found)
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsDeletableContaining(llworld, ll, radius))
			found.append(*fi);
}

const AuGeologicalFeature* AuGeologicalFeatures::FindDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll,
		double radius) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsDeletableContaining(llworld, ll, radius))
			return *fi;
	return 0;
}

AuGeologicalFeature* AuGeologicalFeatures::FindDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius)
{
	for(iterator fi = begin(); fi != end(); fi++)
		if((*fi)->IsDeletableContaining(llworld, ll, radius))
			return *fi;
	return 0;
}

double AuGeologicalFeatures::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks,
		int& hideGrass, bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, AuTime& newest) const
{
	int onRoadOrGrader = 0;
	newest = AuTime();
	for(const_iterator fi = constBegin(); fi != constEnd(); fi++)
		altMSL = (*fi)->AdjustAltMSL(llworld, ll, altMSL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType,
				onRoadOrGrader, newest);
	return altMSL;
}

double AuGeologicalFeatures::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType, AuTime& newest) const
{
	int onRoadOrGrader = 0;
	newest = AuTime();
	for(const_iterator fi = constBegin(); fi != constEnd(); fi++)
		altMSL = (*fi)->AdjustAltMSL(llworld, ll, altMSL, pavementType, onRoadOrGrader, newest);
	return altMSL;
}

double AuGeologicalFeatures::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	// Virgin terrain altitude without developments.
	for(const_iterator fi = constBegin(); fi != constEnd(); fi++)
		altMSL = (*fi)->AdjustAltMSLUndeveloped(llworld, ll, altMSL);
	return altMSL;
}

void AuGeologicalFeatures::RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		AuGLEffects& re, const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * model);
	re.glDisable(GL_DEPTH_TEST);
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		(*fi)->RenderCursor_Shader(world, re, nearLoc, hitRoad, hitColor, roadColor);
	re.glDisable(GL_BLEND);
	re.glEnable(GL_DEPTH_TEST);
	re.GLSL().LineShader().Deselect();
}

void AuGeologicalFeatures::RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
		const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		(*fi)->RenderCursor_Shader(world, re, nearLoc, hitRoad, hitColor, roadColor);
}

bool AuGeologicalFeatures::HasPavementModel() const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->HasPavementModel())
			return true;
	return false;
}

bool AuGeologicalFeatures::TrimExcessBombCraters(int maxBombCraters)
{
	int bombCraters = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->IsBombCrater())
			bombCraters++;
	if(bombCraters <= maxBombCraters)
		return false;
	int removeCraters = bombCraters - maxBombCraters;
	iterator fi = begin();
	while(fi != end())
	{
		if((*fi)->IsBombCrater())
		{
			delete *fi;
			fi = erase(fi);
			removeCraters--;
			if(!removeCraters)
				break;
		}
		else
			fi++;
	}
	return true;
}

void AuGeologicalFeatures::Render_Shader(AuGLSLScBase* shader, const DaWorldLL& world, const AuMatrix4x4& projection,
		const AuMatrix4x4& model, AuGLEffects& re)
{
	if(g_Au.m_DebugSwitches & 0x20000000)
		return;
	// Render roads.
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->Render_Shader(shader, world, projection, model, re);
}

void AuGeologicalFeatures::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
	{
		out << (*ci)->Type();
		out << **ci;
	}
}

void AuGeologicalFeatures::operator<<(QDataStream& in)
{
	Clear();

	quint16 count;
	in >> count;
	quint8 type;
	while(count--)
	{
		in >> type;
		AuGeologicalFeature* geo = AuGeologicalFeature::Create(type);
		if(geo)
		{
			in >> *geo;
			if(type == AuGeoHenge::GeoType())
			{
				//qDebug() << geo->Name() << geo->LL();
				AuGeoHenge* h = (AuGeoHenge*)geo;
				if(h->CentralSlabCount() == 0)
				{
					qWarning() << "AuGeologicalFeatures::operator<< removing broken henge" << geo->Name() << geo->LL();
					delete geo;
					continue;
				}
			}
			append(geo);
		}
		else
			qWarning() << "AuGeologicalFeatures::operator<< received invalid type" << type;
	}
}

bool AuGeologicalFeatures::Delete(const AuGlobeLLF& geoLL)
{
	for(iterator fi = begin(); fi != end(); fi++)
		if((*fi)->IsDeletable() && (*fi)->LL() == geoLL)
		{
			delete *fi;
			erase(fi);
			return true;
		}
	return false;
}

bool AuGeologicalFeatures::DeleteGeo(const AuGeologicalFeature* geo)
{
	for(iterator fi = begin(); fi != end(); fi++)
		if(*fi == geo)
		{
			delete *fi;
			erase(fi);
			return true;
		}
	return false;
}

void AuGeologicalFeatures::CraterPavement(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius)
{
	iterator fi = begin();
	while(fi != end())
	{
		if((*fi)->TestBombCraterHit(llworld, ll, radius))
		{
			// Blow the pavement off the thing by replacing it with an unpaved equivalent.
			// We do not destroy roads because that would break building interconnections.
			AuGeologicalFeature* unpavedRoad = (*fi)->UnpavedEquivalent();
			delete *fi;
			*fi = unpavedRoad;
		}
		else
			fi++;
	}
}

void AuGeologicalFeatures::PaintOnSurfaceMap(const DaWorldLL& world, QPainter& p, unsigned int width, unsigned int height) const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
		(*gi)->PaintOnSurfaceMap(world, p, width, height);
}

void AuGeologicalFeatures::RenderOnSurfaceTexture(const DaWorldLL& world, AuGLEffects& re, float radiansPerFootX,
		float radiansPerFootY) const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
		(*gi)->RenderOnSurfaceTexture(world, re, radiansPerFootX, radiansPerFootY);
}

quint64 AuGeologicalFeatures::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeologicalFeatures);
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
		mem += (*gi)->MemoryUsage();
	return mem;
}

void AuGeologicalFeatures::AddBombCrater(const DaWorldLL& llworld, float craterRadius, float debrisHeight, float bottomMSL,
		const AuGlobeLLF& ll)
{
	// Unpave roads affected by the new bomb crater so we don't leave chunks of pavement hanging in the air.
	CraterPavement(llworld, ll, craterRadius);
	append(new AuGeoBombCrater(craterRadius, debrisHeight, bottomMSL, ll));
}

bool AuGeologicalFeatures::DeleteDeletableFeatures()
{
	bool anyDel = false;
	iterator fi = begin();
	while(fi != end())
	{
		if((*fi)->IsDeletable())
		{
			delete *fi;
			fi = erase(fi);
			anyDel = true;
		}
		else
			fi++;
	}
	return anyDel;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoValley::AuGeoValley(float width, float startMSL, const AuGlobeLLF& startLL, const AuGlobeLLF& endLL) :
	AuGeologicalFeature(startLL)
{
	m_StartMSL = startMSL;
	m_EndLL = endLL;
	m_Width = width;
}

AuGeoValley::AuGeoValley()
{
	m_StartMSL = 0;
	m_Width = 0;
}

AuGeoValley::AuGeoValley(const AuGeoValley& c) : AuGeologicalFeature(c)
{
	m_StartMSL = c.m_StartMSL;
	m_EndLL = c.m_EndLL;
	m_Width = c.m_Width;
}

bool AuGeoValley::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoValley& c = dynamic_cast<const AuGeoValley&>(b);
		if(m_StartMSL != c.m_StartMSL)
			return false;
		if(m_EndLL != c.m_EndLL)
			return false;
		if(m_Width != c.m_Width)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoValley::Clone() const
{
	return new AuGeoValley(*this);
}

void AuGeoValley::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_StartMSL;
	in >> m_EndLL;
	in >> m_Width;
}

void AuGeoValley::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_StartMSL;
	out << m_EndLL;
	out << m_Width;
}

void AuGeoValley::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	AuGeologicalFeature::RandomInit(llworld, dice, existing);

	// Choose a random end location.
	float lat = m_LL.Latitude() + dice.Roll(1,91,-46) * DEG2RADf;
	if(lat > PIf/2)
		lat -= PIf/2;	// Bounce off the north pole.
	else if(lat < -PIf/2)
		lat += PIf/2;	// Bounce off the south pole.
	float lon = m_LL.Longitude() + dice.Roll(1,181,-91) * DEG2RADf;
	m_EndLL = AuGlobeLLF(lon, lat);								// Normalizes longitude into proper range.

	// Choose an altitude for the low end of the valley.
	float altMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()));
	if(altMSL <= 0)
	{
		// Location is already under water. Choose a location lower than the current terrain altitude.
		m_StartMSL = altMSL - dice.Roll(1,100) * FeetPerMeter();
	}
	else
	{
		// Location is above water.
		m_StartMSL = dice.Roll(1,100,-90) * FeetPerMeter();		// Pick an altitude that is probably under water.
		if(altMSL - m_StartMSL < 50*FeetPerMeter())
			m_StartMSL = altMSL - dice.Roll(1,100,50) * FeetPerMeter();	// Force altitude to be at least 50m lower than the terrain.
	}

	// Choose a random width. This is actually half the width since it is treated like a radius.
	m_Width = dice.Roll(1,400,100) * FeetPerMeter();
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoValley::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint loc = llworld.LocOfLL(ll, m_StartMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the beginning of the valley.
		if(altMSL > m_StartMSL)
		{
			// Only pull points down, not up.
			AuPoint startloc = llworld.LocOfLL(m_LL, m_StartMSL);				// Start location at start altitude.
			float dist = loc.Distance2(startloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the start of the valley to be influenced by it.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(m_StartMSL, altMSL, radiusFactor);
			}
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the ending of the valley.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_StartMSL);					// End location at start altitude, for consistency.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the end of the valley to be influenced by it.
			// End of valley is at the terrain altitude.
			float endAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_EndLL.Longitude(), m_EndLL.Latitude()));
			if(altMSL > endAltMSL)
			{
				// Only pull points down, not up.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(endAltMSL, altMSL, radiusFactor);
			}
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_EndLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_StartMSL);				// Location of llvalley at start altitude.
		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the valley floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			// Interpolate the altitude of the valley along the valley segement.
			float endAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_EndLL.Longitude(), m_EndLL.Latitude()));
			float valleyAltMSL = AuNoiseF::Linear_Interpolate(m_StartMSL, endAltMSL, segDot);
			if(altMSL > valleyAltMSL)
			{
				// Only pull points down, not up.
				// Interpolate between valley base altitude and native terrain altitude.
				valleyDist = sqrtf(valleyDist);
				float valleyFactor = valleyDist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(valleyAltMSL, altMSL, valleyFactor);
			}
		}
	}

	return altMSL;
}

double AuGeoValley::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint loc = llworld.LocOfLL(ll, m_StartMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the beginning of the valley.
		if(altMSL > m_StartMSL)
		{
			// Only pull points down, not up.
			AuPoint startloc = llworld.LocOfLL(m_LL, m_StartMSL);				// Start location at start altitude.
			float dist = loc.Distance2(startloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the start of the valley to be influenced by it.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(m_StartMSL, altMSL, radiusFactor);
			}
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the ending of the valley.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_StartMSL);					// End location at start altitude, for consistency.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the end of the valley to be influenced by it.
			// End of valley is at the terrain altitude.
			float endAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_EndLL.Longitude(), m_EndLL.Latitude()));
			if(altMSL > endAltMSL)
			{
				// Only pull points down, not up.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(endAltMSL, altMSL, radiusFactor);
			}
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_EndLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_StartMSL);				// Location of llvalley at start altitude.
		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the valley floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			// Interpolate the altitude of the valley along the valley segement.
			float endAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_EndLL.Longitude(), m_EndLL.Latitude()));
			float valleyAltMSL = AuNoiseF::Linear_Interpolate(m_StartMSL, endAltMSL, segDot);
			if(altMSL > valleyAltMSL)
			{
				// Only pull points down, not up.
				// Interpolate between valley base altitude and native terrain altitude.
				valleyDist = sqrtf(valleyDist);
				float valleyFactor = valleyDist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(valleyAltMSL, altMSL, valleyFactor);
			}
		}
	}
	return altMSL;
}

double AuGeoValley::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	AuPoint loc = llworld.LocOfLL(ll, m_StartMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the beginning of the valley.
		if(altMSL > m_StartMSL)
		{
			// Only pull points down, not up.
			AuPoint startloc = llworld.LocOfLL(m_LL, m_StartMSL);				// Start location at start altitude.
			float dist = loc.Distance2(startloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the start of the valley to be influenced by it.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(m_StartMSL, altMSL, radiusFactor);
			}
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the ending of the valley.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_StartMSL);					// End location at start altitude, for consistency.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the end of the valley to be influenced by it.
			// End of valley is at the terrain altitude.
			float endAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_EndLL.Longitude(), m_EndLL.Latitude()));
			if(altMSL > endAltMSL)
			{
				// Only pull points down, not up.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(endAltMSL, altMSL, radiusFactor);
			}
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_EndLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_StartMSL);				// Location of llvalley at start altitude.
		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the valley floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			// Interpolate the altitude of the valley along the valley segement.
			float endAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_EndLL.Longitude(), m_EndLL.Latitude()));
			float valleyAltMSL = AuNoiseF::Linear_Interpolate(m_StartMSL, endAltMSL, segDot);
			if(altMSL > valleyAltMSL)
			{
				// Only pull points down, not up.
				// Interpolate between valley base altitude and native terrain altitude.
				valleyDist = sqrtf(valleyDist);
				float valleyFactor = valleyDist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(valleyAltMSL, altMSL, valleyFactor);
			}
		}
	}
	return altMSL;
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoValley::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	if(m_StartMSL <= 0)
	{
		// Width is 100 to 500 meters.
		if(m_Width < 150*FeetPerMeter())
			return QObject::tr("a narrow submerged valley");
		else if(m_Width < 350*FeetPerMeter())
			return QObject::tr("a submerged valley");
		else
			return QObject::tr("a wide submerged valley");
	}
	else
	{
		// Width is 100 to 500 meters.
		if(m_Width < 150*FeetPerMeter())
			return QObject::tr("a narrow valley");
		else if(m_Width < 350*FeetPerMeter())
			return QObject::tr("a valley");
		else
			return QObject::tr("a wide valley");
	}
}

QString AuGeoValley::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	if(hliq != HYDRO_None && m_StartMSL <= 0)
		return QObject::tr("Submerged Valley");
	else
		return QObject::tr("Valley");
}

quint64 AuGeoValley::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoValley);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoCrater::AuGeoCrater(float radius, float debrisHeight, float bottomMSL, const AuGlobeLLF& ll) : AuGeologicalFeature(ll)
{
	m_BottomMSL = bottomMSL;
	m_DebrisHeight = debrisHeight;
	m_Radius = radius;
}

AuGeoCrater::AuGeoCrater()
{
	m_BottomMSL = 0;
	m_DebrisHeight = 0;
	m_Radius = 0;
}

AuGeoCrater::AuGeoCrater(const AuGeoCrater& c) : AuGeologicalFeature(c)
{
	m_BottomMSL = c.m_BottomMSL;
	m_DebrisHeight = c.m_DebrisHeight;
	m_Radius = c.m_Radius;
}

bool AuGeoCrater::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoCrater& c = dynamic_cast<const AuGeoCrater&>(b);
		if(m_BottomMSL != c.m_BottomMSL)
			return false;
		if(m_DebrisHeight != c.m_DebrisHeight)
			return false;
		if(m_Radius != c.m_Radius)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoCrater::Clone() const
{
	return new AuGeoCrater(*this);
}

void AuGeoCrater::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_BottomMSL;
	in >> m_DebrisHeight;
	in >> m_Radius;
}

void AuGeoCrater::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_BottomMSL;
	out << m_DebrisHeight;
	out << m_Radius;
}

void AuGeoCrater::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	AuGeologicalFeature::RandomInit(llworld, dice, existing);

	float altMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()));
	m_BottomMSL = altMSL - dice.Roll(1,100,50) * FeetPerMeter();

	m_Radius = dice.Roll(1,400,100) * FeetPerMeter();
	if(llworld.IsMoonOrPlanetoidSize())
		m_Radius = m_Radius * 2 / 3;	// Smaller craters on moons.

	m_DebrisHeight = dice.Roll(1,m_Radius/5,1);
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoCrater::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, m_BottomMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the crater.
		// Add debris at the rim, dropping off exponentially to the center.
		// Rim is at native terrain MSL + debrisHeight.
		// Center is at bottomMSL, rising exponentially to the debris on top of the rim.
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
		altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL + debrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
	}
	else
	{
		float debrisRad = m_Radius * 1.2f;
		if(dist < debrisRad)
		{
			// Location is in the debris zone of the crater, outside the rim.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float debrisFactor = 1. - (dist - m_Radius) / (debrisRad - m_Radius);	// 1 at the rim, 0 at the outskirts
			float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(debrisFactor));
			altMSL += debrisHeight;
		}
	}

	return altMSL;
}

double AuGeoCrater::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, m_BottomMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the crater.
		// Add debris at the rim, dropping off exponentially to the center.
		// Rim is at native terrain MSL + debrisHeight.
		// Center is at bottomMSL, rising exponentially to the debris on top of the rim.
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
		altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL + debrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
	}
	else
	{
		float debrisRad = m_Radius * 1.2f;
		if(dist < debrisRad)
		{
			// Location is in the debris zone of the crater, outside the rim.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float debrisFactor = 1. - (dist - m_Radius) / (debrisRad - m_Radius);	// 1 at the rim, 0 at the outskirts
			float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(debrisFactor));
			altMSL += debrisHeight;
		}
	}
	return altMSL;
}

double AuGeoCrater::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	AuPoint myloc = llworld.LocOfLL(m_LL, m_BottomMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the crater.
		// Add debris at the rim, dropping off exponentially to the center.
		// Rim is at native terrain MSL + debrisHeight.
		// Center is at bottomMSL, rising exponentially to the debris on top of the rim.
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
		altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL + debrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
	}
	else
	{
		float debrisRad = m_Radius * 1.2f;
		if(dist < debrisRad)
		{
			// Location is in the debris zone of the crater, outside the rim.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float debrisFactor = 1. - (dist - m_Radius) / (debrisRad - m_Radius);	// 1 at the rim, 0 at the outskirts
			float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(debrisFactor));
			altMSL += debrisHeight;
		}
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoCrater::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(worldGenSeed);
	if(m_BottomMSL <= 0 && hliq != HYDRO_None)
	{
		// Radius is 100 to 500m.
		switch(hliq)
		{
		case HYDRO_None:
		case HYDRO_Lava:
			if(m_Radius < 150*FeetPerMeter())
				return QObject::tr("a lava filled crater");
			else if(m_Radius < 350*FeetPerMeter())
				return QObject::tr("a large lava filled crater");
			return QObject::tr("a huge lava filled crater");
		case HYDRO_Water:
			if(m_Radius < 150*FeetPerMeter())
				return QObject::tr("a water filled crater");
			else if(m_Radius < 350*FeetPerMeter())
				return QObject::tr("a large water filled crater");
			return QObject::tr("a huge water filled crater");
		case HYDRO_SupercooledLiquid:
			if(m_Radius < 150*FeetPerMeter())
				return QObject::tr("a crater filled with supercooled liquid");
			else if(m_Radius < 350*FeetPerMeter())
				return QObject::tr("a large crater filled with supercooled liquid");
			return QObject::tr("a huge crater filled with supercooled liquid");
		}
	}
	else
	{
		// Radius is 100 to 500m.
		if(m_Radius < 150*FeetPerMeter())
			return QObject::tr("a crater");
		else if(m_Radius < 350*FeetPerMeter())
			return QObject::tr("a large crater");
		return QObject::tr("a huge crater");
	}
	return QObject::tr("a crater");
}

QString AuGeoCrater::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	if(m_BottomMSL > 0)
		return QObject::tr("Crater");

	switch(hliq)
	{
	case HYDRO_None:
	default:
		return QObject::tr("Crater");
	case HYDRO_Lava:
		return QObject::tr("Lava Filled Crater");
	case HYDRO_Water:
		return QObject::tr("Water Filled Crater");
	case HYDRO_SupercooledLiquid:
		return QObject::tr("Supercooled Liquid Filled Crater");
	}
}

quint64 AuGeoCrater::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoCrater);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoMesa::AuGeoMesa(float radius, float topMSL, const AuGlobeLLF& ll) : AuGeologicalFeature(ll)
{
	m_Radius = radius;
	m_TopMSL = topMSL;
}

AuGeoMesa::AuGeoMesa()
{
	m_Radius = 0;
	m_TopMSL = 0;
}

AuGeoMesa::AuGeoMesa(const AuGeoMesa& c) : AuGeologicalFeature(c)
{
	m_Radius = c.m_Radius;
	m_TopMSL = c.m_TopMSL;
}

bool AuGeoMesa::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoMesa& c = dynamic_cast<const AuGeoMesa&>(b);
		if(m_Radius != c.m_Radius)
			return false;
		if(m_TopMSL != c.m_TopMSL)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoMesa::Clone() const
{
	return new AuGeoMesa(*this);
}

void AuGeoMesa::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_Radius;
	in >> m_TopMSL;
}

void AuGeoMesa::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_Radius;
	out << m_TopMSL;
}

void AuGeoMesa::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	AuGeologicalFeature::RandomInit(llworld, dice, existing);

	float altMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()));
	if(altMSL < llworld.AltitudeIncrement() * 2)
		altMSL = llworld.AltitudeIncrement() * 2;
	m_TopMSL = altMSL + dice.Roll(1,101,-51) * FeetPerMeter();

	m_Radius = dice.Roll(1,200,300) * FeetPerMeter();
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoMesa::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	// Mesa only pulls terrain down, not up.
	if(altMSL <= m_TopMSL)
		return altMSL;

	AuPoint myloc = llworld.LocOfLL(m_LL, m_TopMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_TopMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the mesa radius.
		// Limit peaks to mesa height.
		altMSL = m_TopMSL;
	}
	else
	{
		float transitionZone = m_Radius * 1.2f;
		if(dist < transitionZone)
		{
			// Location is in the transition zone of the mesa, outside the mesa proper.
			float zoneFactor = 1. - (dist - m_Radius) / (transitionZone - m_Radius);	// 1 at the rim, 0 at the outskirts
			altMSL = AuNoiseF::Linear_Interpolate(altMSL, m_TopMSL, AuNoiseF::ExponentialCurve(zoneFactor));
		}
	}
	return altMSL;
}

double AuGeoMesa::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	// Mesa only pulls terrain down, not up.
	if(altMSL <= m_TopMSL)
		return altMSL;

	AuPoint myloc = llworld.LocOfLL(m_LL, m_TopMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_TopMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the mesa radius.
		// Limit peaks to mesa height.
		altMSL = m_TopMSL;
	}
	else
	{
		float transitionZone = m_Radius * 1.2f;
		if(dist < transitionZone)
		{
			// Location is in the transition zone of the mesa, outside the mesa proper.
			float zoneFactor = 1. - (dist - m_Radius) / (transitionZone - m_Radius);	// 1 at the rim, 0 at the outskirts
			altMSL = AuNoiseF::Linear_Interpolate(altMSL, m_TopMSL, AuNoiseF::ExponentialCurve(zoneFactor));
		}
	}
	return altMSL;
}

double AuGeoMesa::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	// Mesa only pulls terrain down, not up.
	if(altMSL <= m_TopMSL)
		return altMSL;

	AuPoint myloc = llworld.LocOfLL(m_LL, m_TopMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_TopMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the mesa radius.
		// Limit peaks to mesa height.
		altMSL = m_TopMSL;
	}
	else
	{
		float transitionZone = m_Radius * 1.2f;
		if(dist < transitionZone)
		{
			// Location is in the transition zone of the mesa, outside the mesa proper.
			float zoneFactor = 1. - (dist - m_Radius) / (transitionZone - m_Radius);	// 1 at the rim, 0 at the outskirts
			altMSL = AuNoiseF::Linear_Interpolate(altMSL, m_TopMSL, AuNoiseF::ExponentialCurve(zoneFactor));
		}
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoMesa::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	// Radius is 300 to 500m.
	if(m_Radius < 400*FeetPerMeter())
		return QObject::tr("the mesas");
	else
		return QObject::tr("the expanse of mesas");
}

QString AuGeoMesa::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	Q_UNUSED(hliq);
	return QObject::tr("Mesas");
}

quint64 AuGeoMesa::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoMesa);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoVolcano::AuGeoVolcano(float radius, float rise, const AuGlobeLLF& ll) : AuGeologicalFeature(ll)
{
	m_Radius = radius;
	m_Rise = rise;
}

AuGeoVolcano::AuGeoVolcano()
{
	m_Radius = 0;
	m_Rise = 0;
}

AuGeoVolcano::AuGeoVolcano(const AuGeoVolcano& c) : AuGeologicalFeature(c)
{
	m_Radius = c.m_Radius;
	m_Rise = c.m_Rise;
}

bool AuGeoVolcano::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoVolcano& c = dynamic_cast<const AuGeoVolcano&>(b);
		if(m_Radius != c.m_Radius)
			return false;
		if(m_Rise != c.m_Rise)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoVolcano::Clone() const
{
	return new AuGeoVolcano(*this);
}

void AuGeoVolcano::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_Radius;
	in >> m_Rise;
}

void AuGeoVolcano::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_Radius;
	out << m_Rise;
}

void AuGeoVolcano::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	AuGeologicalFeature::RandomInit(llworld, dice, existing);

	m_Radius = dice.Roll(1, 300, 100) * FeetPerMeter();

	float altMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()));
	float coneAlt = (AuWeather::LayerAltitude(llworld, AuWeather::LowClouds) +
			AuWeather::LayerAltitude(llworld, AuWeather::MiddleClouds))/2.f;
	coneAlt -= dice.Roll(1, coneAlt/2);
	m_Rise = coneAlt - altMSL;
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoVolcano::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, altMSL);
	AuPoint loc = llworld.LocOfLL(ll, altMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the caldera.
		// Determine the amount of rise based on distance from center.
		float centerAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()))+m_Rise-m_Radius/2.;
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		altMSL = AuNoiseF::Linear_Interpolate(centerAltMSL, altMSL + m_Rise, AuNoiseF::ExponentialCurve(radiusFactor, 3));
	}
	else
	{
		float coneRad = m_Rise + m_Radius;
		if(dist < coneRad)
		{
			// Location is on the slope of the volcano cone.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float coneFactor = (dist - m_Radius) / m_Rise;		// 0 at the rim, 1 at the outskirts
			altMSL += AuNoiseF::Linear_Interpolate(m_Rise, 0, coneFactor);
		}
	}
	return altMSL;
}

double AuGeoVolcano::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, altMSL);
	AuPoint loc = llworld.LocOfLL(ll, altMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the caldera.
		// Determine the amount of rise based on distance from center.
		float centerAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()))+m_Rise-m_Radius/2.;
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		altMSL = AuNoiseF::Linear_Interpolate(centerAltMSL, altMSL + m_Rise, AuNoiseF::ExponentialCurve(radiusFactor, 3));
	}
	else
	{
		float coneRad = m_Rise + m_Radius;
		if(dist < coneRad)
		{
			// Location is on the slope of the volcano cone.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float coneFactor = (dist - m_Radius) / m_Rise;		// 0 at the rim, 1 at the outskirts
			altMSL += AuNoiseF::Linear_Interpolate(m_Rise, 0, coneFactor);
		}
	}
	return altMSL;
}

double AuGeoVolcano::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	AuPoint myloc = llworld.LocOfLL(m_LL, altMSL);
	AuPoint loc = llworld.LocOfLL(ll, altMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the caldera.
		// Determine the amount of rise based on distance from center.
		float centerAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()))+m_Rise-m_Radius/2.;
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		altMSL = AuNoiseF::Linear_Interpolate(centerAltMSL, altMSL + m_Rise, AuNoiseF::ExponentialCurve(radiusFactor, 3));
	}
	else
	{
		float coneRad = m_Rise + m_Radius;
		if(dist < coneRad)
		{
			// Location is on the slope of the volcano cone.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float coneFactor = (dist - m_Radius) / m_Rise;		// 0 at the rim, 1 at the outskirts
			altMSL += AuNoiseF::Linear_Interpolate(m_Rise, 0, coneFactor);
		}
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoVolcano::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	AuDice dice;
	dice.Seed(worldGenSeed + LL().Longitude()*RAD2DEGf + LL().Latitude()*RAD2DEGf*1000 + 10000);
	QString name = dice.RandomGeoName();
	return QObject::tr("the %1 volcano").arg(name);
}

QString AuGeoVolcano::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	Q_UNUSED(hliq);
	return QObject::tr("Volcano");
}

quint64 AuGeoVolcano::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoVolcano);
	return mem;
}

double AuGeoVolcano::SmokeRadius() const
{
	return Radius()/2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoRiver::AuGeoRiver()
{
	m_MouthMSL = 0;
	m_Persistence = 0;
	m_Width = 0;
	m_XOff = 0;
}

AuGeoRiver::AuGeoRiver(const AuGeoRiver& c) : AuGeologicalFeature(c)
{
	m_MouthMSL = c.m_MouthMSL;
	m_Persistence = c.m_Persistence;
	m_SpringLL = c.m_SpringLL;
	m_Width = c.m_Width;
	m_XOff = c.m_XOff;
}

bool AuGeoRiver::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoRiver& c = dynamic_cast<const AuGeoRiver&>(b);
		if(m_MouthMSL != c.m_MouthMSL)
			return false;
		if(m_Persistence != c.m_Persistence)
			return false;
		if(m_SpringLL != c.m_SpringLL)
			return false;
		if(m_Width != c.m_Width)
			return false;
		if(m_XOff != c.m_XOff)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoRiver::Clone() const
{
	return new AuGeoRiver(*this);
}

void AuGeoRiver::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_MouthMSL;
	in >> m_Persistence;
	in >> m_SpringLL;
	in >> m_Width;
	in >> m_XOff;
}

void AuGeoRiver::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_MouthMSL;
	out << m_Persistence;
	out << m_SpringLL;
	out << m_Width;
	out << m_XOff;
}

void AuGeoRiver::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	AuGeologicalFeature::RandomInit(llworld, dice, existing);

	// Choose an altitude for the mouth of the river.
	m_MouthMSL = -dice.Roll(1,90,10) / 100.f * llworld.AltitudeIncrement();		// Pick an altitude that is under water.

	// Perlin noise parameters.
	m_XOff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	m_Persistence = dice.Roll(1,19,80)/100.f;	// .81 <= p <= .99

	// Choose a random width. This is actually half the width since it is treated like a radius.
	m_Width = dice.Roll(1,900,100) * FeetPerMeter();

	// Choose a random spring location.
	int sanity = 1000;
	do
	{
		m_SpringLL = llworld.RandomLandLocation(dice, llworld.AltitudeIncrement()*2);
		if(existing.Find(m_SpringLL))
		{
			dice.Random();
			m_SpringLL = AuGlobeLLF();
		}
	}
	while(m_SpringLL == AuGlobeLLF() && sanity--);

	// Find a location for the mouth of the river on the coast that is at least m_MouthMSL deep.
	int tries = 100;
	bool inWater = false;
	do
	{
		// Choose a random direction.
		float dir = dice.Roll(1,361,-181) * DEG2RADf;
		float cd = cosf(dir);
		float sd = sinf(dir);

		// Walk in that direction up to 1/4 of the globe until we find water.
		float lonPerStep = (PIf/2.f)/100.f * cd;
		float latPerStep = (PIf/4.f)/100.f * sd;
		for(int step=0; step<100; step++)
		{
			// Calculate LL at next step.
			float stepLon = m_SpringLL.Longitude() + step * lonPerStep;
			float stepLat = m_SpringLL.Latitude() + step * latPerStep;
			if(stepLat > PIf/2)
			{
				// Wrap over the north pole.
				stepLat -= PIf/2;
				stepLon += PIf;
			}
			else if(stepLat < -PIf/2)
			{
				// Wrap over the south pole.
				stepLat += PIf/2;
				stepLon += PIf;
			}
			AuGlobeLL stepLL(stepLon, stepLat);
			float stepAltMSL = llworld.GeologicalTerrainAltitudeMSL(stepLL);
			if(stepAltMSL <= m_MouthMSL)
			{
				inWater = true;
				m_LL = AuGlobeLLF(stepLL.Longitude(), stepLL.Latitude());
				break;
			}
		}
	}
	while(!inWater && tries--);
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoRiver::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint loc = llworld.LocOfLL(ll, m_MouthMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_SpringLL, m_LL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the spring of the river.
		if(altMSL > 0)
		{
			// Only pull points down, not up.
			AuPoint springloc = llworld.LocOfLL(m_SpringLL, m_MouthMSL);		// Spring loc at mouth altitude, for consistency.
			float dist = loc.Distance2(springloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the spring of the river to be influenced by it.
				// Spring of river is at MSL altitude. Mouth of the river is always under water (negative MSL).
				// Interpolate between sea level altitude and native terrain altitude.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(0, altMSL, radiusFactor);
			}
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the mouth of the river.
		if(altMSL > m_MouthMSL)
		{
			// Only pull points down, not up.
			AuPoint mouthloc = llworld.LocOfLL(m_LL, m_MouthMSL);					// Mouth location at mouth altitude.
			float dist = loc.Distance2(mouthloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the mouth of the river to be influenced by it.
				dist = sqrtf(dist);
				if(dist < 10*FeetPerMeter())
				{
					// Enforce desired bottom depth within 10m of the river center line.
					altMSL = m_MouthMSL;
				}
				else
				{
					// Interpolate between river base altitude and native terrain altitude.
					float radiusFactor = (dist - 10*FeetPerMeter()) / (m_Width - 10*FeetPerMeter());
					altMSL = AuNoiseF::Cosine_Interpolate(m_MouthMSL, altMSL, radiusFactor);
				}
			}
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_MouthMSL);				// Location of llvalley at mouth altitude.

		// Use noise to offset valleyloc, to make the river wiggle.
		AuPoint mouthloc = llworld.LocOfLL(m_LL, m_MouthMSL);					// Mouth location at mouth altitude.
		AuPoint springloc = llworld.LocOfLL(m_SpringLL, m_MouthMSL);			// Spring loc at mouth altitude, for consistency.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = mouthloc - springloc;
		double lenR = vR.Length();												// River vector length
		vR /= lenR;																// River unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*2.f);
		float ampl = m_Width*2.5f;
		float wiggle = AuNoise::CubicPerlinNoise(2, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to river end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the river floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			// Interpolate the altitude of the valley along the valley segement.
			// Spring of river is at MSL altitude. Mouth of the river is always under water (negative MSL).
			float valleyAltMSL = AuNoiseF::Linear_Interpolate(0, m_MouthMSL, segDot);
			if(altMSL > valleyAltMSL)
			{
				// Valley is deeper than altMSL. Valley only pulls points down, not up.
				valleyDist = sqrtf(valleyDist);
				float enforceDepthRad = 10*FeetPerMeter()*segDot;	// 1 at the mouth, narrows to 0 at the source.
				if(valleyDist < enforceDepthRad)
				{
					// Enforce desired bottom depth within 10m of the river center line.
					altMSL = valleyAltMSL;
				}
				else
				{
					// Interpolate between river base altitude and native terrain altitude.
					float valleyFactor = (valleyDist - enforceDepthRad) / (m_Width - enforceDepthRad);
					altMSL = AuNoiseF::Cosine_Interpolate(valleyAltMSL, altMSL, valleyFactor);
				}
			}
		}
	}
	return altMSL;
}

double AuGeoRiver::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint loc = llworld.LocOfLL(ll, m_MouthMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_SpringLL, m_LL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the spring of the river.
		if(altMSL > 0)
		{
			// Only pull points down, not up.
			AuPoint springloc = llworld.LocOfLL(m_SpringLL, m_MouthMSL);		// Spring loc at mouth altitude, for consistency.
			float dist = loc.Distance2(springloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the spring of the river to be influenced by it.
				// Spring of river is at MSL altitude. Mouth of the river is always under water (negative MSL).
				// Interpolate between sea level altitude and native terrain altitude.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(0, altMSL, radiusFactor);
			}
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the mouth of the river.
		if(altMSL > m_MouthMSL)
		{
			// Only pull points down, not up.
			AuPoint mouthloc = llworld.LocOfLL(m_LL, m_MouthMSL);					// Mouth location at mouth altitude.
			float dist = loc.Distance2(mouthloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the mouth of the river to be influenced by it.
				dist = sqrtf(dist);
				if(dist < 10*FeetPerMeter())
				{
					// Enforce desired bottom depth within 10m of the river center line.
					altMSL = m_MouthMSL;
				}
				else
				{
					// Interpolate between river base altitude and native terrain altitude.
					float radiusFactor = (dist - 10*FeetPerMeter()) / (m_Width - 10*FeetPerMeter());
					altMSL = AuNoiseF::Cosine_Interpolate(m_MouthMSL, altMSL, radiusFactor);
				}
			}
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_MouthMSL);				// Location of llvalley at mouth altitude.

		// Use noise to offset valleyloc, to make the river wiggle.
		AuPoint mouthloc = llworld.LocOfLL(m_LL, m_MouthMSL);					// Mouth location at mouth altitude.
		AuPoint springloc = llworld.LocOfLL(m_SpringLL, m_MouthMSL);			// Spring loc at mouth altitude, for consistency.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = mouthloc - springloc;
		double lenR = vR.Length();												// River vector length
		vR /= lenR;																// River unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*2.f);
		float ampl = m_Width*2.5f;
		float wiggle = AuNoise::CubicPerlinNoise(2, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to river end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the river floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			// Interpolate the altitude of the valley along the valley segement.
			// Spring of river is at MSL altitude. Mouth of the river is always under water (negative MSL).
			float valleyAltMSL = AuNoiseF::Linear_Interpolate(0, m_MouthMSL, segDot);
			if(altMSL > valleyAltMSL)
			{
				// Valley is deeper than altMSL. Valley only pulls points down, not up.
				valleyDist = sqrtf(valleyDist);
				float enforceDepthRad = 10*FeetPerMeter()*segDot;	// 1 at the mouth, narrows to 0 at the source.
				if(valleyDist < enforceDepthRad)
				{
					// Enforce desired bottom depth within 10m of the river center line.
					altMSL = valleyAltMSL;
				}
				else
				{
					// Interpolate between river base altitude and native terrain altitude.
					float valleyFactor = (valleyDist - enforceDepthRad) / (m_Width - enforceDepthRad);
					altMSL = AuNoiseF::Cosine_Interpolate(valleyAltMSL, altMSL, valleyFactor);
				}
			}
		}
	}
	return altMSL;
}

double AuGeoRiver::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	AuPoint loc = llworld.LocOfLL(ll, m_MouthMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_SpringLL, m_LL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the spring of the river.
		if(altMSL > 0)
		{
			// Only pull points down, not up.
			AuPoint springloc = llworld.LocOfLL(m_SpringLL, m_MouthMSL);		// Spring loc at mouth altitude, for consistency.
			float dist = loc.Distance2(springloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the spring of the river to be influenced by it.
				// Spring of river is at MSL altitude. Mouth of the river is always under water (negative MSL).
				// Interpolate between sea level altitude and native terrain altitude.
				dist = sqrtf(dist);
				float radiusFactor = dist / m_Width;
				altMSL = AuNoiseF::Cosine_Interpolate(0, altMSL, radiusFactor);
			}
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the mouth of the river.
		if(altMSL > m_MouthMSL)
		{
			// Only pull points down, not up.
			AuPoint mouthloc = llworld.LocOfLL(m_LL, m_MouthMSL);					// Mouth location at mouth altitude.
			float dist = loc.Distance2(mouthloc);
			if(dist < m_Width*m_Width)
			{
				// ll location is close enough to the mouth of the river to be influenced by it.
				dist = sqrtf(dist);
				if(dist < 10*FeetPerMeter())
				{
					// Enforce desired bottom depth within 10m of the river center line.
					altMSL = m_MouthMSL;
				}
				else
				{
					// Interpolate between river base altitude and native terrain altitude.
					float radiusFactor = (dist - 10*FeetPerMeter()) / (m_Width - 10*FeetPerMeter());
					altMSL = AuNoiseF::Cosine_Interpolate(m_MouthMSL, altMSL, radiusFactor);
				}
			}
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_MouthMSL);				// Location of llvalley at mouth altitude.

		// Use noise to offset valleyloc, to make the river wiggle.
		AuPoint mouthloc = llworld.LocOfLL(m_LL, m_MouthMSL);					// Mouth location at mouth altitude.
		AuPoint springloc = llworld.LocOfLL(m_SpringLL, m_MouthMSL);			// Spring loc at mouth altitude, for consistency.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = mouthloc - springloc;
		double lenR = vR.Length();												// River vector length
		vR /= lenR;																// River unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*2.f);
		float ampl = m_Width*2.5f;
		float wiggle = AuNoise::CubicPerlinNoise(2, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to river end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the river floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			// Interpolate the altitude of the valley along the valley segement.
			// Spring of river is at MSL altitude. Mouth of the river is always under water (negative MSL).
			float valleyAltMSL = AuNoiseF::Linear_Interpolate(0, m_MouthMSL, segDot);
			if(altMSL > valleyAltMSL)
			{
				// Valley is deeper than altMSL. Valley only pulls points down, not up.
				valleyDist = sqrtf(valleyDist);
				float enforceDepthRad = 10*FeetPerMeter()*segDot;	// 1 at the mouth, narrows to 0 at the source.
				if(valleyDist < enforceDepthRad)
				{
					// Enforce desired bottom depth within 10m of the river center line.
					altMSL = valleyAltMSL;
				}
				else
				{
					// Interpolate between river base altitude and native terrain altitude.
					float valleyFactor = (valleyDist - enforceDepthRad) / (m_Width - enforceDepthRad);
					altMSL = AuNoiseF::Cosine_Interpolate(valleyAltMSL, altMSL, valleyFactor);
				}
			}
		}
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoRiver::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	AuDice dice;
	dice.Seed(worldGenSeed + LL().Longitude()*RAD2DEGf + LL().Latitude()*RAD2DEGf*1000 + 20000);
	QString name = dice.RandomGeoName();

	// Width/radius is 100m to 1000m.
	if(m_Width < 200*FeetPerMeter())
		return QObject::tr("the mouth of %1, a narrow river").arg(name);
	else if(m_Width < 500*FeetPerMeter())
		return QObject::tr("the mouth of the %1 river").arg(name);
	else if(m_Width < 750*FeetPerMeter())
		return QObject::tr("the mouth of the wide river %1").arg(name);
	else
		return QObject::tr("the mouth of %1, a mighty river").arg(name);
}

QString AuGeoRiver::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	switch(hliq)
	{
	case HYDRO_None:
		return QObject::tr("Dry River Bed");
	case HYDRO_Lava:
		return QObject::tr("Lava Flow");
	case HYDRO_Water:
	default:
		return QObject::tr("River");
	case HYDRO_SupercooledLiquid:
		return QObject::tr("River of Supercooled Liquid");
	}
}

quint64 AuGeoRiver::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoRiver);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoChasm::AuGeoChasm()
{
	m_BottomMSL = 0;
	m_Persistence = 0;
	m_Width = 0;
	m_XOff = 0;
}

AuGeoChasm::AuGeoChasm(const AuGeoChasm& c) : AuGeologicalFeature(c)
{
	m_BottomMSL = c.m_BottomMSL;
	m_EndLL = c.m_EndLL;
	m_Persistence = c.m_Persistence;
	m_Width = c.m_Width;
	m_XOff = c.m_XOff;
}

bool AuGeoChasm::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoChasm& c = dynamic_cast<const AuGeoChasm&>(b);
		if(m_BottomMSL != c.m_BottomMSL)
			return false;
		if(m_EndLL != c.m_EndLL)
			return false;
		if(m_Persistence != c.m_Persistence)
			return false;
		if(m_Width != c.m_Width)
			return false;
		if(m_XOff != c.m_XOff)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoChasm::Clone() const
{
	return new AuGeoChasm(*this);
}

void AuGeoChasm::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_BottomMSL;
	in >> m_EndLL;
	in >> m_Persistence;
	in >> m_Width;
	in >> m_XOff;
}

void AuGeoChasm::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_BottomMSL;
	out << m_EndLL;
	out << m_Persistence;
	out << m_Width;
	out << m_XOff;
}

void AuGeoChasm::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	AuGeologicalFeature::RandomInit(llworld, dice, existing);

	// Choose an altitude for the mouth of the Chasm.
	m_BottomMSL = dice.Roll(1,90,10) / 100.f * llworld.AltitudeIncrement();		// Pick an altitude that is above water.

	// Perlin noise parameters.
	m_XOff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	m_Persistence = dice.Roll(1,38,60)/100.f;	// .61 <= p <= .98

	// Choose a random width. This is actually half the width since it is treated like a radius.
	m_Width = dice.Roll(1,90,10) * FeetPerMeter();

	// Choose a random start location.
	float minDirtMSL = m_BottomMSL + 10*FeetPerMeter();
	int sanity = 1000;
	do
	{
		m_LL = llworld.RandomLandLocation(dice, minDirtMSL);	// At least 10m dirt height above chasm floor at chasm start.
		if(existing.Find(m_LL))
		{
			dice.Random();
			m_LL = AuGlobeLLF();
		}
	}
	while(m_LL == AuGlobeLLF() && sanity--);
	dice.Random();

	// Find a location for the end of the chasm that is shallower than the desired bottom depth.
	int tries = 100;
	bool brokeOut = false;
	do
	{
		// Choose a random direction.
		float dir = dice.Roll(1,361,-181) * DEG2RADf;
		float cd = cosf(dir);
		float sd = sinf(dir);

		// Walk in that direction up to 1/4 of the globe until we find shallower dirt.
		float lonPerStep = (PIf/2.f)/100.f * cd;
		float latPerStep = (PIf/4.f)/100.f * sd;
		for(int step=0; step<100; step++)
		{
			// Calculate LL at next step.
			float stepLon = m_LL.Longitude() + step * lonPerStep;
			float stepLat = m_LL.Latitude() + step * latPerStep;
			if(stepLat > PIf/2)
			{
				// Wrap over the north pole.
				stepLat -= PIf/2;
				stepLon += PIf;
			}
			else if(stepLat < -PIf/2)
			{
				// Wrap over the south pole.
				stepLat += PIf/2;
				stepLon += PIf;
			}
			AuGlobeLL stepLL(stepLon, stepLat);
			float stepAltMSL = llworld.GeologicalTerrainAltitudeMSL(stepLL);
			if(stepAltMSL <= minDirtMSL)
			{
				// Stepped to a place where the dirt is shallow enough to break out.
				brokeOut = true;
				m_EndLL = AuGlobeLLF(stepLL.Longitude(), stepLL.Latitude());
				break;
			}
		}
	}
	while(!brokeOut && tries--);
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoChasm::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	// Chasm only pulls points down, not up.
	if(altMSL < m_BottomMSL)
		return altMSL;

	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the start of the Chasm.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		float dist = loc.Distance2(startloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the start of the Chasm to be influenced by it.
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the spring of the Chasm.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the spring of the Chasm to be influenced by it.
			// Spring of Chasm is at MSL altitude. Mouth of the Chasm is always under water (negative MSL).
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_BottomMSL);				// Location of llvalley at bottom altitude.

		// Use noise to offset valleyloc, to make the chasm wiggle.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = endloc - startloc;
		double lenR = vR.Length();												// Chasm vector length
		vR /= lenR;																// Chasm unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*5.f);
		float ampl = m_Width*10.f;
		float wiggle = AuNoise::CubicPerlinNoise(1, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to chasm end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the chasm floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			valleyDist = sqrtf(valleyDist);

			// Interpolate between chasm base altitude and native terrain altitude.
			float valleyFactor = valleyDist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(valleyFactor));
		}
	}
	return altMSL;
}

double AuGeoChasm::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	// Chasm only pulls points down, not up.
	if(altMSL < m_BottomMSL)
		return altMSL;

	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the start of the Chasm.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		float dist = loc.Distance2(startloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the start of the Chasm to be influenced by it.
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the spring of the Chasm.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the spring of the Chasm to be influenced by it.
			// Spring of Chasm is at MSL altitude. Mouth of the Chasm is always under water (negative MSL).
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_BottomMSL);				// Location of llvalley at bottom altitude.

		// Use noise to offset valleyloc, to make the chasm wiggle.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = endloc - startloc;
		double lenR = vR.Length();												// Chasm vector length
		vR /= lenR;																// Chasm unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*5.f);
		float ampl = m_Width*10.f;
		float wiggle = AuNoise::CubicPerlinNoise(1, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to chasm end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the chasm floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			valleyDist = sqrtf(valleyDist);

			// Interpolate between chasm base altitude and native terrain altitude.
			float valleyFactor = valleyDist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(valleyFactor));
		}
	}
	return altMSL;
}

double AuGeoChasm::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	// Chasm only pulls points down, not up.
	if(altMSL < m_BottomMSL)
		return altMSL;

	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the start of the Chasm.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		float dist = loc.Distance2(startloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the start of the Chasm to be influenced by it.
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the spring of the Chasm.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the spring of the Chasm to be influenced by it.
			// Spring of Chasm is at MSL altitude. Mouth of the Chasm is always under water (negative MSL).
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_BottomMSL);				// Location of llvalley at bottom altitude.

		// Use noise to offset valleyloc, to make the chasm wiggle.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = endloc - startloc;
		double lenR = vR.Length();												// Chasm vector length
		vR /= lenR;																// Chasm unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*5.f);
		float ampl = m_Width*10.f;
		float wiggle = AuNoise::CubicPerlinNoise(1, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to chasm end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the chasm floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			valleyDist = sqrtf(valleyDist);

			// Interpolate between chasm base altitude and native terrain altitude.
			float valleyFactor = valleyDist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(valleyFactor));
		}
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoChasm::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);

	// Width/radius is 10m to 100m.
	if(m_Width < 20*FeetPerMeter())
		return QObject::tr("the entrance to a narrow chasm");
	else if(m_Width < 75*FeetPerMeter())
		return QObject::tr("the entrance to a chasm");
	else
		return QObject::tr("the entrance to a wide chasm");
}

QString AuGeoChasm::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	Q_UNUSED(hliq);
	return QObject::tr("Chasm");
}

quint64 AuGeoChasm::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoChasm);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoHenge::AuGeoHenge(const AuId& destSysId, quint8 destSun, quint8 destShadowPanels, quint8 destArcSegment, float topMSL,
			const AuGlobeLLF& ll) : AuGeologicalFeature(ll)
{
	m_DestArcSegment = destArcSegment;
	m_DestShadowPanels = destShadowPanels;
	m_DestSun = destSun;
	m_DestSysId = destSysId;
	m_TopMSL = topMSL;

	if(m_DestShadowPanels == 0)
		qWarning() << "error: AuGeoHenge::AuGeoHenge destSys" << destSysId << "sun" << destSun << "panels" << destShadowPanels
			<< "arc" << destArcSegment << "topMSL" << topMSL << "ll" << ll;
}

AuGeoHenge::AuGeoHenge()
{
	m_DestArcSegment = 0;
	m_DestShadowPanels = 0;
	m_DestSun = 0;
	m_TopMSL = 0;
}

AuGeoHenge::AuGeoHenge(const AuGeoHenge& c) : AuGeologicalFeature(c)
{
	m_Active = c.m_Active;
	m_DestArcSegment = c.m_DestArcSegment;
	m_DestShadowPanels = c.m_DestShadowPanels;
	m_DestSun = c.m_DestSun;
	m_DestSysId = c.m_DestSysId;
	m_TopMSL = c.m_TopMSL;
}

bool AuGeoHenge::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoHenge& c = dynamic_cast<const AuGeoHenge&>(b);
		if(m_Active != c.m_Active)
			return false;
		if(m_DestArcSegment != c.m_DestArcSegment)
			return false;
		if(m_DestShadowPanels != c.m_DestShadowPanels)
			return false;
		if(m_DestSun != c.m_DestSun)
			return false;
		if(m_DestSysId != c.m_DestSysId)
			return false;
		if(m_TopMSL != c.m_TopMSL)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoHenge::Clone() const
{
	return new AuGeoHenge(*this);
}

void AuGeoHenge::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);

	quint8 ui8 = m_DestSun;	// Dest sun value range is 0-2 so it only takes 2 bits.
	if(IsActive())
		ui8 |= 0x08;
	out << ui8;

	out << m_DestArcSegment;
	out << m_DestShadowPanels;
	out << m_DestSysId;
	out << m_TopMSL;
	if(ui8 & 0x08)
		out << m_Active;
}

void AuGeoHenge::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);

	quint8 ui8;
	in >> ui8;
	m_DestSun = ui8 & 0x03;

	in >> m_DestArcSegment;
	in >> m_DestShadowPanels;
	in >> m_DestSysId;
	in >> m_TopMSL;
	if(ui8 & 0x08)
	{
		AuTime t;
		in >> t;
		if(g_Au.IsSceneSolo())
			m_Active = t;		// Trust the stored time in solo play.
		else
			m_Active.Set();		// When on line, active time value is just reset or we have problems due to clocks out of sync.
	}
	else
		m_Active = AuTime();	// Definitely off.
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoHenge::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, m_TopMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_TopMSL);
	float dist = loc.Distance(myloc);
	float radius = TerrainRadius();
	if(dist < radius)
	{
		// Location is inside the henge radius.
		// Level the terrain here.
		altMSL = m_TopMSL;

		// Put some smooth dirt here so caller knows to remove trees and shrubs, but don't override current pavement.
		hideRocks = true;
		hideGrass = 2;
		hideShrubs = true;
		hideTrees = true;
	}
	else
	{
		float transitionZone = radius * 1.1f;
		if(dist < transitionZone)
		{
			// Location is in the transition zone of the mesa, outside the mesa proper.
			float zoneFactor = 1. - (dist - radius) / (transitionZone - radius);	// 1 at the rim, 0 at the outskirts
			altMSL = AuNoiseF::Linear_Interpolate(altMSL, m_TopMSL, AuNoiseF::ExponentialCurve(zoneFactor));
		}
	}
	return altMSL;
}

double AuGeoHenge::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, m_TopMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_TopMSL);
	float dist = loc.Distance(myloc);
	float radius = TerrainRadius();
	if(dist < radius)
	{
		// Location is inside the henge radius.
		// Level the terrain here.
		altMSL = m_TopMSL;
	}
	else
	{
		float transitionZone = radius * 1.1f;
		if(dist < transitionZone)
		{
			// Location is in the transition zone of the mesa, outside the mesa proper.
			float zoneFactor = 1. - (dist - radius) / (transitionZone - radius);	// 1 at the rim, 0 at the outskirts
			altMSL = AuNoiseF::Linear_Interpolate(altMSL, m_TopMSL, AuNoiseF::ExponentialCurve(zoneFactor));
		}
	}
	return altMSL;
}

double AuGeoHenge::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	AuPoint myloc = llworld.LocOfLL(m_LL, m_TopMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_TopMSL);
	float dist = loc.Distance(myloc);
	float radius = TerrainRadius();
	if(dist < radius)
	{
		// Location is inside the henge radius.
		// Level the terrain here.
		altMSL = m_TopMSL;
	}
	else
	{
		float transitionZone = radius * 1.1f;
		if(dist < transitionZone)
		{
			// Location is in the transition zone of the mesa, outside the mesa proper.
			float zoneFactor = 1. - (dist - radius) / (transitionZone - radius);	// 1 at the rim, 0 at the outskirts
			altMSL = AuNoiseF::Linear_Interpolate(altMSL, m_TopMSL, AuNoiseF::ExponentialCurve(zoneFactor));
		}
	}
	return altMSL;
}

void AuGeoHenge::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	// Choose a random location that is at least 1m above water.
	// Most geological features are not very interesting under water.
	AuDice dice(Cdice);
	int sanity = 1000;
	do
	{
		m_LL = llworld.RandomLandLocationNotForbidden(dice, FeetPerMeter(), TerrainRadius());
		if(existing.Find(m_LL))
		{
			dice.Random();
			m_LL = AuGlobeLLF();
		}
	}
	while(m_LL == AuGlobeLLF() && sanity--);
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoHenge::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	AuDice dice;
	dice.Seed(worldGenSeed + LL().Longitude()*RAD2DEGf + LL().Latitude()*RAD2DEGf*1000 + 30000);
	QString name = dice.RandomGeoName();

	return QObject::tr("the %1, a sacred place for reflection and prayer").arg(name);
}

quint64 AuGeoHenge::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoHenge);
	return mem;
}

ON_GROUND AuGeoHenge::Ground() const
{
	return OG_Rock;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoVent::AuGeoVent(float radius, float rise, const AuGlobeLLF& ll) : AuGeologicalFeature(ll)
{
	m_Radius = radius;
	m_Rise = rise;
}

AuGeoVent::AuGeoVent()
{
	m_Radius = 0;
	m_Rise = 0;
}

AuGeoVent::AuGeoVent(const AuGeoVent& c) : AuGeologicalFeature(c)
{
	m_Radius = c.m_Radius;
	m_Rise = c.m_Rise;
}

bool AuGeoVent::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoVent& c = dynamic_cast<const AuGeoVent&>(b);
		if(m_Radius != c.m_Radius)
			return false;
		if(m_Rise != c.m_Rise)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoVent::Clone() const
{
	return new AuGeoVent(*this);
}

void AuGeoVent::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_Radius;
	in >> m_Rise;
}

void AuGeoVent::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_Radius;
	out << m_Rise;
}

void AuGeoVent::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	int sanity = 1000;
	do
	{
		m_LL = llworld.RandomLL(dice);
		if(existing.Find(m_LL))
		{
			dice.Random();
			m_LL = AuGlobeLLF();
		}
	}
	while(m_LL == AuGlobeLLF() && sanity--);
	dice.Random();

	m_Radius = dice.Roll(1, 20) * FeetPerMeter();

	float altMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()));
	float coneAlt = altMSL + dice.Roll(2,6) * FeetPerMeter();
	m_Rise = coneAlt - altMSL;
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoVent::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, altMSL);
	AuPoint loc = llworld.LocOfLL(ll, altMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the vent.
		// Determine the amount of rise based on distance from center.
		float centerAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()))+m_Rise-m_Radius/2.;
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		altMSL = AuNoiseF::Linear_Interpolate(centerAltMSL, altMSL + m_Rise, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		hideGrass = 1;
		hideShrubs = true;
		hideTrees = true;
	}
	else
	{
		float coneRad = m_Rise + m_Radius;
		if(dist < coneRad)
		{
			// Location is on the slope of the vent cone.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float coneFactor = (dist - m_Radius) / m_Rise;		// 0 at the rim, 1 at the outskirts
			altMSL += AuNoiseF::Linear_Interpolate(m_Rise, 0, coneFactor);
		}
	}
	return altMSL;
}

double AuGeoVent::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	AuPoint myloc = llworld.LocOfLL(m_LL, altMSL);
	AuPoint loc = llworld.LocOfLL(ll, altMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the vent.
		// Determine the amount of rise based on distance from center.
		float centerAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()))+m_Rise-m_Radius/2.;
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		altMSL = AuNoiseF::Linear_Interpolate(centerAltMSL, altMSL + m_Rise, AuNoiseF::ExponentialCurve(radiusFactor, 3));
	}
	else
	{
		float coneRad = m_Rise + m_Radius;
		if(dist < coneRad)
		{
			// Location is on the slope of the vent cone.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float coneFactor = (dist - m_Radius) / m_Rise;		// 0 at the rim, 1 at the outskirts
			altMSL += AuNoiseF::Linear_Interpolate(m_Rise, 0, coneFactor);
		}
	}
	return altMSL;
}

double AuGeoVent::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	AuPoint myloc = llworld.LocOfLL(m_LL, altMSL);
	AuPoint loc = llworld.LocOfLL(ll, altMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the vent.
		// Determine the amount of rise based on distance from center.
		float centerAltMSL = llworld.GeologicalTerrainAltitudeMSL(AuGlobeLL(m_LL.Longitude(), m_LL.Latitude()))+m_Rise-m_Radius/2.;
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		altMSL = AuNoiseF::Linear_Interpolate(centerAltMSL, altMSL + m_Rise, AuNoiseF::ExponentialCurve(radiusFactor, 3));
	}
	else
	{
		float coneRad = m_Rise + m_Radius;
		if(dist < coneRad)
		{
			// Location is on the slope of the vent cone.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float coneFactor = (dist - m_Radius) / m_Rise;		// 0 at the rim, 1 at the outskirts
			altMSL += AuNoiseF::Linear_Interpolate(m_Rise, 0, coneFactor);
		}
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoVent::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	AuDice dice;
	dice.Seed(worldGenSeed + LL().Longitude()*RAD2DEGf + LL().Latitude()*RAD2DEGf*1000 + 10000);
	QString name = dice.RandomGeoName();
	return QObject::tr("the %1 vent").arg(name);
}

QString AuGeoVent::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	Q_UNUSED(hliq);
	return QObject::tr("Geothermal Vent");
}

quint64 AuGeoVent::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoVent);
	return mem;
}

double AuGeoVent::SmokeRadius() const
{
	return Radius()/2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoFissure::AuGeoFissure()
{
	m_BottomMSL = 0;
	m_Persistence = 0;
	m_Width = 0;
	m_XOff = 0;
}

AuGeoFissure::AuGeoFissure(const AuGeoFissure& c) : AuGeologicalFeature(c)
{
	m_BottomMSL = c.m_BottomMSL;
	m_EndLL = c.m_EndLL;
	m_Persistence = c.m_Persistence;
	m_Width = c.m_Width;
	m_XOff = c.m_XOff;
}

bool AuGeoFissure::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoFissure& c = dynamic_cast<const AuGeoFissure&>(b);
		if(m_BottomMSL != c.m_BottomMSL)
			return false;
		if(m_EndLL != c.m_EndLL)
			return false;
		if(m_Persistence != c.m_Persistence)
			return false;
		if(m_Width != c.m_Width)
			return false;
		if(m_XOff != c.m_XOff)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoFissure::Clone() const
{
	return new AuGeoFissure(*this);
}

void AuGeoFissure::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	in >> m_BottomMSL;
	in >> m_EndLL;
	in >> m_Persistence;
	in >> m_Width;
	in >> m_XOff;
}

void AuGeoFissure::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	out << m_BottomMSL;
	out << m_EndLL;
	out << m_Persistence;
	out << m_Width;
	out << m_XOff;
}

void AuGeoFissure::RandomInit(const DaWorldLL& llworld, const AuDice& Cdice, const AuGeologicalFeatures& existing)
{
	AuDice dice(Cdice);
	int sanity = 1000;
	do
	{
		m_LL = llworld.RandomLL(dice);
		if(existing.Find(m_LL))
		{
			dice.Random();
			m_LL = AuGlobeLLF();
		}
	}
	while(m_LL == AuGlobeLLF() && sanity--);
	dice.Random();

	// Choose an altitude for the mouth of the Fissure.
	double lipMSL = llworld.GlobeRadius() - llworld.SeaLevelAltitude();	// For gas giants.
	m_BottomMSL = lipMSL - llworld.GlobeRadius() * dice.Roll(1,81,9) / 100;	// Altitude -.1 to -.9 of globe radius.

	// Perlin noise parameters.
	m_XOff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	m_Persistence = dice.Roll(1,38,50)/100.f;	// .51 <= p <= .88

	// Choose a random width. This is actually half the width since it is treated like a radius.
	m_Width = dice.Roll(1,36,4) * FeetPerMeter();	// 5 to 40 m across

	// Choose random start and end locations.
	sanity = 1000;
	do
	{
		m_EndLL = llworld.RandomLL(dice);
		if(existing.Find(m_EndLL))
		{
			dice.Random();
			m_EndLL = AuGlobeLLF();
		}
	}
	while(m_EndLL == AuGlobeLLF() && sanity--);
}

// Adjusts 'altMSL' at 'll' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoFissure::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	// Fissure only pulls points down, not up.
	if(altMSL < m_BottomMSL)
		return altMSL;

	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the start of the Fissure.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		float dist = loc.Distance2(startloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the start of the Fissure to be influenced by it.
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the spring of the Fissure.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the spring of the Fissure to be influenced by it.
			// Spring of Fissure is at MSL altitude. Mouth of the Fissure is always under water (negative MSL).
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_BottomMSL);				// Location of llvalley at bottom altitude.

		// Use noise to offset valleyloc, to make the chasm wiggle.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = endloc - startloc;
		double lenR = vR.Length();												// Fissure vector length
		vR /= lenR;																// Fissure unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*5.f);
		float ampl = m_Width*10.f;
		float wiggle = AuNoise::CubicPerlinNoise(1, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to chasm end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the chasm floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			valleyDist = sqrtf(valleyDist);

			// Interpolate between chasm base altitude and native terrain altitude.
			float valleyFactor = valleyDist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(valleyFactor));
		}
	}
	return altMSL;
}

double AuGeoFissure::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	// Fissure only pulls points down, not up.
	if(altMSL < m_BottomMSL)
		return altMSL;

	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the start of the Fissure.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		float dist = loc.Distance2(startloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the start of the Fissure to be influenced by it.
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the spring of the Fissure.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the spring of the Fissure to be influenced by it.
			// Spring of Fissure is at MSL altitude. Mouth of the Fissure is always under water (negative MSL).
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_BottomMSL);				// Location of llvalley at bottom altitude.

		// Use noise to offset valleyloc, to make the chasm wiggle.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = endloc - startloc;
		double lenR = vR.Length();												// Fissure vector length
		vR /= lenR;																// Fissure unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*5.f);
		float ampl = m_Width*10.f;
		float wiggle = AuNoise::CubicPerlinNoise(1, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to chasm end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the chasm floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			valleyDist = sqrtf(valleyDist);

			// Interpolate between chasm base altitude and native terrain altitude.
			float valleyFactor = valleyDist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(valleyFactor));
		}
	}
	return altMSL;
}

double AuGeoFissure::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& ll, double altMSL) const
{
	// Fissure only pulls points down, not up.
	if(altMSL < m_BottomMSL)
		return altMSL;

	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);								// Location of 'll' at start altitude.

	float segDot;
	AuGlobeLLF llvalley = AuGlobeLLF(ll.Longitude(), ll.Latitude()).CoordOnSegment(m_LL, m_EndLL, &segDot, llworld.LLShape());
	if(segDot <= 0)
	{
		// ll is beyond the start of the Fissure.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		float dist = loc.Distance2(startloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the start of the Fissure to be influenced by it.
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else if(segDot >= 1)
	{
		// ll is beyond the spring of the Fissure.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		float dist = loc.Distance2(endloc);
		if(dist < m_Width*m_Width)
		{
			// ll location is close enough to the spring of the Fissure to be influenced by it.
			// Spring of Fissure is at MSL altitude. Mouth of the Fissure is always under water (negative MSL).
			dist = sqrtf(dist);
			float radiusFactor = dist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(radiusFactor, 3));
		}
	}
	else
	{
		// ll falls in the valley. It drops perpendicular to the segment from m_LL to m_SpringLL between the end points.
		AuPoint valleyloc = llworld.LocOfLL(llvalley, m_BottomMSL);				// Location of llvalley at bottom altitude.

		// Use noise to offset valleyloc, to make the chasm wiggle.
		AuPoint startloc = llworld.LocOfLL(m_LL, m_BottomMSL);					// Start location at bottom altitude.
		AuPoint endloc = llworld.LocOfLL(m_EndLL, m_BottomMSL);					// End loc at bottom altitude.
		AuPoint vG = llworld.GravityDir(valleyloc);
		AuPoint vR = endloc - startloc;
		double lenR = vR.Length();												// Fissure vector length
		vR /= lenR;																// Fissure unit vector
		AuPoint vP = vG.Cross(vR);												// Unit vector perpendicular to the river's course.
		vP.Unit();
		double freq = lenR / (m_Width*5.f);
		float ampl = m_Width*10.f;
		float wiggle = AuNoise::CubicPerlinNoise(1, m_Persistence, m_XOff + segDot * freq) * ampl;

		// Wiggle must converge to chasm end points at ends.
		if(segDot < .5f)
			wiggle *= segDot;
		else
			wiggle *= 1.f - segDot;
		valleyloc += vP * wiggle;

		float valleyDist = loc.Distance2(valleyloc);							// Distance from loc to the chasm floor segment.
		if(valleyDist < m_Width*m_Width)
		{
			// ll location is close enough to the valley floor to be influenced by it.
			valleyDist = sqrtf(valleyDist);

			// Interpolate between chasm base altitude and native terrain altitude.
			float valleyFactor = valleyDist / m_Width;
			altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL, AuNoiseF::ExponentialCurve(valleyFactor));
		}
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoFissure::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);

	// Width/radius is 10m to 100m.
	if(m_Width < 10*FeetPerMeter())
		return QObject::tr("the beginning of a narrow fissure");
	else if(m_Width < 25*FeetPerMeter())
		return QObject::tr("the beginning of a fissure");
	else
		return QObject::tr("the beginning of a wide fissure");
}

QString AuGeoFissure::TransporterName(HYDROGRAPHIC_LIQUID hliq) const
{
	Q_UNUSED(hliq);
	return QObject::tr("Fissure");
}

quint64 AuGeoFissure::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoFissure);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoRoadGrade::AuGeoRoadGrade(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width,
		bool slab, const DeConstructionProcess* cp) :
	AuGeologicalFeature(endLL)
{
	m_EndMSL = endMSL;
	if(cp)
		m_pConstruct = new DeConstructionProcess(*cp);
	else
		m_pConstruct = 0;
	m_Slab = slab;
	m_StartLL = startLL;
	m_StartMSL = startMSL;
	m_TimeCreated.Set();
	m_Width = width;
}

AuGeoRoadGrade::AuGeoRoadGrade()
{
	m_EndMSL = 0;
	m_pConstruct = 0;
	m_Slab = false;
	m_StartMSL = 0;
	m_Width = 0;
}

AuGeoRoadGrade::AuGeoRoadGrade(const AuGeoRoadGrade& c) : AuGeologicalFeature(c)
{
	m_EndMSL = c.m_EndMSL;
	if(c.m_pConstruct)
		m_pConstruct = new DeConstructionProcess(*c.m_pConstruct);
	else
		m_pConstruct = 0;
	m_Slab = c.m_Slab;
	m_StartLL = c.m_StartLL;
	m_StartMSL = c.m_StartMSL;
	m_TimeCreated = c.m_TimeCreated;
	m_Width = c.m_Width;
}

bool AuGeoRoadGrade::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoRoadGrade& c = dynamic_cast<const AuGeoRoadGrade&>(b);
		if(m_TimeCreated != c.m_TimeCreated)
			return false;
		if(m_EndMSL != c.m_EndMSL)
			return false;
		if(m_pConstruct && c.m_pConstruct)
		{
			// Both under construction.
			if(*m_pConstruct != *c.m_pConstruct)
				return false;
		}
		else if(IsUnderConstruction() != c.IsUnderConstruction())
			return false;
		if(m_Slab != c.m_Slab)
			return false;
		if(m_StartLL != c.m_StartLL)
			return false;
		if(m_StartMSL != c.m_StartMSL)
			return false;
		if(m_Width != c.m_Width)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoRoadGrade::Clone() const
{
	return new AuGeoRoadGrade(*this);
}

void AuGeoRoadGrade::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	quint8 hasWhat = 0;
	if(m_Slab)
		hasWhat |= 0x01;
	if(!m_TimeCreated.IsNull())
		hasWhat |= 0x40;
	if(m_pConstruct)
		hasWhat |= 0x80;
	out << hasWhat;
	out << m_EndMSL;
	out << m_StartLL;
	out << m_StartMSL;
	out << m_Width;
	if(hasWhat & 0x40)
		out << m_TimeCreated;
	if(hasWhat & 0x80)
		*m_pConstruct >> out;
}

void AuGeoRoadGrade::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	m_Slab = hasWhat & 0x01;
	in >> m_EndMSL;
	in >> m_StartLL;
	in >> m_StartMSL;
	in >> m_Width;
	if(hasWhat & 0x40)
		in >> m_TimeCreated;
	if(hasWhat & 0x80)
	{
		if(!m_pConstruct)
			m_pConstruct = new DeConstructionProcess();
		*m_pConstruct << in;
	}
	else
	{
		if(m_pConstruct)
		{
			delete m_pConstruct;
			m_pConstruct = 0;
		}
	}
}

// Adjusts 'altMSL' at 'adjLL' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoRoadGrade::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, bool& hideRocks,
		int& hideGrass, bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	// Graded segment
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideGrass);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(newest);
	// Calculate the closest point on this road segment to 'adjLL'.
	float segFrac;
	AuGlobeLLF tll(adjLL.ToLLF());
	AuGlobeLLF segLL = tll.CoordOnSegment(m_StartLL, m_LL, &segFrac, llworld.LLShape());
	float segAltMSL;
	AuPoint segLoc;
	if(segFrac <= 0)
	{
		// Closest point is the starting location.
		segAltMSL = m_StartMSL;
		segLoc = llworld.LocOfLL(m_StartLL, segAltMSL);
	}
	else if(segFrac >= 1)
	{
		// Closest point is the ending location.
		segAltMSL = m_EndMSL;
		segLoc = llworld.LocOfLL(m_LL, segAltMSL);
	}
	else
	{
		// Calculate the location along this road segment.
		segAltMSL = m_StartMSL + (m_EndMSL - m_StartMSL) * segFrac;
		segLoc = llworld.LocOfLL(segLL, segAltMSL);
	}

	// Calculate the location of adjLL at the altitude of the segment location.
	AuPoint adjLoc = llworld.LocOfLL(adjLL, segAltMSL);

	// Determine if the search location is within the build radius of the development, at the altitude of the development.
	float dist2 = (float)adjLoc.Distance2(segLoc);
	float bldRad = m_Width/2 + 8;

	if(segFrac > 0 && segFrac < 1)
	{
		// The point is between my end points.
		float bldRad2 = bldRad * bldRad;						// squared
		if(dist2 <= bldRad2)
		{
			// The point is on this road segment, between the end points. This is the zone in which my pavement coverage is 100%.
			if(onRoadOrGrader)
			{
				// This is not the first road to contain the point. Calculate the average of all roads at the location.
				if(onRoadOrGrader > 1)
					altMSL *= onRoadOrGrader;	// Multiply the current road count back out to restore the accumulated total alt.
				altMSL += segAltMSL;			// Add my altitude to the accumulated total.
				onRoadOrGrader++;				// Bump the number of roads.
				segAltMSL = altMSL / onRoadOrGrader;	// Average MSL altitude of all roads that were hit, including this one.
			}
			else
				onRoadOrGrader++;				// This is the first road to contain the location.
			//hideRocks = true;
			//hideGrass = 1;
			//hideShrubs = true;
			//hideTrees = true;
			if(IsUnderConstruction())
				pavementType = WORLDTEX_ConstructionZone;
			//if(newest < m_TimeCreated)
				//newest = m_TimeCreated;
			return segAltMSL;
		}

		// The point is outside my pavement zone. Don't do anything with it if it's in somebody else's pavement zone.
		if(onRoadOrGrader)
			return altMSL;

		// Determine if the search location is within 10m of my build radius.
		// My influence on the altitude is blended with the terrain in that zone.
		float xRad = bldRad + (float)(10*FeetPerMeter());
		float xRad2 = xRad*xRad;								// squared
		if(dist2 >= xRad2)
			return altMSL;				// Location is outside the transition zone. altMSL is not affected by me.

		// adjLL is in the transition zone between the end points. Blend from my altitude to the incoming altitude.
		//if(newest < m_TimeCreated)
			//newest = m_TimeCreated;
		// Blend starts at the build radius away, which is the edge of the road. Transition zone is 10m wide.
		float dist = sqrtf(dist2);								// Dreaded square root.
		float blend = (dist - bldRad)/(float)(10*FeetPerMeter());		// Ratio of my altitude vs incoming altMSL.
		return AuNoiseF::Cosine_Interpolate(segAltMSL, altMSL, blend);
	}
	else
	{
		// Point is not between my end points.
		// The point is outside my pavement zone. Don't do anything with it if it's in somebody else's pavement zone.
		if(onRoadOrGrader)
			return altMSL;

		// Determine if the search location is within 10m of my build radius.
		// My influence on the altitude is blended with the terrain in that zone.
		float xRad = bldRad + (float)(10*FeetPerMeter());
		float xRad2 = xRad*xRad;								// squared
		if(dist2 >= xRad2)
			return altMSL;				// Location is outside the transition zone. altMSL is not affected by me.

		// adjLL is in the transition zone beyond one end of the road segment. Blend from my altitude to the incoming altitude.
		//if(newest < m_TimeCreated)
			//newest = m_TimeCreated;
		// Blend starts at the end of the road and blends out to the build radius plus the transition zone.
		float dist = sqrtf(dist2);								// Dreaded square root.
		float blend = dist/(bldRad + (float)(10*FeetPerMeter()));		// Ratio of my altitude vs incoming altMSL.
		return AuNoiseF::Cosine_Interpolate(segAltMSL, altMSL, blend);
	}
}

double AuGeoRoadGrade::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(newest);
	// Graded segment
	// Calculate the closest point on this road segment to 'adjLL'.
	float segFrac;
	AuGlobeLLF tll(adjLL.ToLLF());
	AuGlobeLLF segLL = tll.CoordOnSegment(m_StartLL, m_LL, &segFrac, llworld.LLShape());
	float segAltMSL;
	AuPoint segLoc;
	if(segFrac <= 0)
	{
		// Closest point is the starting location.
		segAltMSL = m_StartMSL;
		segLoc = llworld.LocOfLL(m_StartLL, segAltMSL);
	}
	else if(segFrac >= 1)
	{
		// Closest point is the ending location.
		segAltMSL = m_EndMSL;
		segLoc = llworld.LocOfLL(m_LL, segAltMSL);
	}
	else
	{
		// Calculate the location along this road segment.
		segAltMSL = m_StartMSL + (m_EndMSL - m_StartMSL) * segFrac;
		segLoc = llworld.LocOfLL(segLL, segAltMSL);
	}

	// Calculate the location of adjLL at the altitude of the segment location.
	AuPoint adjLoc = llworld.LocOfLL(adjLL, segAltMSL);

	// Determine if the search location is within the build radius of the development, at the altitude of the development.
	float dist2 = (float)adjLoc.Distance2(segLoc);
	float bldRad = m_Width/2 + 8;

	if(segFrac > 0 && segFrac < 1)
	{
		// The point is between my end points.
		float bldRad2 = bldRad * bldRad;						// squared
		if(dist2 <= bldRad2)
		{
			// The point is on this road segment, between the end points. This is the zone in which my pavement coverage is 100%.
			if(onRoadOrGrader)
			{
				// This is not the first road to contain the point. Calculate the average of all roads at the location.
				if(onRoadOrGrader > 1)
					altMSL *= onRoadOrGrader;	// Multiply the current road count back out to restore the accumulated total alt.
				altMSL += segAltMSL;			// Add my altitude to the accumulated total.
				onRoadOrGrader++;				// Bump the number of roads.
				segAltMSL = altMSL / onRoadOrGrader;	// Average MSL altitude of all roads that were hit, including this one.
			}
			else
				onRoadOrGrader++;				// This is the first road to contain the location.
			if(IsUnderConstruction())
				pavementType = WORLDTEX_ConstructionZone;
			//if(newest < m_TimeCreated)
				//newest = m_TimeCreated;
			return segAltMSL;
		}

		// The point is outside my pavement zone. Don't do anything with it if it's in somebody else's pavement zone.
		if(onRoadOrGrader)
			return altMSL;

		// Determine if the search location is within 10m of my build radius.
		// My influence on the altitude is blended with the terrain in that zone.
		float xRad = bldRad + (float)(10*FeetPerMeter());
		float xRad2 = xRad*xRad;								// squared
		if(dist2 >= xRad2)
			return altMSL;				// Location is outside the transition zone. altMSL is not affected by me.

		// adjLL is in the transition zone between the end points. Blend from my altitude to the incoming altitude.
		//if(newest < m_TimeCreated)
			//newest = m_TimeCreated;
		// Blend starts at the build radius away, which is the edge of the road. Transition zone is 10m wide.
		float dist = sqrtf(dist2);								// Dreaded square root.
		float blend = (dist - bldRad)/(float)(10*FeetPerMeter());		// Ratio of my altitude vs incoming altMSL.
		return AuNoiseF::Cosine_Interpolate(segAltMSL, altMSL, blend);
	}
	else
	{
		// Point is not between my end points.
		// The point is outside my pavement zone. Don't do anything with it if it's in somebody else's pavement zone.
		if(onRoadOrGrader)
			return altMSL;

		// Determine if the search location is within 10m of my build radius.
		// My influence on the altitude is blended with the terrain in that zone.
		float xRad = bldRad + (float)(10*FeetPerMeter());
		float xRad2 = xRad*xRad;								// squared
		if(dist2 >= xRad2)
			return altMSL;				// Location is outside the transition zone. altMSL is not affected by me.

		// adjLL is in the transition zone beyond one end of the road segment. Blend from my altitude to the incoming altitude.
		//if(newest < m_TimeCreated)
			//newest = m_TimeCreated;
		// Blend starts at the end of the road and blends out to the build radius plus the transition zone.
		float dist = sqrtf(dist2);								// Dreaded square root.
		float blend = dist/(bldRad + (float)(10*FeetPerMeter()));		// Ratio of my altitude vs incoming altMSL.
		return AuNoiseF::Cosine_Interpolate(segAltMSL, altMSL, blend);
	}
}

double AuGeoRoadGrade::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL) const
{
	Q_UNUSED(llworld);
	Q_UNUSED(adjLL);
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoRoadGrade::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	if(IsUnderConstruction())
		return QObject::tr("a strip of graded land under construction");
	else
		return QObject::tr("a strip of graded land");
}

void AuGeoRoadGrade::RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		AuGLEffects& re, const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	// Use simple shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * model);
	re.glDisable(GL_DEPTH_TEST);
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);							// Additive blend keeps it nice and bright.
	RenderCursor_Shader(world, re, nearLoc, hitRoad, hitColor, roadColor);
	re.glDisable(GL_BLEND);
	re.glEnable(GL_DEPTH_TEST);
	re.GLSL().LineShader().Deselect();
}

void AuGeoRoadGrade::RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
		const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	AuPoint startLoc = world.LocOfLL(m_StartLL, m_StartMSL);
	AuPoint endLoc = world.LocOfLL(m_LL, m_EndMSL);
	if(nearLoc.DistToSegment(startLoc, endLoc) > 1000)
		return;

	// Draw a linear cursor from the start location to the end location.
	// Right/left designations are in the context of standing at startLoc and looking down the road segment to endLoc.
	QColor color;
	if(hitRoad == LL())
		color = hitColor;
	else
		color = roadColor;

	float bldRad = m_Width/2;
	AuPoint centerVec = endLoc - startLoc;
	double centerLen = centerVec.Length();
	Q_ASSERT(centerLen != 0);
	centerVec /= centerLen;

	// Build road edge lines and center lines.
	// Start with the starting locations.
	AuPointArray roadCenter;
	AuPointArray roadLeft;
	AuPointArray roadRight;
	AuPointArray roadLegs;	// Lines across the road to make a grid.

	// Integrate along the length so we conform to the curvature of the world.
	double seglen = 10*FeetPerMeter();
	int segs = (int)(centerLen / seglen) + 1;			// Do one too many for perfect ending.
	bool atEnd = false;
	for(int seg=0; seg<=segs && !atEnd; seg++)
	{
		double lenAlongSeg = seglen * seg;
		if(lenAlongSeg >= centerLen)
		{
			// We are at the end. Truncate the last segment for perfect ending.
			//qDebug() << "segment" << lenAlongSeg << "clipped to" << centerLen << "seg" << seg << "segs" << segs;
			lenAlongSeg = centerLen;				
			atEnd = true;
		}
		double segRatio = lenAlongSeg / centerLen;
		double altAtSegLoc = m_StartMSL + (m_EndMSL - m_StartMSL) * segRatio;	// Linear alt change from start to end.
		AuPoint segLoc = startLoc + centerVec * lenAlongSeg;
		AuPoint segUp = -world.GravityDir(segLoc);
		world.ElevateLocMSL(segLoc, altAtSegLoc);

		AuPoint segRight = centerVec.Cross(segUp);
		segRight.Unit();

		// Add road level points.
		roadCenter.append(segLoc);
		roadLeft.append(segLoc - segRight*bldRad);
		roadRight.append(segLoc + segRight*bldRad);

		// Add pairs to draw them as GL_LINES.
		roadLegs.append(roadLeft.last());
		roadLegs.append(roadCenter.last());
		roadLegs.append(roadCenter.last());
		roadLegs.append(roadRight.last());
	}

	// Draw lines.
	re.GLSL().LineShader().SetColor(color);

	// Left
	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForLines(re, roadLeft, false, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	// Center
	/*
	if(IsSlab())
	{
		// \ on landscape slabs.
		// Compile a list of vertex locations.
		AuPointArray verts;
		verts.append(roadRight.first());
		verts.append(roadLeft.first());
		verts.append(roadRight.last());
		verts.append(roadLeft.last());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}
	else
	*/
	{
		// Center line on landscapers.
		// Compile a list of vertex locations.
		AuPointArray verts;
		verts.append(roadRight.first());
		verts.append(roadLeft.first());
		for(AuPointArray::const_iterator ri = roadCenter.constBegin(); ri != roadCenter.constEnd(); ri++)
			verts.append(*ri);
		verts.append(roadRight.last());
		verts.append(roadLeft.last());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Right
	// Make a VAO of the vertex locations.
	vao.CreateForLines(re, roadRight, false, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}
}

bool AuGeoRoadGrade::IsDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	return AuRoadSegment::Contains(llworld, ll, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width/2, radius);
}

INDUSTRYID AuGeoRoadGrade::GeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	if(IsUnderConstruction() && AuRoadSegment::Contains(llworld, ll, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width/2, radius))
		return IID_Road;
	return IID_None;
}

bool AuGeoRoadGrade::ConstructionProcess(DeConstructionProcess& cp) const
{
	if(!m_pConstruct)
		return false;
	cp = *m_pConstruct;
	return true;
}

bool AuGeoRoadGrade::ConstructionEnable(bool enable)
{
	if(!m_pConstruct)
		return false;
	if(enable)
	{
		if(m_pConstruct->IsHalted())
			return false;	// no change
		m_pConstruct->Resume();
	}
	else
	{
		if(!m_pConstruct->IsHalted())
			return false;	// no change
		m_pConstruct->Halt();
	}
	return true;
}

bool AuGeoRoadGrade::ConstructionFetch(const DaWorldLL& llworld)
{
	if(!m_pConstruct)
		return false;
	AuZoneResource res;
	llworld.QueryNaturalResources(res, llworld.ResourceZone(LL().Longitude()));
	quint8 stoneQ = res.m_StoneQ;
	if(!stoneQ)
		stoneQ = 1;		// Minimal quality if it isn't really present.
	quint8 oilQ = res.m_CrudeOilQ;
	if(!oilQ)
		oilQ = 1;		// Minimal quality if it isn't really present.
	AuItemHash inventory, tools;
	inventory.AddItem(AuItem(CID_Stone, stoneQ, 0, 1));
	inventory.AddItem(AuItem(CID_CrudeOil, oilQ, 0, 1));
	return m_pConstruct->FetchMaterials(inventory, tools);
}

int AuGeoRoadGrade::ConstructionGo(AuGear& gear)
{
	if(!m_pConstruct)
		return 0;						// I am not under construction.
	if(m_pConstruct->LaborNeeded())
	{
		// Verify presence of required items before wearing down any tools.
		bool reqMissing = false;
		for(DeProcessComponentArray::iterator cc = m_pConstruct->Components().begin(); cc != m_pConstruct->Components().end(); ++cc)
		{
			if((*cc).IsRequiredModifier())
			{
				// This is a required component that is not consumed.
				if((*cc).IsSatisfied())
					continue;					// Requirement has already been satisfied.

				// Attempt to satisfy using the gear.
				if(!gear.FindReadyItem((*cc).CommodityId()).IsValid())
				{
					reqMissing = true;	// Required component is not present.
					break;
				}
			}
		}
		if(reqMissing)
			return 0x01;					// Still under construction. Some required item is missing.

		int ilabor = 4;						// Standard amount of labor per effort.

		// Set the satisfied state of requirements that don't consume anything, based on current availability of tools, etc.
		// This helps to eliminate a step when the only requirement might be a tool and the unit is equipped with one.
		// Check to see if the person applying the effort has any objects that will add to the labor applied.
		int lLaborBoost = 0;
		for(DeProcessComponentArray::iterator cc = m_pConstruct->Components().begin(); cc != m_pConstruct->Components().end(); ++cc)
		{
			if((*cc).Effect() == CE_DecreasesTime)
			{
				// This component affects labor rate.
				AuGearPos pos = gear.FindReadyItem((*cc).CommodityId());
				if(pos.IsValid())
				{
					const AuItem* item = gear.ItemAt(pos);
					if(item)
					{
						int boost = item->GetLaborBoost();
						if(boost)
						{
							(*cc).SetQtyApplied(1);
							lLaborBoost += boost;
						}
					}
				}
			}
			else if((*cc).IsRequiredModifier())
			{
				// This is a required component that is not consumed.
				if((*cc).IsSatisfied())
					continue;					// Requirement has already been satisfied.

				// Check the inventory of the unit to determine if any equipped item satisfies the requirement.
				if(gear.FindReadyItem((*cc).CommodityId()).IsValid())
					(*cc).SetQtyApplied(1);
			}
		}
		ilabor += lLaborBoost;
		m_pConstruct->ApplyConstructionLabor(ilabor);
	}
	if(m_pConstruct->IsComplete())
	{
		delete m_pConstruct;
		m_pConstruct = 0;
		return 0x02;	// Construction complete.
	}
	return 0x01;	// Still under construction.
}

float AuGeoRoadGrade::AlignmentRot(const DaWorldLL& llworld, const AuGlobeLLF& alignLL) const
{
	// Calculate sea level location of 'alignLL'.
	AuPoint alignLoc = llworld.LocOfLL(alignLL);

	// Calculate sea level location of drop location on this road segment.
	AuPoint startLoc = llworld.LocOfLL(LL());
	AuPoint endLoc = llworld.LocOfLL(LL2());

	AuPoint dropLoc = alignLoc.PointOnSegment(startLoc, endLoc);
	AuGlobeLLF dropLL;
	llworld.LLOfLoc(dropLL, dropLoc);
	dropLoc = llworld.LocOfLL(dropLL);

	// Calculate vector from alignLoc to dropLoc.
	AuPoint dropVec = dropLoc - alignLoc;
	dropVec.Unit();

	// Calculate X vector with zero Z rotation.
	AuPoint vUp = -llworld.GravityDir(alignLoc);
	AuMatrix4x4 mBld;
	mBld *= AuPoint(0,0,1).GetAlignmentTransform(vUp);

	// Convert the dropVec into a vector relative to the building matrix.
	dropVec = mBld.UnProject(dropVec);
	return dropVec.ZRotation() - PIf/2;
}

bool AuGeoRoadGrade::IsConstructionWorkerEnabled() const
{
	return m_pConstruct && !m_pConstruct->IsHalted();
}

quint64 AuGeoRoadGrade::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoRoadGrade);
	if(m_pConstruct)
		mem += m_pConstruct->MemoryUsage();
	return mem;
}

AuPoint AuGeoRoadGrade::Center(const DaWorldLL& world) const
{
	AuPoint startLoc = world.LocOfLL(LL(), LLMSL());
	AuPoint endLoc = world.LocOfLL(LL2(), LL2MSL());
	AuPoint cenLoc((startLoc + endLoc)/2);
	double cenMSL = (LLMSL() + LL2MSL())/2;
	world.ElevateLocMSL(cenLoc, cenMSL);
	return cenLoc;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoRoadDirt::AuGeoRoadDirt(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width,
		bool slab, const DeConstructionProcess* cp) :
	AuGeoRoadGrade(startLL, startMSL, endLL, endMSL, width, slab, cp)
{
}

AuGeoRoadDirt::AuGeoRoadDirt()
{
}

AuGeoRoadDirt::AuGeoRoadDirt(const AuGeoRoadDirt& c) : AuGeoRoadGrade(c)
{
}

bool AuGeoRoadDirt::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoRoadDirt& c = dynamic_cast<const AuGeoRoadDirt&>(b);
		return AuGeoRoadGrade::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoRoadDirt::Clone() const
{
	return new AuGeoRoadDirt(*this);
}

void AuGeoRoadDirt::operator<<(QDataStream& in)
{
	AuGeoRoadGrade::operator<<(in);
}

void AuGeoRoadDirt::operator>>(QDataStream& out) const
{
	AuGeoRoadGrade::operator>>(out);
}

// Adjusts 'altMSL' at 'adjLL' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoRoadDirt::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, bool& hideRocks, int& hideGrass,
		bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	int org = onRoadOrGrader;
	altMSL = AuGeoRoadGrade::AdjustAltMSL(llworld, adjLL, altMSL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType,
			onRoadOrGrader, newest);
	if(onRoadOrGrader > org)
	{
		// The location is on this road surface.
		hideRocks = true;
		hideGrass = 1;
		hideShrubs = true;
		hideTrees = true;
		if(IsUnderConstruction())
			pavementType = WORLDTEX_ConstructionZone;
		else
			pavementType = WORLDTEX_DirtSmooth;	// Looks like bare dirt. This works for the bed under paved roads too.
	}
	return altMSL;
}

double AuGeoRoadDirt::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	int org = onRoadOrGrader;
	altMSL = AuGeoRoadGrade::AdjustAltMSL(llworld, adjLL, altMSL, pavementType, onRoadOrGrader, newest);
	if(onRoadOrGrader > org)
	{
		// The location is on this road surface.
		if(IsUnderConstruction())
			pavementType = WORLDTEX_ConstructionZone;
		else
			pavementType = WORLDTEX_DirtSmooth;	// Looks like bare dirt. This works for the bed under paved roads too.
	}
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoRoadDirt::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	if(IsUnderConstruction())
		return QObject::tr("a dirt road under construction");
	else
		return QObject::tr("a dirt road");
}

void AuGeoRoadDirt::RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
		const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	AuPoint startLoc = world.LocOfLL(m_StartLL, m_StartMSL);
	AuPoint endLoc = world.LocOfLL(m_LL, m_EndMSL);
	if(nearLoc.DistToSegment(startLoc, endLoc) > 1000)
		return;

	// Draw a linear cursor from the start location to the end location.
	// Right/left designations are in the context of standing at startLoc and looking down the road segment to endLoc.
	QColor color;
	if(hitRoad == LL())
		color = hitColor;
	else
		color = roadColor;

	float bldRad = m_Width/2;
	AuPoint centerVec = endLoc - startLoc;
	double centerLen = centerVec.Length();
	Q_ASSERT(centerLen != 0);
	centerVec /= centerLen;

	// Build road edge lines and center lines.
	// Start with the starting locations.
	AuPointArray roadCenter;
	AuPointArray roadLeft;
	AuPointArray roadRight;
	AuPointArray roadLegs;	// Lines across the road to make a grid.

	// Integrate along the length so we conform to the curvature of the world.
	double seglen = 10*FeetPerMeter();
	int segs = (int)(centerLen / seglen) + 1;			// Do one too many for perfect ending.
	bool atEnd = false;
	for(int seg=0; seg<=segs && !atEnd; seg++)
	{
		double lenAlongSeg = seglen * seg;
		if(lenAlongSeg >= centerLen)
		{
			// We are at the end. Truncate the last segment for perfect ending.
			//qDebug() << "segment" << lenAlongSeg << "clipped to" << centerLen << "seg" << seg << "segs" << segs;
			lenAlongSeg = centerLen;				
			atEnd = true;
		}
		double segRatio = lenAlongSeg / centerLen;
		double altAtSegLoc = m_StartMSL + (m_EndMSL - m_StartMSL) * segRatio;	// Linear alt change from start to end.
		AuPoint segLoc = startLoc + centerVec * lenAlongSeg;
		AuPoint segUp = -world.GravityDir(segLoc);
		world.ElevateLocMSL(segLoc, altAtSegLoc);

		AuPoint segRight = centerVec.Cross(segUp);
		segRight.Unit();

		// Add road level points.
		roadCenter.append(segLoc);
		roadLeft.append(segLoc - segRight*bldRad);
		roadRight.append(segLoc + segRight*bldRad);

		// Add pairs to draw them as GL_LINES.
		roadLegs.append(roadLeft.last());
		roadLegs.append(roadCenter.last());
		roadLegs.append(roadCenter.last());
		roadLegs.append(roadRight.last());
	}

	// Draw lines.
	re.GLSL().LineShader().SetColor(color);

	// Left
	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForLines(re, roadLeft, false, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	// Center
	/*
	if(IsSlab())
	{
		// X on slabs.
		// Compile a list of vertex locations.
		AuPointArray verts;
		verts.append(roadRight.first());
		verts.append(roadLeft.last());
		verts.append(roadRight.last());
		verts.append(roadLeft.first());
		verts.append(roadRight.first());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}
	else
	*/
	{
		// Grid on road strips.
		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, roadCenter, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// Make a VAO of the vertex locations.
		vao.CreateForLines(re, roadLegs, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Right
	// Make a VAO of the vertex locations.
	vao.CreateForLines(re, roadRight, false, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}
}

void AuGeoRoadDirt::CreatePavement(const DaWorldLL& world, AuScFaceList& faces, quint8 texIndex) const
{
	// Right/left designations are in the context of standing at startLoc and looking down the road segment to endLoc.
	AuGlobeLLF startLL = LL2();
	double startAlt = LL2MSL();
	AuPoint startLoc = world.LocOfLL(startLL, startAlt);
	AuGlobeLLF endLL = LL();
	double endAlt = LLMSL();
	AuPoint endLoc = world.LocOfLL(endLL, endAlt);
	float bldRad = Radius();

	AuPoint centerVec = endLoc - startLoc;
	double centerLen = centerVec.Length();
	Q_ASSERT(centerLen != 0);
	centerVec /= centerLen;

	// Integrate along the length so we conform to the curvature of the world.
	/*
	if(IsSlab())
	{
		double seglen = 10;
		int segs = (int)(centerLen / seglen) + 1;			// Do one too many for perfect ending.
		bool atEnd = false;
		AuPoint prevRightLoc, prevLeftLoc;
		for(int seg=0; seg<=segs && !atEnd; seg++)
		{
			double lenAlongSeg = seglen * seg;
			if(lenAlongSeg >= centerLen)
			{
				// We are at the end. Truncate the last segment for perfect ending.
				//qDebug() << "segment" << lenAlongSeg << "clipped to" << centerLen << "seg" << seg << "segs" << segs;
				lenAlongSeg = centerLen;				
				atEnd = true;
			}
			double segRatio = lenAlongSeg / centerLen;
			double altAtSegLoc = startAlt + (endAlt - startAlt) * segRatio;	// Linear alt change from start to end.
			AuPoint segLoc = startLoc + centerVec * lenAlongSeg;
			AuPoint segUp = -world.GravityDir(segLoc);
			world.ElevateLocMSL(segLoc, altAtSegLoc);

			AuPoint segRight = centerVec.Cross(segUp);
			segRight.Unit();

			AuPoint rightLoc = segLoc + segRight * bldRad;
			AuPoint leftLoc = segLoc - segRight * bldRad;
			if(seg && !IsUnderConstruction())
				CreateSlabModelSection(world, faces, prevLeftLoc, leftLoc, rightLoc, prevRightLoc, seg==1, atEnd);

			prevRightLoc = rightLoc;
			prevLeftLoc = leftLoc;
		}
	}
	else
	*/
	{
		double seglen = 10;
		int segs = (int)(centerLen / seglen) + 1;			// Do one too many for perfect ending.
		bool atEnd = false;
		AuPoint prevRightLoc, prevLeftLoc;
		for(int seg=0; seg<=segs && !atEnd; seg++)
		{
			double lenAlongSeg = seglen * seg;
			if(lenAlongSeg >= centerLen)
			{
				// We are at the end. Truncate the last segment for perfect ending.
				//qDebug() << "segment" << lenAlongSeg << "clipped to" << centerLen << "seg" << seg << "segs" << segs;
				lenAlongSeg = centerLen;				
				atEnd = true;
			}
			double segRatio = lenAlongSeg / centerLen;
			double altAtSegLoc = startAlt + (endAlt - startAlt) * segRatio;	// Linear alt change from start to end.
			AuPoint segLoc = startLoc + centerVec * lenAlongSeg;
			AuPoint segUp = -world.GravityDir(segLoc);
			world.ElevateLocMSL(segLoc, altAtSegLoc);

			AuPoint segRight = centerVec.Cross(segUp);
			segRight.Unit();

			AuPoint rightLoc = segLoc + segRight * bldRad;
			AuPoint leftLoc = segLoc - segRight * bldRad;
			if(seg && !IsUnderConstruction())
				CreateRoadModelSection(world, faces, prevLeftLoc, leftLoc, rightLoc, prevRightLoc, seg==1, atEnd);

			prevRightLoc = rightLoc;
			prevLeftLoc = leftLoc;
		}
	}

	// Texture the result.
	AuScSelectInfo selInfo;
	faces.FaceIds(selInfo.m_Faces);
	faces.TextureUnwrap(selInfo, texIndex, 2, AuScFaceList::UNWRAP_ByFacing, PIf/4, false, 0, AuMatrix4x4());
}

void AuGeoRoadDirt::CreateRoadModelSection(const DaWorldLL& world, AuScFaceList& faces, const AuPoint& prevLeft,
		const AuPoint& nextLeft, const AuPoint& nextRight, const AuPoint& prevRight, bool prevCap, bool nextCap) const
{
	// Use paint for striping.
	QColor noPaint(0,0,0,0);
	QColor whitePaint(255,255,255,200);
	QColor yellowPaint(243,187,64,200);

	// Calculate surface vectors.
	AuPoint vl = nextLeft - prevLeft;							// unit vector along left side
	vl.Unit();
	AuPoint vr = nextLeft - prevLeft;							// unit vector along right side
	vr.Unit();
	AuPoint plz = -world.GravityDir(prevLeft);					// up vector at prevLeft
	AuPoint nlz = -world.GravityDir(nextLeft);					// up vector at nextLeft
	AuPoint nrz = -world.GravityDir(nextRight);				// up vector at nextRight
	AuPoint prz = -world.GravityDir(prevRight);				// up vector at prevRight
	AuPoint nl = plz.Cross(vl);									// unit vector normal to the left side wall
	nl.Unit();
	AuPoint nr = vr.Cross(prz);									// unit vector normal to the right side wall
	nr.Unit();

	// Stripes.
	double stripeWidth = .5;									// Google says stripes are 6" wide.
	AuPoint pls = prevLeft - nl * stripeWidth;					// Left side stripe
	AuPoint nls = nextLeft - nl * stripeWidth;					// Left side stripe
	AuPoint nrs = nextRight - nr * stripeWidth;					// Right side stripe
	AuPoint prs = prevRight - nr * stripeWidth;					// Right side stripe
	AuPoint pnc = (plz + prz)/2;								// Unit normal at the prev centerline.
	pnc.Unit();
	AuPoint nnc = (nlz + nrz)/2;								// Unit normal at the next centerline.
	nnc.Unit();
	AuPoint pmid = (pls + prs)/2 + pnc * 1.5/12.;				// Middle of road surface at prev end.
	AuPoint nmid = (nls + nrs)/2 + nnc * 1.5/12.;				// Middle of road surface at next end.
	AuPoint plc = pmid + nl * stripeWidth;						// Left of wide centerline stripe
	AuPoint nlc = nmid + nl * stripeWidth;						// Left of wide centerline stripe
	AuPoint nrc = nmid + nr * stripeWidth;						// Right of wide centerline stripe
	AuPoint prc = pmid + nr * stripeWidth;						// Right of wide centerline stripe

	// Bottom points of slab.
	AuPoint plb = prevLeft - plz;
	AuPoint nlb = nextLeft - nlz;
	AuPoint nrb = nextRight - nrz;
	AuPoint prb = prevRight - prz;
	AuPoint plsb = pls - plz;
	AuPoint nlsb = nls - nlz;
	AuPoint nrsb = nrs - nlz;
	AuPoint prsb = prs - plz;
	AuPoint plcb = plc - pnc;
	AuPoint nlcb = nlc - nnc;
	AuPoint nrcb = nrc - nnc;
	AuPoint prcb = prc - pnc;

	// Surface with striping.
	CreateModelSectionQuad(faces, pls, nls, nlc, plc, plz, nlz, nnc, pnc, noPaint);	// Left side pavement.
	CreateModelSectionQuad(faces, prc, nrc, nrs, prs, pnc, nnc, nrz, prz, noPaint);	// Right side pavement.
	CreateModelSectionQuad(faces, plc, nlc, nrc, prc, pnc, nnc, nnc, pnc, yellowPaint);	// Center line stripe.
	CreateModelSectionQuad(faces, prevLeft, nextLeft, nls, pls, plz, nlz, nlz, plz, whitePaint);	// Left side stripe
	CreateModelSectionQuad(faces, prs, nrs, nextRight, prevRight, prz, nrz, nrz, prz, whitePaint);	// Right side stripe

	// Underside.
	CreateModelSectionQuad(faces, prb, nrb, nrsb, prsb, -plz, -nrz, -nnc, -pnc, noPaint);
	CreateModelSectionQuad(faces, prsb, nrsb, nrcb, prcb, -pnc, -nnc, -nnc, -pnc, noPaint);
	CreateModelSectionQuad(faces, prcb, nrcb, nlcb, plcb, -pnc, -nnc, -nnc, -pnc, noPaint);
	CreateModelSectionQuad(faces, plcb, nlcb, nlsb, plsb, -pnc, -nnc, -nnc, -pnc, noPaint);
	CreateModelSectionQuad(faces, plsb, nlsb, nlb, plb, -pnc, -nnc, -nlz, -plz, noPaint);

	// Side walls.
	CreateModelSectionQuad(faces, nlb, nextLeft, prevLeft, plb, nl, nl, nl, nl, noPaint);	// Left side pavement.
	CreateModelSectionQuad(faces, prevRight, nextRight, nrb, prb, nr, nr, nr, nr, noPaint);	// Right side pavement.

	if(prevCap)
	{
		// End cap at prev end.
		CreateModelSectionQuad(faces, prevLeft, pls, plsb, plb, -vl, -vl, -vl, -vl, noPaint);	// Left stripe
		CreateModelSectionQuad(faces, pls, plc, plcb, plsb, -vl, -vl, -vl, -vl, noPaint);	// Left pavement
		CreateModelSectionQuad(faces, plc, prc, prcb, plcb, -vl, -vr, -vr, -vl, noPaint);	// Center stripe
		CreateModelSectionQuad(faces, prc, prs, prsb, prcb, -vr, -vr, -vr, -vr, noPaint);	// Right pavement
		CreateModelSectionQuad(faces, prs, prevRight, prb, prsb, -vr, -vr, -vr, -vr, noPaint);	// Right stripe
	}

	if(nextCap)
	{
		// End cap at next end.
		CreateModelSectionQuad(faces, nls, nextLeft, nlb, nlsb, vl, vl, vl, vl, noPaint);	// Left stripe
		CreateModelSectionQuad(faces, nlc, nls, nlsb, nlcb, vl, vl, vl, vl, noPaint);	// Left pavement
		CreateModelSectionQuad(faces, nrc, nlc, nlcb, nrcb, vl, vr, vr, vl, noPaint);	// Center stripe
		CreateModelSectionQuad(faces, nrs, nrc, nrcb, nrsb, vr, vr, vr, vr, noPaint);	// Right pavement
		CreateModelSectionQuad(faces, nextRight, nrs, nrsb, nrb, vr, vr, vr, vr, noPaint);	// Right stripe
	}
}

void AuGeoRoadDirt::CreateSlabModelSection(const DaWorldLL& world, AuScFaceList& faces, const AuPoint& prevLeft,
		const AuPoint& nextLeft, const AuPoint& nextRight, const AuPoint& prevRight, bool prevCap, bool nextCap) const
{
	// Use paint for striping.
	QColor noPaint(0,0,0,0);
	QColor whitePaint(255,255,255,200);

	// Calculate surface vectors.
	AuPoint vl = nextLeft - prevLeft;							// unit vector along left side
	vl.Unit();
	AuPoint vr = nextLeft - prevLeft;							// unit vector along right side
	vr.Unit();
	AuPoint plz = -world.GravityDir(prevLeft);					// up vector at prevLeft
	AuPoint nlz = -world.GravityDir(nextLeft);					// up vector at nextLeft
	AuPoint nrz = -world.GravityDir(nextRight);				// up vector at nextRight
	AuPoint prz = -world.GravityDir(prevRight);				// up vector at prevRight
	AuPoint nl = plz.Cross(vl);									// unit vector normal to the left side wall
	nl.Unit();
	AuPoint nr = vr.Cross(prz);									// unit vector normal to the right side wall
	nr.Unit();

	// Stripes to box in the center.
	double stripeWidth = 1;
	AuPoint cpl = prevLeft - nl * stripeWidth + vl * stripeWidth;
	AuPoint cnl = nextLeft - nl * stripeWidth - vl * stripeWidth;
	AuPoint cnr = nextRight - nr * stripeWidth - vr * stripeWidth;
	AuPoint cpr = prevRight - nr * stripeWidth + vr * stripeWidth;

	// Bottom points of slab.
	AuPoint plb = prevLeft - plz;
	AuPoint nlb = nextLeft - nlz;
	AuPoint nrb = nextRight - nrz;
	AuPoint prb = prevRight - prz;

	// Surface with striping.
	if(prevCap && nextCap)
	{
		CreateModelSectionQuad(faces, cpl, cnl, cnr, cpr, plz, nlz, nrz, prz, noPaint);	// Center slab
		CreateModelSectionQuad(faces, prevLeft, nextLeft, cnl, cpl, plz, nlz, nlz, plz, whitePaint);	// Left edge stripe.
		CreateModelSectionQuad(faces, nextLeft, nextRight, cpr, cpl, nlz, nrz, nrz, nlz, whitePaint);	// Next edge stripe.
		CreateModelSectionQuad(faces, nextRight, prevRight, cpr, cnr, nrz, prz, prz, nrz, whitePaint);	// Right edge stripe.
		CreateModelSectionQuad(faces, prevRight, prevLeft, cpl, cpr, nrz, nlz, nlz, nrz, whitePaint);	// Prev edge stripe.
	}
	else if(prevCap)
	{
		AuPoint nle(cnl + vl*stripeWidth);
		AuPoint nre(cnr + vr*stripeWidth);
		CreateModelSectionQuad(faces, cpl, nle, nre, cpr, plz, nlz, nrz, prz, noPaint);	// Center slab
		CreateModelSectionQuad(faces, prevLeft, nextLeft, nle, cpl, plz, nlz, nlz, plz, whitePaint);	// Left edge stripe.
		CreateModelSectionQuad(faces, nextRight, prevRight, cpr, nre, nrz, prz, prz, nrz, whitePaint);	// Right edge stripe.
		CreateModelSectionQuad(faces, prevRight, prevLeft, cpl, cpr, nrz, nlz, nlz, nrz, whitePaint);	// Prev edge stripe.
	}
	else if(nextCap)
	{
		AuPoint ple(cpl - vl*stripeWidth);
		AuPoint pre(cpr - vr*stripeWidth);
		CreateModelSectionQuad(faces, ple, cnl, cnr, pre, plz, nlz, nrz, prz, noPaint);	// Center slab
		CreateModelSectionQuad(faces, prevLeft, nextLeft, cnl, ple, plz, nlz, nlz, plz, whitePaint);	// Left edge stripe.
		CreateModelSectionQuad(faces, nextLeft, nextRight, cnr, cnl, nlz, nrz, nrz, nlz, whitePaint);	// Next edge stripe.
		CreateModelSectionQuad(faces, nextRight, prevRight, pre, cnr, nrz, prz, prz, nrz, whitePaint);	// Right edge stripe.
	}
	else
	{
		AuPoint nle(cnl + vl*stripeWidth);
		AuPoint nre(cnr + vr*stripeWidth);
		AuPoint ple(cpl - vl*stripeWidth);
		AuPoint pre(cpr - vr*stripeWidth);
		CreateModelSectionQuad(faces, ple, nle, nre, pre, plz, nlz, nrz, prz, noPaint);	// Center slab
		CreateModelSectionQuad(faces, prevLeft, nextLeft, nle, ple, plz, nlz, nlz, plz, whitePaint);	// Left edge stripe.
		CreateModelSectionQuad(faces, nextRight, prevRight, pre, nre, nrz, prz, prz, nrz, whitePaint);	// Right edge stripe.
	}

	// Underside.
	CreateModelSectionQuad(faces, prb, nrb, nlb, plb, -prz, -nrz, -nlz, -plz, noPaint);

	// Side walls.
	CreateModelSectionQuad(faces, nlb, nextLeft, prevLeft, plb, nl, nl, nl, nl, noPaint);	// Left side pavement.
	CreateModelSectionQuad(faces, prevRight, nextRight, nrb, prb, nr, nr, nr, nr, noPaint);	// Right side pavement.

	if(prevCap)
		CreateModelSectionQuad(faces, prevLeft, prevRight, prb, plb, -vl, -vl, -vl, -vl, noPaint);	// Prev end cap

	if(nextCap)
		CreateModelSectionQuad(faces, nextRight, nextLeft, nlb, nrb, vl, vl, vl, vl, noPaint);	// Prev end cap
}

void AuGeoRoadDirt::CreateModelSectionQuad(AuScFaceList& faces, const AuPoint& pl, const AuPoint& nl, const AuPoint& nr,
		const AuPoint& pr, const AuPoint& plz, const AuPoint& nlz, const AuPoint& nrz, const AuPoint& prz,
		const QColor& paint) const
{
	const double minSize = 8;
	AuPoint lv(nl - pl);
	double ll = lv.Length();
	AuPoint nv(nr - nl);
	double ln = nv.Length();
	AuPoint rv(nr - pr);
	double lr = rv.Length();
	AuPoint pv(pr - pl);
	double lp = pv.Length();
	if(ll <= minSize && ln <= minSize && lr <= minSize && lp <= minSize)
	{
		CreateModelQuad(faces, pl, nl, nr, pr, plz, nlz, nrz, prz, paint);
		return;
	}

	// Subdivide the quad.
	int stepsL;	// steps along the length of the quad.
	if(ll > lr)
		stepsL = (int)ceil(ll/minSize);
	else
		stepsL = (int)ceil(lr/minSize);
	int stepsW;	// steps along the width of the quad.
	if(ln > lp)
		stepsW = (int)ceil(ln/minSize);
	else
		stepsW = (int)ceil(lp/minSize);

	double yfracp = 0;
	for(int yd=1; yd<=stepsL; yd++)
	{
		// Walk up from the bottom.
		double yfrac = (double)yd / (double)stepsL;
		AuPoint lb = pl.LinearInterpolate(nl, yfracp);			// Left bottom, along my edge line.
		AuPoint lt = pl.LinearInterpolate(nl, yfrac);			// Left top, along my edge line.
		AuPoint rb = pr.LinearInterpolate(nr, yfracp);			// Right bottom, along my edge line.
		AuPoint rt = pr.LinearInterpolate(nr, yfrac);			// Right top, along my edge line.
		AuPoint lbn = plz.LinearInterpolate(nlz, yfracp);			// Left bottom, along my edge line.
		lbn.Unit();
		AuPoint ltn = plz.LinearInterpolate(nlz, yfrac);			// Left top, along my edge line.
		ltn.Unit();
		AuPoint rbn = prz.LinearInterpolate(nrz, yfracp);			// Right bottom, along my edge line.
		rbn.Unit();
		AuPoint rtn = prz.LinearInterpolate(nrz, yfrac);			// Right top, along my edge line.
		rtn.Unit();

		double xfracp = 0;
		for(int xd=1; xd<=stepsW; xd++)
		{
			double xfrac = (double)xd / (double)stepsW;

			AuPoint ll = lb.LinearInterpolate(rb, xfracp);
			AuPoint ul = lt.LinearInterpolate(rt, xfracp);
			AuPoint lr = lb.LinearInterpolate(rb, xfrac);
			AuPoint ur = lt.LinearInterpolate(rt, xfrac);

			AuPoint lln = lbn.LinearInterpolate(rbn, xfracp);
			lln.Unit();
			AuPoint uln = ltn.LinearInterpolate(rtn, xfracp);
			uln.Unit();
			AuPoint lrn = lbn.LinearInterpolate(rbn, xfrac);
			lrn.Unit();
			AuPoint urn = ltn.LinearInterpolate(rtn, xfrac);
			urn.Unit();
			CreateModelQuad(faces, ll, ul, ur, lr, lln, uln, urn, lrn, paint);

			xfracp = xfrac;
		}

		yfracp = yfrac;
	}
}

void AuGeoRoadDirt::CreateModelQuad(AuScFaceList& faces, const AuPoint& pl, const AuPoint& nl, const AuPoint& nr,
		const AuPoint& pr, const AuPoint& plz, const AuPoint& nlz, const AuPoint& nrz, const AuPoint& prz,
		const QColor& paint) const
{
	AuScPoint ll(pl, 0,0, plz);
	AuScPoint ul(nl, 0,1, nlz);
	AuScPoint ur(nr, 1,1, nrz);
	AuScPoint lr(pr, 1,0, prz);
	AuScFaceQuad q(ll, ul, ur, lr);
	q.SetPaint(paint);
	faces.AddFace(q);
}

INDUSTRYID AuGeoRoadDirt::GeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	if(AuRoadSegment::Contains(llworld, ll, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width/2, radius))
		return IID_Road;
	return IID_None;
}

void AuGeoRoadDirt::CreateRoadMap(const DaWorldLL& world, AuRoadMap& roadMap) const
{
	AuRoadSegment road(AuRoadSegment::GeoRoad, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width, world.Id());
	roadMap.AddRoad(world, road);
}

QColor AuGeoRoadDirt::MapColor() const
{
	/*
	if(IsSlab())
		return DbIndustry::MapColor(IID_None);					// Road slab
	else
	*/
		return DbIndustry::MapColor(IID_Road);					// Road
}

// Adds one or more quads to 'verts' to draw a road on a 2D map.
//static
void AuGeoRoadDirt::BuildRoadVerts(const AuPoint& s, const AuPoint& e, float wid2, float radiansPerFootX, float radiansPerFootY,
		AuPointArray& verts)
{
	// Calculate road vectors.
	AuPoint rvec = e - s;				// road vector
	AuPoint svec = rvec.PerpendicularVector_2D();
	svec.Unit_2D(wid2);
	svec.x *= radiansPerFootX;
	svec.y *= radiansPerFootY;

	// Calculate corner points of road segment quad.
	AuPoint sl = s - svec;
	AuPoint el = e - svec;
	AuPoint er = e + svec;
	AuPoint sr = s + svec;

	// Prevent wrap around the globe the wrong way.
	if(fabs(s.x) == 1)
	{
		sl.x = s.x;
		sr.x = s.x;
	}

	if(fabs(e.x) == 1)
	{
		el.x = e.x;
		er.x = e.x;
	}

	// Add the vertices.
	verts.append(sl);
	verts.append(el);
	verts.append(er);

	verts.append(er);
	verts.append(sr);
	verts.append(sl);
}

void AuGeoRoadDirt::RenderOnSurfaceTexture(const DaWorldLL& world, AuGLEffects& re, float radiansPerFootX,
		float radiansPerFootY) const
{
	if(world.Hydrographics() && !world.IsGasGiant() && LLMSL() < 0 && LL2MSL() < 0)
		return;													// Undersea cities cannot be seen on the surface texture.

	// Compile vertices of quads to draw road.
	AuPointArray drverts;										// road quads
	float wid2 = Radius();										// 1/2 width
	if(IsSlab())
		wid2 *= 1.3f;											// Wider to improve edge alignment between slab segments.
	float sx = LL().Longitude() / PIf;							// -1 to 1
	float sy = LL().Latitude() / (PIf/2.f);						// -1 to 1
	float ex = LL2().Longitude() / PIf;							// -1 to 1
	float ey = LL2().Latitude() / (PIf/2.f);					// -1 to 1
	float longang = LL2().Longitude() - LL().Longitude();
	if(fabs(longang) < PIf)
	{
		// Start to end is less than 180 degrees around the globe.
		BuildRoadVerts(AuPoint(sx, sy), AuPoint(ex, ey), wid2, radiansPerFootX, radiansPerFootY, drverts);
	}
	else
	{
		// Start to end is more than 180 degrees around the globe. Draw the segment in two pieces, to go around the
		// globe the other way.
		float lensx = 1.f - fabsf(sx);					// Distance from sx to seam.
		float lenex = 1.f - fabsf(ex);					// Distance from ex to seam.
		float lenx = lensx + lenex;						// Length of final segment.
		float frac = lensx/lenx;						// sx's fraction of the segment.
		float my = sy + frac*(ey - sy);					// Latitude at the seam.
		if(sx > ex)
		{
			BuildRoadVerts(AuPoint(sx, sy), AuPoint(1, my), wid2, radiansPerFootX, radiansPerFootY, drverts);
			BuildRoadVerts(AuPoint(-1, my), AuPoint(ex, ey), wid2, radiansPerFootX, radiansPerFootY, drverts);
		}
		else
		{
			BuildRoadVerts(AuPoint(sx, sy), AuPoint(-1, my), wid2, radiansPerFootX, radiansPerFootY, drverts);
			BuildRoadVerts(AuPoint(1, my), AuPoint(ex, ey), wid2, radiansPerFootX, radiansPerFootY, drverts);
		}
	}
	if(drverts.isEmpty())
		return;

	// The alpha value is a glow factor. The lower the alpha, the brighter the glow. The closer it approaches 1, the less glow.
	const QColor& c = AuGLLight::ColorSodiumVapor();
	re.GLSL().LineShader().SetColor(c.redF(), c.greenF(), c.blueF(), .95f);

	// Make a VAO of the vertex locations.
	AuGLVAO rvao;
	rvao.CreateForLines(re, drverts, false, GL_STREAM_DRAW);
	if(rvao.VAO())
	{
		// Display the VAO once and clean it up.
		re.glDisable(GL_DEPTH_TEST);
		re.glDisable(GL_CULL_FACE);
		rvao.SelectGL();
		rvao.DrawGL(GL_TRIANGLES);
		rvao.DeselectGL();
		rvao.DeleteGL();
		re.glEnable(GL_CULL_FACE);
		re.glEnable(GL_DEPTH_TEST);
	}
}

void AuGeoRoadDirt::PaintOnSurfaceMap(const DaWorldLL& world, QPainter& p, unsigned int width, unsigned int height) const
{
	Q_UNUSED(world);
	// Make slabs stand out.
	QColor color(MapColor());
	/*
	if(IsSlab())
		p.setPen(QPen(color, 5.f));
	else
	*/
		p.setPen(QPen(color, 0.f));

	// Draw the road segment.
	int sx = (int)((LL().Longitude() + PIf) * width / (PIf*2.f));
	int sy = (int)(((PIf/2.f) - LL().Latitude()) * height / PIf);
	int ex = (int)((LL2().Longitude() + PIf) * width / (PIf*2.f));
	int ey = (int)(((PIf/2.f) - LL2().Latitude()) * height / PIf);
	float longang = LL2().Longitude() - LL().Longitude();
	if(fabs(longang) < PIf)
	{
		// Start to end is less than 180 degrees around the globe.
		p.drawLine(sx,sy, ex,ey);
	}
	else
	{
		// Start to end is more than 180 degrees around the globe. Draw the segment in two pieces, to go around the globe the
		// other way.
		if(sx < ex)
		{
			int lenx = sx + (width-ex);						// Length of final segment.
			float frac = (float)sx/(float)lenx;				// sx's fraction of the segment.
			int my = sy + frac*(ey - sy);					// Latitude at the seam.
			p.drawLine(sx,sy, 0,my);
			p.drawLine(ex,ey, width,my);
		}
		else
		{
			int lenx = ex + (width-sx);						// Length of final segment.
			float frac = (float)ex/(float)lenx;				// ex's fraction of the segment.
			int my = ey + frac*(ey - sy);					// Latitude at the seam.
			p.drawLine(ex,ey, 0,my);
			p.drawLine(sx,sy, width,my);
		}
	}
}

quint64 AuGeoRoadDirt::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoRoadDirt);
	return mem;
}

ON_GROUND AuGeoRoadDirt::Ground() const
{
	return OG_Sand;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoRoadAsphalt::AuGeoRoadAsphalt(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width,
		bool slab, const DeConstructionProcess* cp) :
	AuGeoRoadDirt(startLL, startMSL, endLL, endMSL, width, slab, cp)
{
	m_VAO = 0;
}

AuGeoRoadAsphalt::AuGeoRoadAsphalt()
{
	m_VAO = 0;
}

AuGeoRoadAsphalt::~AuGeoRoadAsphalt()
{
	if(m_VAO)
		delete m_VAO;
}

AuGeoRoadAsphalt::AuGeoRoadAsphalt(const AuGeoRoadAsphalt& c) : AuGeoRoadDirt(c)
{
	m_VAO = 0;
}

bool AuGeoRoadAsphalt::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoRoadAsphalt& c = dynamic_cast<const AuGeoRoadAsphalt&>(b);
		return AuGeoRoadDirt::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoRoadAsphalt::Clone() const
{
	return new AuGeoRoadAsphalt(*this);
}

void AuGeoRoadAsphalt::operator<<(QDataStream& in)
{
	AuGeoRoadDirt::operator<<(in);
}

void AuGeoRoadAsphalt::operator>>(QDataStream& out) const
{
	AuGeoRoadDirt::operator>>(out);
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoRoadAsphalt::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	if(IsUnderConstruction())
		return QObject::tr("a road being made of pitch");
	else
		return QObject::tr("a road made of pitch");
}

void AuGeoRoadAsphalt::DeleteDeviceObjects(AuGLEffects& re)
{
	AuGeoRoadGrade::DeleteDeviceObjects(re);
	if(m_VAO && m_VAO->VAO())
	{
		m_VAO->DeleteGL();
		delete m_VAO;
		m_VAO = 0;
	}
}

void AuGeoRoadAsphalt::Render_Shader(AuGLSLScBase* shader, const DaWorldLL& world, const AuMatrix4x4& projection,
		const AuMatrix4x4& model, AuGLEffects& re)
{
	Q_UNUSED(projection);
	if(!m_VAO)
	{
		AuScFaceList faces;						// Triangle that make up the structure.
		CreatePavement(world, faces, 0);		// 0=asphalt
		if(!faces.isEmpty())
		{
			AuScTriRenderList opaqueTris, transparentTris;
			faces.RenderTriangles(opaqueTris, transparentTris);
			m_VAO = new AuGLVAO;
			m_VAO->CreateForSc(re, opaqueTris, GL_STATIC_DRAW, 0);
		}
	}

	if(m_VAO && m_VAO->VAO())
	{
		AuPoint loc = world.LocOfLL(LL());
		AuPoint loc2 = world.LocOfLL(LL2());
		AuPoint mid = (loc + loc2)/2;
		AuPoint vup = -world.GravityDir(mid);
		AuPoint ambientUp = model.ProjectVector(vup);			// Up vector in eye space for spherical ambient light calc.
		ambientUp.Unit();
		shader->SetAmbientLightUp(ambientUp);

		// Ambient light starts with starlight.
		float starLight;
		const DaSolarSystem* pss = dynamic_cast<const DaSolarSystem*>(world.FindEnvironmentAtom());
		if(pss)
			starLight = pss->StarLight();							// Actual starlight level based on local star density.
		else
			starLight = 0.2f;	// Designer preview?
		float sunLight = world.QuerySunlight(mid);
		float fillum = starLight + sunLight/2;
		float ambientR = fillum;
		float ambientG = fillum;
		float ambientB = fillum;
		float cityAmbientLight = (world.BuildingAmbientLight(LL()) + world.BuildingAmbientLight(LL2())) / 2;
		if(cityAmbientLight > 0.f)
		{
			const QColor& c = AuGLLight::ColorSodiumVapor();
			ambientR += cityAmbientLight * c.redF() * AuGLLight::AmbientLitCity();
			ambientG += cityAmbientLight * c.greenF() * AuGLLight::AmbientLitCity();
			ambientB += cityAmbientLight * c.blueF() * AuGLLight::AmbientLitCity();
		}

		if(ambientR < 0)
			ambientR = 0;
		else if(ambientR > 1)
			ambientR = 1;

		if(ambientG < 0)
			ambientG = 0;
		else if(ambientG > 1)
			ambientG = 1;

		if(ambientB < 0)
			ambientB = 0;
		else if(ambientB > 1)
			ambientB = 1;

		shader->SetAmbientLight(ambientR, ambientG, ambientB);

		// Render road exterior.
		m_VAO->SelectGL();
		m_VAO->DrawGL();
		m_VAO->DeselectGL();
	}
}

AuGeologicalFeature* AuGeoRoadAsphalt::UnpavedEquivalent() const
{
	return AuGeoRoadDirt::Clone();
}

QColor AuGeoRoadAsphalt::MapColor() const
{
	/*
	if(IsSlab())
		return DbIndustry::MapColor(IID_None);					// Road slab
	else
	*/
		return DbIndustry::MapColor(IID_Road);					// Road
}

quint64 AuGeoRoadAsphalt::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoRoadAsphalt);
	return mem;
}

bool AuGeoRoadAsphalt::TestBombCraterHit(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	return AuRoadSegment::Contains(llworld, ll, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width/2, radius);
}

ON_GROUND AuGeoRoadAsphalt::Ground() const
{
	return OG_Pavement;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoRoadConcrete::AuGeoRoadConcrete(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width,
		bool slab, const DeConstructionProcess* cp) :
	AuGeoRoadDirt(startLL, startMSL, endLL, endMSL, width, slab, cp)
{
	m_VAO = 0;
}

AuGeoRoadConcrete::AuGeoRoadConcrete()
{
	m_VAO = 0;
}

AuGeoRoadConcrete::~AuGeoRoadConcrete()
{
	if(m_VAO)
		delete m_VAO;
}

AuGeoRoadConcrete::AuGeoRoadConcrete(const AuGeoRoadConcrete& c) : AuGeoRoadDirt(c)
{
	m_VAO = 0;
}

bool AuGeoRoadConcrete::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoRoadConcrete& c = dynamic_cast<const AuGeoRoadConcrete&>(b);
		return AuGeoRoadDirt::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoRoadConcrete::Clone() const
{
	return new AuGeoRoadConcrete(*this);
}

void AuGeoRoadConcrete::operator<<(QDataStream& in)
{
	AuGeoRoadDirt::operator<<(in);
}

void AuGeoRoadConcrete::operator>>(QDataStream& out) const
{
	AuGeoRoadDirt::operator>>(out);
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoRoadConcrete::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	if(IsUnderConstruction())
		return QObject::tr("a road being made of stone");
	else
		return QObject::tr("a road made of stone");
}

void AuGeoRoadConcrete::DeleteDeviceObjects(AuGLEffects& re)
{
	AuGeoRoadGrade::DeleteDeviceObjects(re);
	if(m_VAO && m_VAO->VAO())
	{
		m_VAO->DeleteGL();
		delete m_VAO;
		m_VAO = 0;
	}
}

void AuGeoRoadConcrete::Render_Shader(AuGLSLScBase* shader, const DaWorldLL& world, const AuMatrix4x4& projection,
		const AuMatrix4x4& model, AuGLEffects& re)
{
	Q_UNUSED(projection);
	if(!m_VAO)
	{
		AuScFaceList faces;						// Triangle that make up the structure.
		CreatePavement(world, faces, 1);		// 1=concrete
		if(!faces.isEmpty())
		{
			AuScTriRenderList opaqueTris, transparentTris;
			faces.RenderTriangles(opaqueTris, transparentTris);
			m_VAO = new AuGLVAO;
			m_VAO->CreateForSc(re, opaqueTris, GL_STATIC_DRAW, 0);
		}
	}

	if(m_VAO && m_VAO->VAO())
	{
		AuPoint loc = world.LocOfLL(LL());
		AuPoint loc2 = world.LocOfLL(LL2());
		AuPoint mid = (loc + loc2)/2;
		AuPoint vup = -world.GravityDir(mid);
		AuPoint ambientUp = model.ProjectVector(vup);			// Up vector in eye space for spherical ambient light calc.
		ambientUp.Unit();
		shader->SetAmbientLightUp(ambientUp);

		// Ambient light starts with starlight.
		float starLight;
		const DaSolarSystem* pss = dynamic_cast<const DaSolarSystem*>(world.FindEnvironmentAtom());
		if(pss)
			starLight = pss->StarLight();							// Actual starlight level based on local star density.
		else
			starLight = 0.2f;	// Designer preview?
		float sunLight = world.QuerySunlight(mid);
		float fillum = starLight + sunLight/2;
		float ambientR = fillum;
		float ambientG = fillum;
		float ambientB = fillum;
		float cityAmbientLight = (world.BuildingAmbientLight(LL()) + world.BuildingAmbientLight(LL2())) / 2;
		if(cityAmbientLight > 0.f)
		{
			const QColor& c = AuGLLight::ColorSodiumVapor();
			ambientR += cityAmbientLight * c.redF() * AuGLLight::AmbientLitCity();
			ambientG += cityAmbientLight * c.greenF() * AuGLLight::AmbientLitCity();
			ambientB += cityAmbientLight * c.blueF() * AuGLLight::AmbientLitCity();
		}

		if(ambientR < 0)
			ambientR = 0;
		else if(ambientR > 1)
			ambientR = 1;

		if(ambientG < 0)
			ambientG = 0;
		else if(ambientG > 1)
			ambientG = 1;

		if(ambientB < 0)
			ambientB = 0;
		else if(ambientB > 1)
			ambientB = 1;

		shader->SetAmbientLight(ambientR, ambientG, ambientB);

		// Render road exterior.
		m_VAO->SelectGL();
		m_VAO->DrawGL();
		m_VAO->DeselectGL();
	}
}

AuGeologicalFeature* AuGeoRoadConcrete::UnpavedEquivalent() const
{
	return AuGeoRoadDirt::Clone();
}

QColor AuGeoRoadConcrete::MapColor() const
{
	/*
	if(IsSlab())
		return DbIndustry::MapColor(IID_None);					// Road slab
	else
	*/
		return DbIndustry::MapColor(IID_Road);					// Road
}

quint64 AuGeoRoadConcrete::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoRoadConcrete);
	return mem;
}

bool AuGeoRoadConcrete::TestBombCraterHit(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	return AuRoadSegment::Contains(llworld, ll, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width/2, radius);
}

ON_GROUND AuGeoRoadConcrete::Ground() const
{
	return OG_Pavement;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoLandscape::AuGeoLandscape(const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL, float width,
		bool hideRocks, int hideGrass, bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavementType,
		const DeConstructionProcess* cp) : AuGeologicalFeature(endLL)
{
	m_EndMSL = endMSL;
	m_HideRocks = hideRocks;
	m_HideGrass = hideGrass;
	m_HideShrubs = hideShrubs;
	m_HideTrees = hideTrees;
	m_Pavement = pavementType;
	if(cp)
		m_pConstruct = new DeConstructionProcess(*cp);
	else
		m_pConstruct = 0;
	m_StartLL = startLL;
	m_StartMSL = startMSL;
	m_TimeCreated.Set();
	m_Width = width;
}

AuGeoLandscape::AuGeoLandscape()
{
	m_EndMSL = 0;
	m_HideRocks = false;
	m_HideGrass = 0;
	m_HideShrubs = false;
	m_HideTrees = false;
	m_Pavement = WORLDTEX_ConstructionZone;
	m_pConstruct = 0;
	m_StartMSL = 0;
	m_Width = 0;
}

AuGeoLandscape::AuGeoLandscape(const AuGeoLandscape& c) : AuGeologicalFeature(c)
{
	m_EndMSL = c.m_EndMSL;
	m_HideRocks = c.m_HideRocks;
	m_HideGrass = c.m_HideGrass;
	m_HideShrubs = c.m_HideShrubs;
	m_HideTrees = c.m_HideTrees;
	m_Pavement = c.m_Pavement;
	if(c.m_pConstruct)
		m_pConstruct = new DeConstructionProcess(*c.m_pConstruct);
	else
		m_pConstruct = 0;
	m_StartLL = c.m_StartLL;
	m_StartMSL = c.m_StartMSL;
	m_TimeCreated = c.m_TimeCreated;
	m_Width = c.m_Width;
}

bool AuGeoLandscape::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoLandscape& c = dynamic_cast<const AuGeoLandscape&>(b);
		if(m_TimeCreated != c.m_TimeCreated)
			return false;
		if(m_EndMSL != c.m_EndMSL)
			return false;
		if(m_HideRocks != c.m_HideRocks)
			return false;
		if(m_HideGrass != c.m_HideGrass)
			return false;
		if(m_HideShrubs != c.m_HideShrubs)
			return false;
		if(m_HideTrees != c.m_HideTrees)
			return false;
		if(m_Pavement != c.m_Pavement)
			return false;
		if(m_pConstruct && c.m_pConstruct)
		{
			// Both under construction.
			if(*m_pConstruct != *c.m_pConstruct)
				return false;
		}
		else if(IsUnderConstruction() != c.IsUnderConstruction())
			return false;
		if(m_StartLL != c.m_StartLL)
			return false;
		if(m_StartMSL != c.m_StartMSL)
			return false;
		if(m_Width != c.m_Width)
			return false;
		return AuGeologicalFeature::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoLandscape::Clone() const
{
	return new AuGeoLandscape(*this);
}

void AuGeoLandscape::operator>>(QDataStream& out) const
{
	AuGeologicalFeature::operator>>(out);
	quint8 hasWhat = m_HideGrass;	// Bottom 2 bits
	if(m_HideRocks)
		hasWhat |= 0x04;
	if(m_HideShrubs)
		hasWhat |= 0x08;
	if(m_HideTrees)
		hasWhat |= 0x10;
	if(m_Pavement)
		hasWhat |= 0x20;
	if(!m_TimeCreated.IsNull())
		hasWhat |= 0x40;
	if(m_pConstruct)
		hasWhat |= 0x80;
	out << hasWhat;
	out << m_EndMSL;
	out << m_StartLL;
	out << m_StartMSL;
	out << m_Width;
	if(hasWhat & 0x20)
		out << (quint8)m_Pavement;
	if(hasWhat & 0x40)
		out << m_TimeCreated;
	if(hasWhat & 0x80)
		*m_pConstruct >> out;
}

void AuGeoLandscape::operator<<(QDataStream& in)
{
	AuGeologicalFeature::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	m_HideGrass = hasWhat & 0x03;	// Bottom 2 bits.
	m_HideRocks = hasWhat & 0x04;
	m_HideShrubs = hasWhat & 0x08;
	m_HideTrees = hasWhat & 0x10;
	in >> m_EndMSL;
	in >> m_StartLL;
	in >> m_StartMSL;
	in >> m_Width;
	if(hasWhat & 0x20)
	{
		quint8 p;
		in >> p;
		m_Pavement = (WORLDTEX_TYPE)p;
	}
	if(hasWhat & 0x40)
		in >> m_TimeCreated;
	if(hasWhat & 0x80)
	{
		if(!m_pConstruct)
			m_pConstruct = new DeConstructionProcess();
		*m_pConstruct << in;
	}
	else
	{
		if(m_pConstruct)
		{
			delete m_pConstruct;
			m_pConstruct = 0;
		}
	}
}

// Adjusts 'altMSL' at 'adjLL' on 'llworld' based on this geological feature.
// Returns the adjusted altitude. Will be the same as 'altMSL' if no adjustment is made.
double AuGeoLandscape::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, bool& hideRocks,
		int& hideGrass, bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	// Landscape segment does not grade terrain, only affects appearance of the area.
	// Calculate the closest point on this road segment to 'adjLL'.
	float segFrac;
	AuGlobeLLF tll(adjLL.ToLLF());
	AuGlobeLLF segLL = tll.CoordOnSegment(m_StartLL, m_LL, &segFrac, llworld.LLShape());
	float segAltMSL;
	AuPoint segLoc;
	if(segFrac <= 0)
	{
		// Closest point is the starting location.
		segAltMSL = m_StartMSL;
		segLoc = llworld.LocOfLL(m_StartLL, segAltMSL);
	}
	else if(segFrac >= 1)
	{
		// Closest point is the ending location.
		segAltMSL = m_EndMSL;
		segLoc = llworld.LocOfLL(m_LL, segAltMSL);
	}
	else
	{
		// Calculate the location along this road segment.
		segAltMSL = m_StartMSL + (m_EndMSL - m_StartMSL) * segFrac;
		segLoc = llworld.LocOfLL(segLL, segAltMSL);
	}

	// Calculate the location of adjLL at the altitude of the segment location.
	AuPoint adjLoc = llworld.LocOfLL(adjLL, segAltMSL);

	// Determine if the search location is within the build radius of the development, at the altitude of the development.
	float dist2 = (float)adjLoc.Distance2(segLoc);
	float bldRad = m_Width/2 + 8;

	if(segFrac > 0 && segFrac < 1)
	{
		// The point is between my end points.
		float bldRad2 = bldRad * bldRad;						// squared
		if(dist2 <= bldRad2)
		{
			// The point is on this road segment, between the end points. This is the zone in which my pavement coverage is 100%.
			if(IsUnderConstruction())
			{
				pavementType = WORLDTEX_ConstructionZone;
				hideGrass = 1;
				hideShrubs = true;
				hideTrees = true;
			}
			else
			{
				if(m_HideRocks)
					hideRocks = true;
				if(m_HideGrass)
					hideGrass = m_HideGrass;
				if(m_HideShrubs)
					hideShrubs = true;
				if(m_HideTrees)
					hideTrees = true;
				if(m_Pavement != WORLDTEX_ConstructionZone)
					pavementType = m_Pavement;
			}
		}
	}
	return altMSL;
}

double AuGeoLandscape::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(llworld);
	Q_UNUSED(adjLL);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	Q_UNUSED(newest);
	return altMSL;
}

double AuGeoLandscape::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL) const
{
	Q_UNUSED(llworld);
	Q_UNUSED(adjLL);
	return altMSL;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoLandscape::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	if(IsUnderConstruction())
		return QObject::tr("a clearing under construction");
	else if(m_HideGrass == 1)
		return QObject::tr("a barren clearing");	// defoliated area uses this setting
	else if(m_HideGrass == 2)
		return QObject::tr("a clearing");			// clearing uses this setting
	else if(m_HideGrass == 3)
		return QObject::tr("a lush clearing");		// irrigated area uses this setting
	else
		return QObject::tr("a clearing");			// wut?
}

void AuGeoLandscape::RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		AuGLEffects& re, const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	// Use simple shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * model);
	re.glDisable(GL_DEPTH_TEST);
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);							// Additive blend keeps it nice and bright.
	RenderCursor_Shader(world, re, nearLoc, hitRoad, hitColor, roadColor);
	re.glDisable(GL_BLEND);
	re.glEnable(GL_DEPTH_TEST);
	re.GLSL().LineShader().Deselect();
}

void AuGeoLandscape::RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
		const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	AuPoint startLoc = world.LocOfLL(m_StartLL, m_StartMSL);
	AuPoint endLoc = world.LocOfLL(m_LL, m_EndMSL);
	if(nearLoc.DistToSegment(startLoc, endLoc) > 1000)
		return;

	// Draw a linear cursor from the start location to the end location.
	// Right/left designations are in the context of standing at startLoc and looking down the road segment to endLoc.
	QColor color;
	if(hitRoad == LL())
		color = hitColor;
	else
		color = roadColor;

	float bldRad = m_Width/2;
	AuPoint centerVec = endLoc - startLoc;
	double centerLen = centerVec.Length();
	Q_ASSERT(centerLen != 0);
	centerVec /= centerLen;

	// Build road edge lines and center lines.
	// Start with the starting locations.
	AuPointArray roadCenter;
	AuPointArray roadLeft;
	AuPointArray roadRight;
	AuPointArray roadLegs;	// Lines across the road to make a grid.

	// Integrate along the length so we conform to the curvature of the world.
	double seglen = 10*FeetPerMeter();
	int segs = (int)(centerLen / seglen) + 1;			// Do one too many for perfect ending.
	bool atEnd = false;
	for(int seg=0; seg<=segs && !atEnd; seg++)
	{
		double lenAlongSeg = seglen * seg;
		if(lenAlongSeg >= centerLen)
		{
			// We are at the end. Truncate the last segment for perfect ending.
			//qDebug() << "segment" << lenAlongSeg << "clipped to" << centerLen << "seg" << seg << "segs" << segs;
			lenAlongSeg = centerLen;				
			atEnd = true;
		}
		double segRatio = lenAlongSeg / centerLen;
		double altAtSegLoc = m_StartMSL + (m_EndMSL - m_StartMSL) * segRatio;	// Linear alt change from start to end.
		AuPoint segLoc = startLoc + centerVec * lenAlongSeg;
		AuPoint segUp = -world.GravityDir(segLoc);
		world.ElevateLocMSL(segLoc, altAtSegLoc);

		AuPoint segRight = centerVec.Cross(segUp);
		segRight.Unit();

		// Add road level points.
		roadCenter.append(segLoc);
		roadLeft.append(segLoc - segRight*bldRad);
		roadRight.append(segLoc + segRight*bldRad);

		// Add pairs to draw them as GL_LINES.
		roadLegs.append(roadLeft.last());
		roadLegs.append(roadCenter.last());
		roadLegs.append(roadCenter.last());
		roadLegs.append(roadRight.last());
	}

	// Draw lines.
	re.GLSL().LineShader().SetColor(color);

	// Left
	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForLines(re, roadLeft, false, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	// Center
	{
		// Center line on landscapers.
		// Compile a list of vertex locations.
		AuPointArray verts;
		verts.append(roadRight.first());
		verts.append(roadLeft.first());
		for(AuPointArray::const_iterator ri = roadCenter.constBegin(); ri != roadCenter.constEnd(); ri++)
			verts.append(*ri);
		verts.append(roadRight.last());
		verts.append(roadLeft.last());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Right
	// Make a VAO of the vertex locations.
	vao.CreateForLines(re, roadRight, false, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}
}

bool AuGeoLandscape::IsDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	return AuRoadSegment::Contains(llworld, ll, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width/2, radius);
}

INDUSTRYID AuGeoLandscape::GeoIndustryContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	if(IsUnderConstruction() && AuRoadSegment::Contains(llworld, ll, m_StartLL, m_StartMSL, m_LL, m_EndMSL, m_Width/2, radius))
		return IID_Landscape;
	return IID_None;
}

bool AuGeoLandscape::ConstructionProcess(DeConstructionProcess& cp) const
{
	if(!m_pConstruct)
		return false;
	cp = *m_pConstruct;
	return true;
}

bool AuGeoLandscape::ConstructionEnable(bool enable)
{
	if(!m_pConstruct)
		return false;
	if(enable)
	{
		if(m_pConstruct->IsHalted())
			return false;	// no change
		m_pConstruct->Resume();
	}
	else
	{
		if(!m_pConstruct->IsHalted())
			return false;	// no change
		m_pConstruct->Halt();
	}
	return true;
}

bool AuGeoLandscape::ConstructionFetch(const DaWorldLL& llworld)
{
	Q_UNUSED(llworld);
	if(!m_pConstruct)
		return false;
	// Landscapers do not require materials or tools.
	AuItemHash inventory, tools;
	return m_pConstruct->FetchMaterials(inventory, tools);
}

int AuGeoLandscape::ConstructionGo(AuGear& gear)
{
	if(!m_pConstruct)
		return 0;						// I am not under construction.
	if(m_pConstruct->LaborNeeded())
	{
		// Verify presence of required items before wearing down any tools.
		bool reqMissing = false;
		for(DeProcessComponentArray::iterator cc = m_pConstruct->Components().begin(); cc != m_pConstruct->Components().end(); ++cc)
		{
			if((*cc).IsRequiredModifier())
			{
				// This is a required component that is not consumed.
				if((*cc).IsSatisfied())
					continue;					// Requirement has already been satisfied.

				// Attempt to satisfy using the gear.
				if(!gear.FindReadyItem((*cc).CommodityId()).IsValid())
				{
					reqMissing = true;	// Required component is not present.
					break;
				}
			}
		}
		if(reqMissing)
			return 0x01;					// Still under construction. Some required item is missing.

		int ilabor = 4;						// Standard amount of labor per effort.

		// Set the satisfied state of requirements that don't consume anything, based on current availability of tools, etc.
		// This helps to eliminate a step when the only requirement might be a tool and the unit is equipped with one.
		// Check to see if the person applying the effort has any objects that will add to the labor applied.
		int lLaborBoost = 0;
		for(DeProcessComponentArray::iterator cc = m_pConstruct->Components().begin(); cc != m_pConstruct->Components().end(); ++cc)
		{
			if((*cc).Effect() == CE_DecreasesTime)
			{
				// This component affects labor rate.
				AuGearPos pos = gear.FindReadyItem((*cc).CommodityId());
				if(pos.IsValid())
				{
					const AuItem* item = gear.ItemAt(pos);
					if(item)
					{
						int boost = item->GetLaborBoost();
						if(boost)
						{
							(*cc).SetQtyApplied(1);
							lLaborBoost += boost;
						}
					}
				}
			}
			else if((*cc).IsRequiredModifier())
			{
				// This is a required component that is not consumed.
				if((*cc).IsSatisfied())
					continue;					// Requirement has already been satisfied.

				// Check the inventory of the unit to determine if any equipped item satisfies the requirement.
				if(gear.FindReadyItem((*cc).CommodityId()).IsValid())
					(*cc).SetQtyApplied(1);
			}
		}
		ilabor += lLaborBoost;
		m_pConstruct->ApplyConstructionLabor(ilabor);
	}
	if(m_pConstruct->IsComplete())
	{
		delete m_pConstruct;
		m_pConstruct = 0;
		return 0x02;	// Construction complete.
	}
	return 0x01;	// Still under construction.
}

bool AuGeoLandscape::IsConstructionWorkerEnabled() const
{
	return m_pConstruct && !m_pConstruct->IsHalted();
}

quint64 AuGeoLandscape::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoLandscape);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeoBombCrater::AuGeoBombCrater(float radius, float debrisHeight, float bottomMSL, const AuGlobeLLF& ll) : AuGeoCrater(radius,
		debrisHeight, bottomMSL, ll)
{
	m_TimeCreated.Set();
}

AuGeoBombCrater::AuGeoBombCrater()
{
}

AuGeoBombCrater::AuGeoBombCrater(const AuGeoBombCrater& c) : AuGeoCrater(c)
{
	m_TimeCreated = c.m_TimeCreated;
}

bool AuGeoBombCrater::operator==(const AuGeologicalFeature& b) const
{
	try
	{
		const AuGeoBombCrater& c = dynamic_cast<const AuGeoBombCrater&>(b);
		if(m_TimeCreated != c.m_TimeCreated)
			return false;
		return AuGeoCrater::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuGeologicalFeature* AuGeoBombCrater::Clone() const
{
	return new AuGeoBombCrater(*this);
}

void AuGeoBombCrater::operator>>(QDataStream& out) const
{
	AuGeoCrater::operator>>(out);
	out << m_TimeCreated;
}

void AuGeoBombCrater::operator<<(QDataStream& in)
{
	AuGeoCrater::operator<<(in);
	in >> m_TimeCreated;
}

// 'worldGenSeed' is the GenSeed for this feature's world, in case we want to consistently generate a name.
// Returns the name to fit into these sentences:
//   "Nearby is %1."
//   "You should see %1."
//   "Visitors from afar tell about %1."
QString AuGeoBombCrater::IndigenousName(HYDROGRAPHIC_LIQUID hliq, quint32 worldGenSeed) const
{
	Q_UNUSED(hliq);
	Q_UNUSED(worldGenSeed);
	if(m_BottomMSL <= 0)
	{
		// Radius is 100 to 500m.
		if(m_Radius < 150*FeetPerMeter())
			return QObject::tr("a water filled bomb crater");
		else if(m_Radius < 350*FeetPerMeter())
			return QObject::tr("a large water filled bomb crater");
		else
			return QObject::tr("a huge water filled bomb crater");
	}
	else
	{
		// Radius is 100 to 500m.
		if(m_Radius < 150*FeetPerMeter())
			return QObject::tr("a bomb crater");
		else if(m_Radius < 350*FeetPerMeter())
			return QObject::tr("a large bomb crater");
		else
			return QObject::tr("a huge bomb crater");
	}
}

bool AuGeoBombCrater::IsDeletableCraterContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	return IsDeletableContaining(llworld, ll, radius);
}

bool AuGeoBombCrater::IsDeletableContaining(const DaWorldLL& llworld, const AuGlobeLLF& ll, double radius) const
{
	AuPoint myloc = llworld.LocOfLL(m_LL, m_BottomMSL);
	AuPoint loc = llworld.LocOfLL(ll, m_BottomMSL);
	float dist = loc.Distance(myloc);
	float debrisRad = m_Radius * 1.2f;
	if(dist < debrisRad + radius)
		return true;	// Location is in the debris zone of the crater, outside the rim.
	return false;
}

double AuGeoBombCrater::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, bool& hideRocks,
		int& hideGrass, bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(hideRocks);
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	AuPoint myloc = llworld.LocOfLL(m_LL, m_BottomMSL);
	AuPoint loc = llworld.LocOfLL(adjLL, m_BottomMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the crater.
		// Add debris at the rim, dropping off exponentially to the center.
		// Rim is at native terrain MSL + debrisHeight.
		// Center is at bottomMSL, rising exponentially to the debris on top of the rim.
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
		altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL + debrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
		hideGrass = 1;
		hideShrubs = true;
		hideTrees = true;
		if(newest < m_TimeCreated)
			newest = m_TimeCreated;
	}
	else
	{
		float debrisRad = m_Radius * 1.2f;
		if(dist < debrisRad)
		{
			// Location is in the debris zone of the crater, outside the rim.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float debrisFactor = 1. - (dist - m_Radius) / (debrisRad - m_Radius);	// 1 at the rim, 0 at the outskirts
			float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(debrisFactor));
			altMSL += debrisHeight;
			if(newest < m_TimeCreated)
				newest = m_TimeCreated;
		}
	}
	return altMSL;
}

double AuGeoBombCrater::AdjustAltMSL(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL, WORLDTEX_TYPE& pavementType,
		int& onRoadOrGrader, AuTime& newest) const
{
	Q_UNUSED(pavementType);
	Q_UNUSED(onRoadOrGrader);
	AuPoint myloc = llworld.LocOfLL(m_LL, m_BottomMSL);
	AuPoint loc = llworld.LocOfLL(adjLL, m_BottomMSL);
	float dist = loc.Distance(myloc);
	if(dist < m_Radius)
	{
		// Location is inside the rim of the crater.
		// Add debris at the rim, dropping off exponentially to the center.
		// Rim is at native terrain MSL + debrisHeight.
		// Center is at bottomMSL, rising exponentially to the debris on top of the rim.
		float radiusFactor = dist / m_Radius;					// Linear from 0 at the center to 1 at the rim.
		float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
		altMSL = AuNoiseF::Linear_Interpolate(m_BottomMSL, altMSL + debrisHeight, AuNoiseF::ExponentialCurve(radiusFactor, 4));
		if(newest < m_TimeCreated)
			newest = m_TimeCreated;
	}
	else
	{
		float debrisRad = m_Radius * 1.2f;
		if(dist < debrisRad)
		{
			// Location is in the debris zone of the crater, outside the rim.
			// Add debris at the rim, dropping off exponentially to the outer edge.
			float debrisFactor = 1. - (dist - m_Radius) / (debrisRad - m_Radius);	// 1 at the rim, 0 at the outskirts
			float debrisHeight = AuNoiseF::Linear_Interpolate(0, m_DebrisHeight, AuNoiseF::ExponentialCurve(debrisFactor));
			altMSL += debrisHeight;
			if(newest < m_TimeCreated)
				newest = m_TimeCreated;
		}
	}
	return altMSL;
}

double AuGeoBombCrater::AdjustAltMSLUndeveloped(const DaWorldLL& llworld, const AuGlobeLL& adjLL, double altMSL) const
{
	Q_UNUSED(llworld);
	Q_UNUSED(adjLL);
	return altMSL;
}

void AuGeoBombCrater::RenderCursor(const DaWorldLL& world, const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuPoint& nearLoc, const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	// Use simple shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * model);
	re.glDisable(GL_DEPTH_TEST);
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);							// Additive blend keeps it nice and bright.
	RenderCursor_Shader(world, re, nearLoc, hitRoad, hitColor, roadColor);
	re.glDisable(GL_BLEND);
	re.glEnable(GL_DEPTH_TEST);
	re.GLSL().LineShader().Deselect();
}

void AuGeoBombCrater::RenderCursor_Shader(const DaWorldLL& world, AuGLEffects& re, const AuPoint& nearLoc,
		const AuGlobeLLF& hitRoad, const QColor& hitColor, const QColor& roadColor) const
{
	AuPoint loc = world.LocOfLL(LL(), m_BottomMSL);
	float dist = loc.Distance(nearLoc);
	if(dist > m_Radius*2 && dist > 100*FeetPerMeter())
		return;	// Too far away to bother drawing.
	AuPointArray circle;
	circle.MakeCircle(AuPoint(0,0), AuPoint(m_Radius,0), 20);
	AuPoint vup = -world.GravityDir(loc);
	AuPoint vx;
	vup.ArbitraryAxis(vx);
	AuPoint vy = vup.Cross(vx);
	AuMatrix4x4 gM(loc, vx, vy);
	circle.Transform(gM);

	QColor color;
	if(hitRoad == LL())
		color = hitColor;
	else
		color = roadColor;
	re.GLSL().LineShader().SetColor(color);

	// Display the VAO once and clean it up.
	AuGLVAO vao;
	vao.CreateForLines(re, circle, true, GL_STREAM_DRAW);
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}
}

quint64 AuGeoBombCrater::MemoryUsage() const
{
	quint64 mem = sizeof(AuGeoBombCrater);
	return mem;
}

bool AuGeoBombCrater::IsSmokey() const
{
	// Bomb craters emit smoke for a while, based on their size.
	int smokeSec = (int)ceil(Radius() * 20);
	//int elapsedSec = m_TimeCreated.ElapsedSec();
	//if(elapsedSec < smokeSec)
		//qDebug() << "AuGeoBombCrater::IsSmokey" << LL() << "radius" << Radius()
			//<< "smokeSec" << smokeSec << m_TimeCreated.ElapsedSec();
	return m_TimeCreated.ElapsedSec() < smokeSec;
}

double AuGeoBombCrater::SmokeRadius() const
{
	return Radius() / 2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
