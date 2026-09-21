//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldring.cpp,v 1.164 2022/05/17 18:43:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daworldring.h"
//
#include "auatomrenderinfo.h"
#include "aunoise.h"
#include "auParticleSystem.h"
#include "auscparts.h"
#include "ausensorblip.h"
#include "auzoneresource.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "daworldgasgiant.h"
#include <QSqlError>
#include <QSqlQuery>

DaWorldRing::DaWorldRing()
{
	m_glTexRing = 0;
	m_RadInside = 0;
	m_RadOutside = 0;
	m_Resources = 0;
}

DaWorldRing::~DaWorldRing()
{
	/*
#if !defined(QT_NO_DEBUG)
	if(m_Ring.VAO())
		qDebug() << "D DaWorldRing::~DaWorldRing - Ring =" << m_Ring.VAO();
#endif
*/

	if(m_Resources)
	{
		delete [] m_Resources;
		m_Resources = 0;
	}
}

DaWorldRing::DaWorldRing(const DaWorldRing &rhs) : DaWorld(rhs)
{
	//qDebug("DaWorldRing:: copy constructor id %d", rhs.Id().GetID());
	m_glTexRing = 0;
	rhs.m_ResourcesLock.lockForRead();
	if(rhs.m_Resources)
	{
		m_Resources = new AuZoneResource[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_Resources[rz] = rhs.m_Resources[rz];
	}
	else
		m_Resources = 0;
	rhs.m_ResourcesLock.unlock();

	// Do not copy Think variables.
	m_RadInside = rhs.m_RadInside;
	m_RadOutside = rhs.m_RadOutside;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaWorldRing::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaWorld::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
}

// Receive updates to this atom from a server.
void DaWorldRing::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaWorld::RxUpdate(fromServer, isFirstUpdate);
}

// Refresh data from valid counterpart.
bool DaWorldRing::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaWorld::Refresh(pAtom, ari);

	//const DaWorldRing* pA = (const DaWorldRing*)pAtom;

	return false;
}

DaAtom *DaWorldRing::Clone() const
{
	return new DaWorldRing(*this);
}

void DaWorldRing::Create(double orbitRadius, ORBITZONE orbitZone)
{
	DaWorld::Create(orbitRadius, orbitZone);
}

void DaWorldRing::CreateForScan(double orbitRadius, ORBITZONE orbitZone)
{
	DaWorld::CreateForScan(orbitRadius, orbitZone);
}

QString DaWorldRing::DefaultName(const QString& solarSystemName) const
{
	// The name is empty so do the work to build the name.
	const DaStar* pstar = FindParentStarInEnv();
	if(!pstar)
		return StringId();

	// Develop a name based on this planets position in the solarsystem.

	// What is the star's position in the solar system.
	int istar = pstar->StarNumber();
	const char *squal;
	switch(istar)
	{
	case -1:
	default:
		squal = "";
		break;
	case 0:
		squal = " Alpha";
		break;
	case 1:
		squal = " Beta";
		break;
	case 2:
		squal = " Gamma";
		break;
	}

	// Get orbit radius so I can determine which position I hold around my parent.
	int ipos = 0;
	const DaWorld* planet = (const DaWorld*)Parent();
	double myrad = OrbitRadius();
	DaAtomList almoons;
	planet->ListPlanets(almoons);
	for(DaAtomList::const_iterator it = almoons.begin(); it != almoons.end(); it++)
	{
		const DaWorld *pb = (const DaWorld*)(*it);
		if(pb->OrbitRadius() < myrad - (FeetPerMoonOrbit()/2.0))
			ipos++;						// Found something in a closer orbit than me.
	}

	// Moon designator.
	QChar satorbit = QChar('a' + ipos);

	// Use parent's orbit radius for planet designation based on solar orbit.
	myrad = planet->OrbitRadius();
	int orbitIndex = planet->OrbitIndex();

	// Determine which orbit position I am holding around my parent star.
	ipos = 0;
	DaAtomList alplanets;
	pstar->ListPlanets(alplanets);
	for(DaAtomList::const_iterator it = alplanets.begin(); it != alplanets.end(); it++)
	{
		const DaWorld *pb = (const DaWorld*)(*it);
		if(pb->OrbitRadius() < myrad - (FeetPerOrbit()/2.0))
			ipos++;
	}
	ipos += orbitIndex;

	// Rings are denoted with a trailing 'r'.
	return solarSystemName + squal + " " + RomanNumber(ipos+1) + satorbit + "r";
}

// Report a longer description of this Atom for examination purposes.
QString DaWorldRing::Description() const
{
	QString desc;

	// Planet nature and size.
	double rad = FROUND(Location().Length() / FeetPerMeter(), 100);
	desc += tr("Ring %L1m rad").arg(rad);

	// Parent star name
	const DaStar* star = FindParentStarInEnv();
	if(star)
		desc += tr(", %1").arg(star->Name());

	// Orbit zone around parent star
	const QString ozones[] =
	{
		tr("Inferno zone"),
		tr("Inner zone"),
		tr("Habitable zone"),
		tr("Outer zone"),
		tr("Frigid zone")
	};
	desc += tr(", %1").arg(ozones[(int)OrbitZone()]);

	return desc;
}

void DaWorldRing::Radii(double& inside, double& outside) const
{
	m_RingLock.lockForRead();
	if(m_RadInside != 0 && m_RadOutside != 0)
	{
		inside = m_RadInside;
		outside = m_RadOutside;
		m_RingLock.unlock();
		return;
	}
	m_RingLock.unlock();

    // Calculate ring parameters.
    // The ring can't be widacent orbits would touch at the edges.
    const DaWorld* planet = (const DaWorld*)Parent();
    if(!planet)
    {
        // At this size, two rings in adjacent orbits would touch at the edges.
        inside = FeetPerMoonOrbit()/2;
        outside = FeetPerMoonOrbit()/2;
		m_RingLock.lockForWrite();
        m_RadInside = inside;
        m_RadOutside = outside;
		m_RingLock.unlock();
        return;
    }

    // Get information about neighboring orbits. Can I spread out?
    QMap<double, double> moons;         // Key is orbit radius, value is the atmosphere radius.
    planet->ListMoons(moons);           // This list will include me.

    // Using sorted moons map, I can figure out how much to spread toward my neighbors.
    const double myOrbitRad = OrbitRadius();
    QMap<double, double>::const_iterator mi = moons.find(myOrbitRad);           // My position in the map.
    if(mi == moons.cend())
    {
        // Wut?
        inside = FeetPerMoonOrbit()/2;
        outside = FeetPerMoonOrbit()/2;
		m_RingLock.lockForWrite();
        m_RadInside = inside;
        m_RadOutside = outside;
		m_RingLock.unlock();
        return;
    }

	quint32 genSeed = GenSeed();
	AuDice dice;
	dice.Seed(genSeed);

    // Calculate inside radius.
    if(mi == moons.cbegin())
    {
        // I am the closest moon to the parent world.
        // Inside radius can reach almost to the atmosphere.
        inside = myOrbitRad - (planet->AtmosphereRadius() + OrbitAltAboveAtm() + 2000 * FeetPerMeter());
		if(!dynamic_cast<const DaWorldGasGiant*>(planet))
			inside /= 2;
    }
    else
    {
        // I am not the closest moon to the parent world.
        QMap<double, double>::const_iterator ii = mi - 1;
        if(*ii > 0)
        {
            // Next inner satellite is not a ring.
            inside = myOrbitRad - (ii.key() + *ii + OrbitAltAboveAtm() + 2000 * FeetPerMeter());
        }
        else
        {
            // Next inner satellite is a ring.
            inside = myOrbitRad - (ii.key() + myOrbitRad) / 2;
			double gapFactor = (double)dice.Roll(1,101,-1)/100.;
			inside -= 1000 * FeetPerMeter() * gapFactor;			// Leave a gap.
        }
    }

    // Calculate outside radius.
    QMap<double, double>::const_iterator oi = mi + 1;
    if(oi == moons.cend())
    {
        // I am the farthest moon from the parent world.
        outside = (FeetPerOrbit()/2 - 2000 * FeetPerMeter()) - myOrbitRad;
		if(!dynamic_cast<const DaWorldGasGiant*>(planet))
			outside /= 2;											// Avoid the vinyl album appearance.
    }
    else
    {
        // I am not the farthest moon from the parent world.
        if(*oi > 0)
        {
            // Next outer satellite is not a ring.
            outside = (oi.key() - *oi - OrbitAltAboveAtm() - 2000 * FeetPerMeter()) - myOrbitRad;
        }
        else
        {
            // Next outer satellite is a ring.
            outside = (oi.key() + myOrbitRad) / 2 - myOrbitRad;
			double gapFactor = (double)dice.Roll(1,101,-1)/100.;
			outside -= 1000 * FeetPerMeter() * gapFactor;		// Leave a gap.
        }
    }
	m_RingLock.lockForWrite();
	m_RadInside = inside;
	m_RadOutside = outside;
	m_RingLock.unlock();
}

// Create a model of this planet as a ring.
void DaWorldRing::CreateRingModel(AuOpenGLFunctions& GLf)
{
	double inside, outside;
	Radii(inside, outside);

	AuPoint fs = Location();
	double rad = fs.Length();
	double radi = rad - inside;
	double rado = rad + outside;
	AuPoint fi, fo;

	double separate = ::Radius100km(10);	// 1/2 the maximum possible separation between ring layers.
	double sepFactor = (inside + outside) / FeetPerMoonOrbit() / 5;
	separate *= sepFactor;

	double circumf = PI * rado * 2.;	// C = PI*D
	int iarcstrokes = (int)ceil(circumf / 25000);
	if(iarcstrokes < 8)
		iarcstrokes = 8;

	// Configure parameters for generating spindle profile using 1D Perlin noise.
	quint32 genSeed = GenSeed();
	AuDice dice;
	dice.Seed(genSeed);

	double radOffset = (double)dice.Roll(1,100000,-1) + (double)dice.Roll(1,100)/100.;	// Offset into 1D perlin noise

	// Persistence causes jaggier terrain boundaries as it goes up. Persistence also causes the potential magnitude of the Perlin
	// noise function to increase so we must calculate an associated factor to scale the result into a manageable range.
	double persistence = (double)dice.Roll(1, 91, 4)/100.f;

	double freq = .000075;

	// Max Perlin noise due to persistence value at 7 octaves. This factor is needed to scale the return result of the Perlin noise
	// function into a range of -1..1.
	double persFactor = 1.f +				// 1st octave.
		persistence +						// 2nd octave.
		persistence*persistence +			// 3rd octave.
		powf(persistence,3);// +			// 4th octave.
		//powf(persistence,4) +				// 5th octave.
		//powf(persistence,5) +				// 6th octave.
		//powf(persistence,6);				// 7th octave.

	// Create a spindle profile using Perlin noise.
	AuPointArray profile;

	// Integrate the shape in even steps from inside to outside.
	for(double prad=radi; prad<rado; prad += 500)
	{
		// Dampen the curves based on distance from true center.
		double damper;
		if(prad < rad)
		{
			double d = (prad - rad) / inside;	// -1 at the rim. 0 at the rad.
			damper = cos(d * PI/2);
		}
		else
		{
			double d = (prad - rad) / outside;	// 1 at the rim. 0 at the rad.
			damper = cos(d * PI/2);
		}

		double ht;
		if(prad == radi)
			ht = 0;
		else
		{
			double r = prad * freq + radOffset;
			double altFactor = AuNoise::CubicPerlinNoise(4, persistence, r) / persFactor + 1;	// Range 0 to 2.
			ht = separate * altFactor * damper;
		}

		profile.append(AuPoint(prad,0,-ht));
	}
	profile.append(AuPoint(rado,0,0));

	// Now adjust the top up so it doesn't go through the 0 plane and mirror the bottom shape onto the top going back the other way.
	for(int idx=profile.size()-2; idx >= 0; idx--)
	{
		AuPoint p(profile[idx]);
		p.z = -p.z;
		profile.append(p);
	}

	AuMatrix4x4 gridPlane;
	gridPlane.RotateX(PI/2);
	gridPlane.RotateZ(PI/2);

	AuScMesh mesh;
	mesh.MakeSpindle(profile, iarcstrokes, gridPlane);
	AuScSelectInfo sel;
	mesh.FaceIds(sel.m_Faces);

	AuScFaceList faces;
	mesh.Faces(faces);
	faces.TextureProject(sel, 0, inside, AuMatrix4x4());

	AuScTriList faceTris;
	faces.Triangles(faceTris);

	float ringShine = 100;
	QList<AuTriTexNS> tris;
	for(AuScTriList::const_iterator ti = faceTris.cbegin(); ti != faceTris.cend(); ti++)
	{
		AuPointTexNS v0((*ti).V0(), ringShine);
		AuPointTexNS v1((*ti).V1(), ringShine);
		AuPointTexNS v2((*ti).V2(), ringShine);

		AuTriTexNS tri(v0, v1, v2);

		tris.append(tri);
	}

	m_Ring.CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
}

void DaWorldRing::RenderRing(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool viewerIsClose)
{
	Q_UNUSED(projection);
	const DaWorld* world = dynamic_cast<const DaWorld*>(Parent());
	if(!world)
		return;

	// Glow slightly in the color of the parent world's atmosphere.
	QColor atmCol = world->AtmosphereColor();
	float r = atmCol.redF() / 10.f;
	float g = atmCol.greenF() / 10.f;
	float b = atmCol.blueF() / 10.f;

	// Alpha of ring.
	float ringAlpha = .75f;

	re.GLSL().ModelTexShader().Select(projection, re);
	re.GLSL().ModelTexShader().SetModelView(model);
	re.GLSL().ModelTexShader().SetTexture(m_glTexRing);
	re.GLSL().ModelTexShader().SetGlowAlpha(r,g,b, ringAlpha);

	// Ambient light
	float starLight = FindParentSolarSystem()->StarLight();
	re.GLSL().ModelTexShader().SetAmbientLight(starLight, starLight, starLight);
	AuPoint ambientLightUp = EnvLocation();
	ambientLightUp.Unit();
	ambientLightUp = model.ProjectVector(ambientLightUp);
	re.GLSL().ModelTexShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	if(viewerIsClose)
		re.glDisable(GL_CULL_FACE);

	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Render the model.
	m_Ring.SelectGL();
	m_Ring.DrawGL();
	m_Ring.DeselectGL();

	re.glDisable(GL_BLEND);

	if(viewerIsClose)
		re.glEnable(GL_CULL_FACE);

	re.GLSL().ModelTexShader().Deselect();
}

void DaWorldRing::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Render in RenderLast for transparency.
	const AuPoint& pv = rmod.ViewLoc(ParentId());
	double viewDistance = pv.Length();
	if(!rmod.RenderThisPass(pass, viewDistance))
		return;
	renderLast.Insert(pv.Length2(), this);

	// Base atom class needs to render special effects.
	DaWorld::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

bool DaWorldRing::IsLocInRing(const AuPoint& loc) const
{
	double inside, outside;
	Radii(inside, outside);

	double separate = ::Radius100km(10);	// 1/2 the maximum possible separation between ring layers.
	double sepFactor = (inside + outside) / FeetPerMoonOrbit() / 5;
	separate *= sepFactor;
	if(fabs(loc.z) > separate)
		return false;					// loc is above or below the maximum thickness of this ring.

	AuPoint fs = Location();
	double rad = fs.Length();
	double radi = rad - inside;
	double rado = rad + outside;
	double lrad = loc.Length_2D();
	if(lrad < radi || lrad > rado)
		return false;					// loc is inside my inner radius or outside my outer radius.

	return true;
}

void DaWorldRing::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Get scene atom's location relative to my parent.
	AuPoint sloc = Parent()->EnvTransform().UnProject(sceneAtom->EnvLocation());
	bool viewerIsClose = IsLocInRing(sloc);

	RenderRing(projection, rmod.Matrix(ParentId()), *ari.re, viewerIsClose);

	DaAtom::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);
}

// This is an approximation of the period based roughly on Keppler's formula:
//	t^2 = r^3 where t is time and r is radius.
// The return result is in earth years.
double DaWorldRing::Period() const
{
	double r = OrbitRadiusAu() / StandardMoonOrbitRadiusAu();		// ratio of orbit radius to lunar standard
	double r3 = r * r * r;											// r^3
	double lunarMonth = sqrt(r3);									// standard lunar orbit results in period of 1 month
	return lunarMonth / 12.0;										// Return the result in years.
}

void DaWorldRing::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	if(!m_Ring.VAO())
		CreateRingModel(*ari.re);

	if(!m_glTexRing)
	{
		// Textures that look ok on a ring.
		WORLDTEX_TYPE texChoices[] =
		{
			WORLDTEX_CobblestoneLight,
			WORLDTEX_ConcreteDarkModerate,
			WORLDTEX_ConcreteHeavy,
			WORLDTEX_DirtCoarse,
			WORLDTEX_DirtGray,
			WORLDTEX_DirtHeavy,
			WORLDTEX_DirtModerate,
			WORLDTEX_DirtSmooth,
			WORLDTEX_IceModerate,
			WORLDTEX_RockGrayModerate,
			WORLDTEX_RockGraySmooth,
			WORLDTEX_SandSmooth,
			WORLDTEX_SnowModerate,
			WORLDTEX_SupercooledLiquid,
			WORLDTEX_Tarmac,
		};
		int texChoiceCount = sizeof(texChoices)/sizeof(WORLDTEX_TYPE);
		quint32 genSeed = GenSeed();
		AuDice dice;
		dice.Seed(genSeed);
		int texChoice = dice.Roll(1, texChoiceCount, -1);
		m_glTexRing = DaWorldLL::CreateTerrainTexture(*ari.re, texChoices[texChoice], g_Au.GroundTextureSize());
	}

	DaWorld::CreateDeviceObjects(ari);
}

void DaWorldRing::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	//qDebug("DaWorldRing::DeleteDeviceObjects - %d glModel %d",
			//Id().GetID(), m_glModels);
	DaWorld::DeleteDeviceObjects(ari);

	if(m_Ring.VAO())
		m_Ring.DeleteGL();

	m_glTexRing = 0;
}

void DaWorldRing::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	Q_UNUSED(ari);
	Q_UNUSED(sceneAtom);
	Q_UNUSED(lastAnim);
	// Change the world transformation matrix for this planetary body.
	// Planet origin is at its orbit position.
	AuMatrix4x4 tBody;
	double rangle = AngleToPlanet();
	tBody.RotateZ(rangle);								// Rotate the model space to the planet's orbit position.
	tBody.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Translate to the location of this planet relative to the parent star.
	tBody.RotateZ(-rangle);								// Rotate the model space to the planet's orbit position.
	ChangePosAndOrient(AuAtomPos(ParentId(), tBody));
}

bool DaWorldRing::IsTicker() const
{
	if(DaWorld::IsTicker())
		return true;
	return IsInAnyScene() || Transform().IsIdentity();
}

void DaWorldRing::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// Must perform axial rotation in tick so things rotate smoothly with the planet's rotation.
	if(!last.isValid())
		InitSystemPosition();
	else if(IsInAnyScene())
	{
		m_RingLock.lockForRead();
		AuMatrix4x4 tBody(m_ThinkTransform);
		m_RingLock.unlock();
		ChangePosAndOrient(AuAtomPos(ParentId(), tBody));
	}
	DaWorld::Tick(last, now);
}

void DaWorldRing::TickOnClient(const AuElapsedTimer& last)
{
	DaWorld::TickOnClient(last);
	InitSystemPosition();
}

unsigned int DaWorldRing::IsTocker() const
{
	if(IsInAnyScene())
		return 100;
	return DaWorld::IsTocker();
}

void DaWorldRing::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	// Perform axial rotation in tock because tick does not get called when no people or officers are in this system.
	// Axial rotation is needed to calculate sunlight properly.
	if(!IsInAnyScene())
	{
		InitSystemPosition();			// Do the rotation.
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else
	{
		ThinkPerformAxialRotation();	// Calculate the rotation.
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	DaWorld::Tock(last, now, scripts, tockTrans);
}

void DaWorldRing::ThinkPerformAxialRotation()
{
	double rangle = AngleToPlanet();

	// Change the world transformation matrix for this planetary body.
	m_RingLock.lockForWrite();
	m_ThinkTransform.LoadIdentity();

	// Planet origin is at its orbit position.
	m_ThinkTransform.RotateZ(rangle);							// Rotate the model space to the planet's orbit position.
	m_ThinkTransform.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Location of this planet relative to the parent star.
	m_ThinkTransform.RotateZ(-rangle);							// Rotate the model space to the planet's orbit position.
	m_RingLock.unlock();
}

void DaWorldRing::InitSystemPosition()
{
	double rangle = AngleToPlanet();

	// Change the world transformation matrix for this planetary body.
	AuMatrix4x4 tBody;

	// Planet origin is at its orbit position.
	tBody.RotateZ(rangle);								// Rotate the model space to the planet's orbit position.
	tBody.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Translate to the location of this planet relative to the parent star.
	tBody.RotateZ(-rangle);								// Rotate the model space to the planet's orbit position.
	ChangePosAndOrient(AuAtomPos(ParentId(), tBody));

	m_RingLock.lockForWrite();
	m_ThinkTransform = tBody;
	m_RingLock.unlock();
}

void DaWorldRing::QueryNaturalResources(AuZoneResource& res, int resourceZone) const
{
	m_ResourcesLock.lockForRead();
	bool hasRes = m_Resources;
	if(hasRes)
		res = m_Resources[resourceZone];
	m_ResourcesLock.unlock();
	if(hasRes)
		return;

	GALAXY galaxy = Galaxy();
	m_ResourcesLock.lockForWrite();
	if(m_Resources)
	{
		// Somebody beat me to it.
		m_ResourcesLock.unlock();
		return;
	}
	m_Resources = new AuZoneResource[ResourceZones()];

	// Seed the random number generator for consistent values for the planet.
	AuDice dice;
	dice.Seed(GenSeed());
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		// Atmospheric Resources don't vary between zones.
		if(rz == 0)
		{
			// Orbit zone.
			m_Resources[rz].m_AtmType = ATMTYPE_Standard;
			m_Resources[rz].m_Zone = OrbitZone();

			m_Resources[rz].m_Radius100km = 0;
			const DaStar* star = FindParentStarInEnv();
			if(star)
				m_Resources[rz].m_StarType = star->StarType();

			// General atmosphere.
			m_Resources[rz].m_Atmosphere = 0;
			m_Resources[rz].m_AtmosphereQ = dice.RandomQuality();

			// Atmospheric components.
			m_Resources[rz].m_AirQ = dice.RandomQuality();
			m_Resources[rz].m_AntifluxParticleQ = dice.RandomQuality();
			m_Resources[rz].m_BorexinoPrecipQ = dice.RandomQuality();
			m_Resources[rz].m_CryozineQ = dice.RandomQuality();
			m_Resources[rz].m_HydrogenQ = dice.RandomQuality();
			m_Resources[rz].m_IoplasmaQ = dice.RandomQuality();

			// Sunlight in habitable zone = 50
			m_Resources[rz].m_Sunlight = 90 - (OrbitZone() * 20);
			int sunQ = dice.RandomQuality();
			m_Resources[rz].m_SunlightQ = MAX(sunQ, 10);
		}
		else
		{
			// Orbit zone.
			m_Resources[rz].m_AtmType = m_Resources[0].m_AtmType;
			m_Resources[rz].m_Zone = m_Resources[0].m_Zone;

			m_Resources[rz].m_Radius100km = m_Resources[0].m_Radius100km;
			m_Resources[rz].m_StarType = m_Resources[0].m_StarType;

			// General atmosphere.
			m_Resources[rz].m_Atmosphere = m_Resources[0].m_Atmosphere;
			m_Resources[rz].m_AtmosphereQ = m_Resources[0].m_AtmosphereQ;

			// Atmospheric components.
			m_Resources[rz].m_AirQ = m_Resources[0].m_AirQ;
			m_Resources[rz].m_AntifluxParticleQ = m_Resources[0].m_AntifluxParticleQ;
			m_Resources[rz].m_BorexinoPrecipQ = m_Resources[0].m_BorexinoPrecipQ;
			m_Resources[rz].m_CryozineQ = m_Resources[0].m_CryozineQ;
			m_Resources[rz].m_HydrogenQ = m_Resources[0].m_HydrogenQ;
			m_Resources[rz].m_IoplasmaQ = m_Resources[0].m_IoplasmaQ;

			// Sunlight in habitable zone = 50
			m_Resources[rz].m_Sunlight = m_Resources[0].m_Sunlight;
			m_Resources[rz].m_SunlightQ = m_Resources[0].m_SunlightQ;
		}

		// Plant Resources
		{
			// Use average foliage density of terrain that is above water in the zone.
			m_Resources[rz].m_Vegetation = 0;
			m_Resources[rz].m_VegetationQ = dice.RandomQuality();

			// Quality factors are required even though the environment doesn't provide these things.
			m_Resources[rz].m_Beans = 0;
			m_Resources[rz].m_BeansQ = dice.RandomQuality();

			m_Resources[rz].m_Fiber = 0;
			m_Resources[rz].m_FiberQ = dice.RandomQuality();

			m_Resources[rz].m_Fruit = 0;
			m_Resources[rz].m_FruitQ = dice.RandomQuality();

			m_Resources[rz].m_Grain = 0;
			m_Resources[rz].m_GrainQ = dice.RandomQuality();

			m_Resources[rz].m_Grapes = 0;
			m_Resources[rz].m_GrapesQ = dice.RandomQuality();

			m_Resources[rz].m_Herbs = 0;
			m_Resources[rz].m_HerbsQ = dice.RandomQuality();

			m_Resources[rz].m_Hops = 0;
			m_Resources[rz].m_HopsQ = dice.RandomQuality();

			m_Resources[rz].m_Nuts = 0;
			m_Resources[rz].m_NutsQ = dice.RandomQuality();

			m_Resources[rz].m_Spices = 0;
			m_Resources[rz].m_SpicesQ = dice.RandomQuality();

			m_Resources[rz].m_Vegetables = 0;
			m_Resources[rz].m_VegetablesQ = dice.RandomQuality();

			m_Resources[rz].m_Wood = 0;
			m_Resources[rz].m_WoodQ = dice.RandomQuality();
		}

		// Animal Resources
		{
			m_Resources[rz].m_Animals = 0;
			m_Resources[rz].m_AnimalsQ = dice.RandomQuality();
			m_Resources[rz].m_CheeseQ = dice.RandomQuality();
			m_Resources[rz].m_EggsQ = dice.RandomQuality();
			m_Resources[rz].m_FertilizerQ = dice.RandomQuality();
			m_Resources[rz].m_MilkQ = dice.RandomQuality();
			m_Resources[rz].m_Fish = 0;
			m_Resources[rz].m_FishQ = 0;
		}

		// Mineral Resources
		{
			// Stone prevalence reduced by vegetation
			m_Resources[rz].m_Stone = dice.Roll(1,100, -m_Resources[rz].m_Vegetation*2/3);
			m_Resources[rz].m_Stone = MAX(m_Resources[rz].m_Stone, 10);	// >= 10
			m_Resources[rz].m_StoneQ = dice.RandomQuality();

			m_Resources[rz].m_Crystals = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Crystals = MAX(m_Resources[rz].m_Crystals, 2); // >= 2
			m_Resources[rz].m_CrystalsQ = dice.RandomQuality();

			// Eludium is not found on planets with a significant atmosphere.
			if(galaxy == MilkyWay)
				m_Resources[rz].m_Eludium = 0;	// No eludium in Milky Way galaxy.
			else if(m_Resources[rz].m_Atmosphere < 15)
			{
				// This should be fairly abundant.
				m_Resources[rz].m_Eludium = dice.Roll(2,m_Resources[rz].m_Stone, -m_Resources[rz].m_Atmosphere);
				m_Resources[rz].m_Eludium = MAX(m_Resources[rz].m_Eludium, 1); // >= 1
			}
			else
				m_Resources[rz].m_Eludium = 0;	// No eludium here.
			m_Resources[rz].m_EludiumQ = dice.RandomQuality();

			m_Resources[rz].m_Gems = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Gems = MAX(m_Resources[rz].m_Gems, 2);	// >= 2
			m_Resources[rz].m_GemsQ = dice.RandomQuality();

			m_Resources[rz].m_GoldOre = 10 - OrbitZone();	// More common closer to the sun.
			m_Resources[rz].m_GoldOreQ = dice.RandomQuality();

			m_Resources[rz].m_Ice = 100;
			if(OrbitZone() == OZONE_Inferno)
				m_Resources[rz].m_Ice = 0;
			m_Resources[rz].m_IceQ = dice.RandomQuality();

			// Lumenite is rare.
			if(galaxy == MilkyWay)
				m_Resources[rz].m_Lumenite = 0;	// No lumenite in Milky Way galaxy.
			else
			{
				// Prevalence increases as orbit zone increases.
				// Prevalence is reduced by planet size.
				int lumenite = OrbitZone()*2;	// lumenite<2 = notpresent
				lumenite += 2;				// Present in inferno zone and outward. Oz0 *2 +2 = 2. Res: 2, 4, 6, 8, 10
				if(lumenite > 1)
					m_Resources[rz].m_Lumenite = dice.Roll(2,lumenite,-2);	// Bell curve so zero is less common when possible.
				else
					m_Resources[rz].m_Lumenite = 0;
			}
			m_Resources[rz].m_LumeniteQ = dice.RandomQuality();

			m_Resources[rz].m_Minerals = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Minerals = MAX(m_Resources[rz].m_Minerals, 2);
			m_Resources[rz].m_MineralsQ = dice.RandomQuality();

			m_Resources[rz].m_Ore = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Ore = MAX(m_Resources[rz].m_Ore, 2);
			m_Resources[rz].m_OreQ = dice.RandomQuality();

			m_Resources[rz].m_Radioactives = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Radioactives = MAX(m_Resources[rz].m_Radioactives, 2);
			m_Resources[rz].m_RadioactivesQ = dice.RandomQuality();

			// Water is considered to be lava or supercooled liquid in inferno and frigid environments respectively.
			// The quality does not vary by resource zone.
			m_Resources[rz].m_WaterInEnvironment = 0;
			m_Resources[rz].m_WaterInEnvironmentQ = dice.RandomQuality();

			// Nonessential specialty mineral resources with quantity based on hydrographics.
			m_Resources[rz].m_MagmexQ = dice.RandomQuality();
			m_Resources[rz].m_MyrathaneQ = dice.RandomQuality();
			if(rz != 0)
			{
				// These are overridden in this way rather than no rolling them so as not to upset the current resource balance
				// in the existing terrain.
				m_Resources[rz].m_WaterInEnvironmentQ = m_Resources[0].m_WaterInEnvironmentQ;

				// Nonessential specialty mineral resources with quantity based on hydrographics.
				m_Resources[rz].m_MagmexQ = m_Resources[0].m_MagmexQ;
				m_Resources[rz].m_MyrathaneQ = m_Resources[0].m_MyrathaneQ;
			}

			// Nonessential specialty mineral resources that exist as visible resources in the environment.
			m_Resources[rz].m_BoliteQ = dice.RandomQuality();
			m_Resources[rz].m_PhlogistonQ = dice.RandomQuality();
			m_Resources[rz].m_PolytarideQ = dice.RandomQuality();
			m_Resources[rz].m_VulcaniteQ = dice.RandomQuality();
			{
				// These resources are never found on rings.
				m_Resources[rz].m_Bolite = 0;
				m_Resources[rz].m_Phlogiston = 0;
				m_Resources[rz].m_Polytaride = 0;
				m_Resources[rz].m_Vulcanite = 0;
			}
		}

		// No hydrocarbon resources.
		m_Resources[rz].m_Coal = 0;
		m_Resources[rz].m_CoalQ = dice.RandomQuality();

		m_Resources[rz].m_CrudeOil = 0;
		m_Resources[rz].m_CrudeOilQ = dice.RandomQuality();

		m_Resources[rz].m_NaturalGas = 0;
		m_Resources[rz].m_NaturalGasQ = dice.RandomQuality();
	}

	res = m_Resources[resourceZone];
	m_ResourcesLock.unlock();
}

void DaWorldRing::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float /*scannerWeaponRange*/, int scanContent,
		const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& /*stanceCache*/) const
{
	Q_UNUSED(scannerId);
	Q_UNUSED(scannerEmpire);
	Q_UNUSED(scannerDomain);
	Q_UNUSED(scannerWorld);
	// Must scan me if looking for locked-on blip and I'm it.
	bool scanMe = true;
	if((!(scanContent & 0x0008) || !Is(lockedBlip)))
	{
		// Not looking for locked-on blip or I am not the locked-on blip.
		// I am only scanned for sensor consoles, fire control consoles, transporter consoles, and AI pilots.
		if(!(scanContent & 0x0017))
			scanMe = false;
	}

	if(scanMe)
	{
		// Calculate closest point on ring from scanner, for sensor range scan.
		const DaAtom* parent = Parent();
		if(!parent)
			return;
		AuPoint sysLoc = parent->EnvLocation();
		double ringRad = Location().Length();		// Radius of this ring.
		AuPoint onRing = scannerLoc.ClosestPointOnCircle(sysLoc, ringRad, AuPoint(0,0,1));	// Closest point on ring
		double blipRange = onRing.Length();			// Distance to closest point on ring.
		bool inSensorRange = blipRange <= scannerSensorRange;

		// A ring reports the center of the parent planet. This makes drawing the ring on the console easier.
		blips.Add(AuBlipRing(ringRad, inSensorRange, Id(), sysLoc));
	}
}

// Insert first time database record.
void DaWorldRing::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	// Storing the solar system id with planets makes the empire status report queries a lot faster.
	// The id of the parent solar system never changes so we don't update it ever again.
	AuId systemId;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(system)
		systemId = system->Id();

	QSqlQuery qa(QString(), db);

	// Assemble an INSERT query.
	QString fields("id,idp, locX,locY,locZ, rotX,rotY,rotZ, timeCreate,timeModified,timeTock,timeDeath, "
			"ids,name,orbitRadius,orbitZone");
	QString qmarks("?,?, ?,?,?, ?,?,?, ?,?,?,?, ?,?,?,?");

	// Build the full query out of the pieces;
	QString sq = QString("INSERT INTO a_WorldRing (") + fields + ") VALUES (" + qmarks + ");";
	qa.prepare(sq);
	qa.addBindValue(Id().GetID());
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(CreatedTime().UTC());
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// a_World fields
	qa.addBindValue(systemId.GetID());
	qa.addBindValue(Name());
	qa.addBindValue(OrbitRadiusAu());
	qa.addBindValue(OrbitZone());

	if(!qa.exec())
		qWarning("DaWorldRing::Insert %s FAILED. %s sq '%s'",
				(const char*)Name().toLatin1(),
				(const char*)qa.lastError().text().toLatin1(),
				(const char*)sq.toLatin1());
}

// Update existing database record.
void DaWorldRing::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq("UPDATE a_WorldRing SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, timeModified=?, "
			"timeTock=?, timeDeath=?, "
			"name=?");
	sq += QString(" WHERE id=%1;").arg(Id().GetID());

	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// a_World fields
	qa.addBindValue(Name());

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaWorldRing::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaWorldRing::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully
// loaded.
DaWorldRing* DaWorldRing::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS /*loadWhat*/,
		const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ, timeCreate,timeModified,timeTock,timeDeath, "
			"name,orbitRadius,orbitZone "
			"FROM a_WorldRing WHERE ");
	if(!idToLoad.IsNull())
		sq += QString("id=%1;").arg(idToLoad.GetID());
	else
		sq += QString("idp=%1;").arg(parentId.GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaWorldRing *pnu = new DaWorldRing;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
			quint32 id = qa.value(qv++).toUInt();
			pnu->SetId(AuId(id));
			double vx = qa.value(qv++).toDouble();
			double vy = qa.value(qv++).toDouble();
			double vz = qa.value(qv++).toDouble();
			double rx = qa.value(qv++).toDouble();
			double ry = qa.value(qv++).toDouble();
			double rz = qa.value(qv++).toDouble();
			AuAtomPos pos(parentId, vx,vy,vz, rx,ry,rz);
			pnu->SetPosAndOrient(pos);

			QDateTime tmCreated = qa.value(qv++).toDateTime();
			QDateTime tmModify = qa.value(qv++).toDateTime();
			QDateTime tmTocked = qa.value(qv++).toDateTime();
			QDateTime tmDeath = qa.value(qv++).toDateTime();
			tmCreated.setTimeSpec(Qt::UTC);
			tmModify.setTimeSpec(Qt::UTC);
			tmTocked.setTimeSpec(Qt::UTC);
			tmDeath.setTimeSpec(Qt::UTC);
			QDateTime now(QDateTime::currentDateTime());
			now.setTimeSpec(Qt::UTC);
			if(tmCreated > now)
				tmCreated = now;
			if(tmModify > now)
				tmModify = now;
			if(tmTocked > now)
				tmTocked = now;
			pnu->SetCreatedTime(tmCreated);
			pnu->SetModifyTime(tmModify);
			pnu->SetTockTime(tmTocked);
			pnu->SetDeathTime(tmDeath);

			// a_World fields
			pnu->SetName(qa.value(qv++).toString());
			pnu->SetOrbitRadiusAu(qa.value(qv++).toDouble());
			pnu->m_OrbitZone = (ORBITZONE)qa.value(qv++).toInt();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			// We never get here if we're the SceneMaster.
			// The SceneManager needs planets and moons.
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

int DaWorldRing::OkToRename(quint32 empireId) const
{
	const DaWorld* world = dynamic_cast<const DaWorld*>(Parent());
	if(!world)
		return 0;

	// Ring can be renamed if the parent world can be renamed.
	// NOTE: This is why gas giants do not test rings in their OkToRename function.
	return world->OkToRename(empireId);
}

void DaWorldRing::PrepareForScan()
{
	DaWorld::PrepareForScan();
	InitSystemPosition();
}

QString DaWorldRing::SystemScanReport(int scanEx) const
{
	QString sOrbitZone;
	QString sLiquid;
	switch(OrbitZone())
	{
	case OZONE_Inferno:
		sOrbitZone = QObject::tr("Inferno Zone");
		sLiquid = QObject::tr("Lava");
		break;
	case OZONE_Inner:
		sOrbitZone = QObject::tr("Inner Zone");
		sLiquid = QObject::tr("Water");
		break;
	case OZONE_Habitable:
		sOrbitZone = QObject::tr("Habitable Zone");
		sLiquid = QObject::tr("Water");
		break;
	case OZONE_Outer:
		sOrbitZone = QObject::tr("Outer Zone");
		sLiquid = QObject::tr("Water");
		break;
	case OZONE_Frigid:
		sOrbitZone = QObject::tr("Frigid Zone");
		sLiquid = QObject::tr("Liquid");
		break;
	case OZONE_DeepSpace:
		sOrbitZone = QObject::tr("Deep Space");
		sLiquid = QObject::tr("Liquid");
		break;
	}

	QString sSize;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	{
		sBodyType = tr("Ring");
		double rad = Location().Length();			// Radius in feet.
		rad /= FeetPerMeter();			// Convert to meters.
		rad = round(rad/100) * 100;		// Round to nearest 100 meters.
		sSize = QObject::tr("%L1m Radius").arg(rad);

		double inside, outside;
		Radii(inside, outside);
		double rwidth = inside + outside;
		rwidth /= FeetPerMeter();
		rwidth = round(rwidth/100) * 100;	// Round to nearest 100 meters.
		sSize += "<br>" + QObject::tr("%L1m Width").arg(rwidth);
	}

	// Make a list of the natural resources the scanner can detect.
	QMap<QString, AuWorldRes_> atmlist;			// Atmosphere resources.
	QMap<QString, AuWorldRes_> biolist;			// Biosphere resources.
	QMap<QString, AuWorldRes_> geolist;			// Geosphere resources.
	QMap<QString, AuWorldRes_> hydlist;			// Geosphere resources.
	if(scanEx == 2)
	{
		// Must scan every resource zone.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> atmcount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> biocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> geocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> hydcount;	// Count of total between all zones. We will remove the zero ones at the end.
		for(int rz=0; rz<ResourceZones(); rz++)
		{
			QString cname;
			AuZoneResource res;
			QueryNaturalResources(res, rz);

			if(rz == 0)
			{
				// These resources exist in only a single planet-wide zone.
				// There is no need to repeat the information in multiple zones.

				// Atmosphere
				COMMODITYID atmres[] =
				{
					CID_Air,
					CID_AntifluxParticles,
					CID_BorexinoPrecipitate,
					CID_Cryozine,
					CID_Hydrogen,
					CID_Ioplasma,
				};
				int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, atmres, atmrescount, atmcount, atmlist);

				// Hydrosphere
				COMMODITYID hydres[] =
				{
					CID_Hydrographics,
					CID_Magmex,
					CID_Myrathane,
					CID_Flomentum,
				};
				int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, hydres, hydrescount, hydcount, hydlist);
			}

			// Remaining resources can exist in multiple zones.
			// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
			// To do this, we'll report all of them and keep count of the total probability.
			// At the end, we'll remove the ones that ended up with zero total probability.

			// Geosphere
			COMMODITYID geores[] =
			{
				CID_Bolite,
				CID_Coal,
				CID_CrudeOil,
				CID_Crystals,
				CID_Eludium,
				CID_Gems,
				CID_GoldOre,
				CID_Ice,
				CID_Lumenite,
				CID_Minerals,
				CID_NaturalGas,
				CID_Ore,
				CID_Phlogiston,
				CID_Polytaride,
				CID_Radioactives,
				CID_Stone,
				CID_Vulcanite,
				CID_Adamantite,
				CID_Viathol,
			};
			int georescount = sizeof(geores)/sizeof(COMMODITYID);
			ReportZoneFetchResources(rz, res, geores, georescount, geocount, geolist);
		}

		// Remove the ones that ended up with zero total probability.
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = atmcount.begin(); bci != atmcount.end(); bci++)
			if(!(*bci).second)
				atmlist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = biocount.begin(); bci != biocount.end(); bci++)
			if(!(*bci).second)
				biolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = geocount.begin(); bci != geocount.end(); bci++)
			if(!(*bci).second)
				geolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = hydcount.begin(); bci != hydcount.end(); bci++)
			if(!(*bci).second)
				hydlist.remove(g_Au.CommodityName((*bci).first));
	}

	// Orbital position.
	QString sOrbiting;
	{
		double orbitAngle = PI/2 - Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(Parent()->Name())
			.arg(AuSensorBlip::FormatBigNumber(Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	// Put it all together.
	QString rpt = QString("<html><big><b>%1</b></big><p>").arg(Name());

	// Show page of general info about the planet.
	rpt += sOrbiting + "<br>";
	rpt += sOrbitZone;

	// Geosphere
	rpt += QObject::tr("<hr><b>Geosphere</b><br>");
	rpt += sBodyType + ", " + sSize;
	if(!geolist.isEmpty())
	{
		rpt += "<br><table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
		rpt += "<tbody>";
		{
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality and Abundance</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				QString r0;
				if((*ri).prob[0])
					r0 = QString("Q%1 %2%").arg((*ri).qual[0]).arg((*ri).prob[0]);
				else
					r0 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0);
			}
		}
		rpt += "</tbody></table>";
	}

	// Hydrosphere
	if(!sWater.isEmpty())
	{
		rpt += QObject::tr("<p><hr><b>Hydrosphere</b><br>");
		rpt += sWater;
		if(!hydlist.isEmpty())
		{
			rpt += "<br><table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
			rpt += "<tbody>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality and Abundance</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = hydlist.begin(); ri != hydlist.end(); ri++)
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>Q%3 %4%</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg((*ri).qual[0])
					.arg((*ri).prob[0]);
			rpt += "</tbody></table>";
		}
	}

	// Atmosphere
	if(!sAtm.isEmpty() || !atmlist.isEmpty())
	{
		rpt += QObject::tr("<p><hr><b>Atmosphere</b>");
		if(!sAtm.isEmpty())
			rpt += "<br>" + sAtm;
		if(!atmlist.isEmpty())
		{
			rpt += "<br><table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
			rpt += "<tbody>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality and Abundance</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = atmlist.begin(); ri != atmlist.end(); ri++)
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>Q%3 %4%</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg((*ri).qual[0])
					.arg((*ri).prob[0]);
			rpt += "</tbody></table>";
		}
	}

	rpt += "</html>";
	return rpt;
}

void DaWorldRing::SystemScanReportXML(QXmlStreamWriter& stream, int scanEx) const
{
	QString sOrbitZone;
	QString sLiquid;
	switch(OrbitZone())
	{
	case OZONE_Inferno:
		sOrbitZone = QObject::tr("Inferno Zone");
		sLiquid = QObject::tr("Lava");
		break;
	case OZONE_Inner:
		sOrbitZone = QObject::tr("Inner Zone");
		sLiquid = QObject::tr("Water");
		break;
	case OZONE_Habitable:
		sOrbitZone = QObject::tr("Habitable Zone");
		sLiquid = QObject::tr("Water");
		break;
	case OZONE_Outer:
		sOrbitZone = QObject::tr("Outer Zone");
		sLiquid = QObject::tr("Water");
		break;
	case OZONE_Frigid:
		sOrbitZone = QObject::tr("Frigid Zone");
		sLiquid = QObject::tr("Liquid");
		break;
	case OZONE_DeepSpace:
		sOrbitZone = QObject::tr("Deep Space");
		sLiquid = QObject::tr("Liquid");
		break;
	}

	QString sSize;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	{
		sBodyType = tr("Ring");
		double rad = Location().Length();			// Radius in feet.
		rad /= FeetPerMeter();			// Convert to meters.
		rad = round(rad/100) * 100;		// Round to nearest 100 meters.
		sSize = QObject::tr("%1m Radius").arg(rad);

		double inside, outside;
		Radii(inside, outside);
		double rwidth = inside + outside;
		rwidth /= FeetPerMeter();
		rwidth = round(rwidth/100) * 100;	// Round to nearest 100 meters.
		sSize += "<br>" + QObject::tr("%1m Width").arg(rwidth);
	}

	// Make a list of the natural resources the scanner can detect.
	QMap<QString, AuWorldRes_> atmlist;			// Atmosphere resources.
	QMap<QString, AuWorldRes_> biolist;			// Biosphere resources.
	QMap<QString, AuWorldRes_> geolist;			// Geosphere resources.
	QMap<QString, AuWorldRes_> hydlist;			// Geosphere resources.
	if(scanEx == 2)
	{
		// Must scan every resource zone.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> atmcount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> biocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> geocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> hydcount;	// Count of total between all zones. We will remove the zero ones at the end.
		for(int rz=0; rz<ResourceZones(); rz++)
		{
			QString cname;
			AuZoneResource res;
			QueryNaturalResources(res, rz);

			if(rz == 0)
			{
				// These resources exist in only a single planet-wide zone.
				// There is no need to repeat the information in multiple zones.

				// Atmosphere
				COMMODITYID atmres[] =
				{
					CID_Air,
					CID_AntifluxParticles,
					CID_BorexinoPrecipitate,
					CID_Cryozine,
					CID_Hydrogen,
					CID_Ioplasma,
				};
				int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, atmres, atmrescount, atmcount, atmlist);

				// Hydrosphere
				COMMODITYID hydres[] =
				{
					CID_Hydrographics,
					CID_Magmex,
					CID_Myrathane,
					CID_Flomentum,
				};
				int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, hydres, hydrescount, hydcount, hydlist);
			}

			// Remaining resources can exist in multiple zones.
			// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
			// To do this, we'll report all of them and keep count of the total probability.
			// At the end, we'll remove the ones that ended up with zero total probability.

			// Geosphere
			COMMODITYID geores[] =
			{
				CID_Bolite,
				CID_Coal,
				CID_CrudeOil,
				CID_Crystals,
				CID_Eludium,
				CID_Gems,
				CID_GoldOre,
				CID_Ice,
				CID_Lumenite,
				CID_Minerals,
				CID_NaturalGas,
				CID_Ore,
				CID_Phlogiston,
				CID_Polytaride,
				CID_Radioactives,
				CID_Stone,
				CID_Vulcanite,
				CID_Adamantite,
				CID_Viathol,
			};
			int georescount = sizeof(geores)/sizeof(COMMODITYID);
			ReportZoneFetchResources(rz, res, geores, georescount, geocount, geolist);
		}

		// Remove the ones that ended up with zero total probability.
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = atmcount.begin(); bci != atmcount.end(); bci++)
			if(!(*bci).second)
				atmlist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = biocount.begin(); bci != biocount.end(); bci++)
			if(!(*bci).second)
				biolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = geocount.begin(); bci != geocount.end(); bci++)
			if(!(*bci).second)
				geolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = hydcount.begin(); bci != hydcount.end(); bci++)
			if(!(*bci).second)
				hydlist.remove(g_Au.CommodityName((*bci).first));
	}

	// Orbital position.
	QString sOrbiting;
	{
		double orbitAngle = PI/2 - Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(Parent()->Name())
			.arg(AuSensorBlip::FormatBigNumber(Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	// Put it all together.
	stream.writeStartElement("planet");
	stream.writeAttribute("planetId", Id().StringId());
	stream.writeAttribute("name", Name());
	stream.writeAttribute("bodyType", sBodyType);

	// Show page of general info about the planet.
	stream.writeAttribute("orbit", sOrbiting);
	stream.writeAttribute("zone", sOrbitZone);

	// Geosphere
	stream.writeStartElement("geosphere");
	stream.writeAttribute("diameter", sSize);
	stream.writeAttribute("resourceZones", QString("%1").arg(ResourceZones()));
	if(!geolist.isEmpty())
	{
		for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
		{
			stream.writeStartElement("resource");
			stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
			stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
			stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
			stream.writeEndElement();
		}
	}
	stream.writeEndElement();

	// Hydrosphere
	if(!sWater.isEmpty())
	{
		stream.writeStartElement("hydrosphere");
		stream.writeAttribute("surfaceLiquid", sWater);
		if(!hydlist.isEmpty())
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = hydlist.begin(); ri != hydlist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeEndElement();
			}
		}
		stream.writeEndElement();
	}

	// Atmosphere
	if(!sAtm.isEmpty() || !atmlist.isEmpty())
	{
		stream.writeStartElement("atmosphere");
		if(!sAtm.isEmpty())
			stream.writeAttribute("composition", sAtm);
		if(!atmlist.isEmpty())
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = atmlist.begin(); ri != atmlist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeEndElement();
			}
		}
		stream.writeEndElement();
	}

	stream.writeEndElement();
}

void DaWorldRing::FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& shipSysLoc, float gunRange,
		const DaAtom*& targetAtom, double& targetRange) const
{
	if(!CanBeHarvested(harvesterModule))
		return;

	// Perform range test.
	AuPoint targetVec = WeaponLockLocation(shipSysLoc, harvesterModule) - shipSysLoc;	// Vector to target from ship's location.
	double range = targetVec.Length();
	if(range > gunRange)
		return;

	if(targetAtom && range > targetRange)
		return;		// I am not closer than the current target.

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = range;
}

AuPoint DaWorldRing::WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const
{
	Q_UNUSED(harvesterModule);
	// Ring calculates closest point on centerline of ring.
	double ringRad = Location().Length();		// Radius of this ring.
	DaAtom* parent = Parent();
	if(!parent)
		return AuPoint();
	double shipRad = shipSysLoc.Distance_2D(parent->EnvLocation());	// Ship's 2D distance from parent world.

	double inside, outside;
	Radii(inside, outside);

	double harvestRad;
	if(shipRad < ringRad - inside)
		harvestRad = ringRad - inside;
	else if(shipRad > ringRad + outside)
		harvestRad = ringRad + outside;
	else
		harvestRad = shipRad;

	AuPoint onRing = shipSysLoc.ClosestPointOnCircle(parent->EnvLocation(), harvestRad, AuPoint(0,0,1));
	//qDebug() << "DaWorldRing::WeaponLockLocation" << Id() << Name() << "shipLoc" << shipSysLoc << "cen" << parent->EnvLocation()
		//<< "rad" << harvestRad << "onRing" << onRing << "dist" << onRing.Distance(shipSysLoc);
	return onRing;
}

void DaWorldRing::SystemScanResources(QMultiMap<quint8, COMMODITYID>& atmRes, QMultiMap<quint8, COMMODITYID>& hydRes,
		QMultiMap<quint8, COMMODITYID>& geoRes, QMultiMap<quint8, COMMODITYID>& bioRes) const
{
	Q_UNUSED(bioRes);

	// Must scan every resource zone.
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		AuZoneResource res;
		QueryNaturalResources(res, rz);

		if(rz == 0)
		{
			// These resources exist in only a single planet-wide zone.
			// There is no need to repeat the information in multiple zones.

			// Atmosphere
			COMMODITYID atmres[] =
			{
				CID_Air,
				CID_AntifluxParticles,
				CID_BorexinoPrecipitate,
				CID_Cryozine,
				CID_Hydrogen,
				CID_Ioplasma,
			};
			int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
			ReportZoneFetchResources(res, atmres, atmrescount, atmRes);

			// Hydrosphere
			COMMODITYID hydres[] =
			{
				CID_Hydrographics,
				CID_Magmex,
				CID_Myrathane,
				CID_Flomentum,
			};
			int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
			ReportZoneFetchResources(res, hydres, hydrescount, hydRes);
		}

		// Remaining resources can exist in multiple zones.
		// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
		// To do this, we'll report all of them and keep count of the total probability.
		// At the end, we'll remove the ones that ended up with zero total probability.

		// Geosphere
		COMMODITYID geores[] =
		{
			CID_Bolite,
			CID_Coal,
			CID_CrudeOil,
			CID_Crystals,
			CID_Eludium,
			CID_Gems,
			CID_GoldOre,
			CID_Ice,
			CID_Lumenite,
			CID_Minerals,
			CID_NaturalGas,
			CID_Ore,
			CID_Phlogiston,
			CID_Polytaride,
			CID_Radioactives,
			CID_Stone,
			CID_Vulcanite,
			CID_Adamantite,
			CID_Viathol,
		};
		int georescount = sizeof(geores)/sizeof(COMMODITYID);
		ReportZoneFetchResources(res, geores, georescount, geoRes);
	}
}

quint8 DaWorldRing::NaturalResourceQuality(COMMODITYID cid, int resourceZone) const
{
	quint8 q = 0;
	m_ResourcesLock.lockForRead();
	if(m_Resources)
		q = m_Resources[resourceZone].Quality(cid);
	m_ResourcesLock.unlock();
	if(q)
		return q;

	AuZoneResource res;
	QueryNaturalResources(res, resourceZone);
	return res.Quality(cid);
}

int DaWorldRing::AirTravellerInterval(qint8 morale) const
{
	Q_UNUSED(morale);
	return 0;
}

int DaWorldRing::IndigenousPopulationInterval(qint8 morale) const
{
	Q_UNUSED(morale);
	return 0;
}

quint64 DaWorldRing::MemoryUsage() const
{
	quint64 mem = DaWorld::MemoryUsage();
	mem += sizeof(m_RingLock);
	mem += sizeof(m_ThinkTransform);
	mem += sizeof(m_ResourcesLock);
	mem += sizeof(m_Resources);
	if(m_Resources)
		mem += sizeof(AuZoneResource) * ResourceZones();
	mem += sizeof(m_Ring);
	mem += sizeof(m_glTexRing);
	return mem;
}

DaWorldRing* DaWorldRing::FindRing(const AuPoint& searchLoc) const
{
	Q_UNUSED(searchLoc);
	return 0;
}

DaAtom::ATOM_TYPE DaWorldRing::AtomType() const
{
	return ATOM_WorldRing;
}

void DaWorldRing::operator>>(QDataStream& out) const
{
	DaWorld::operator>>(out);
	m_RingLock.lockForRead();
	out << m_RadInside;
	out << m_RadOutside;
	m_RingLock.unlock();
}

void DaWorldRing::operator<<(QDataStream& in)
{
	DaWorld::operator<<(in);
	m_RingLock.lockForWrite();
	in >> m_RadInside;
	in >> m_RadOutside;

	m_ThinkTransform = Transform();
	m_RingLock.unlock();
}

void DaWorldRing::ParentDestroyedByAsteroid(DaAtom* nuParent, quint8 orbitIndex, double orbitRadiusAu)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	Q_UNUSED(nuParent);			// I won't become a planetoid.
	Q_UNUSED(orbitIndex);		// I won't become a planetoid.
	Q_UNUSED(orbitRadiusAu);	// I won't become a planetoid.
    // Scatter some little rocks around where I was.
    AuDice dice;
    dice.Seed(GenSeed());
    double rwidth = (double)dice.Roll(1,80,10) / 100.0; // 11 to 90%
    rwidth *= FeetPerMoonOrbit();

    AuPoint fs = Location();
    double rad = fs.Length();
    double circumf = PI * rad * 2.; // C = PI*D
    int iarcstrokes = (int)ceil(circumf / rwidth);
    if(iarcstrokes < 8)
        iarcstrokes = 8;
    int rocks = iarcstrokes;
    rwidth /= 2.0;              // Need half the width to calc the centerline.

    for(int rock = 0; rock < rocks; rock++)
    {
        // Pick a random direction.
        AuPoint godir = AuParticleSystem::GetRandomVector();
        AuMatrix4x4 troid;                      // Identity matrix is the center of my parent world.
        double rockrad = rad + rwidth * g_Au.Dice().Roll(1,201,-101) / 100.;    // +/- 1-100% of the ring radius
        troid.Translate(godir * rockrad);
		int asize = g_Au.Dice().Roll(1,7,5);	// 6 to 12, 5 less than a size 1 moon at 17.

        DaAsteroid* roid = new DaAsteroid(asize);
		roid->ConnectSignals();
		roid->Create();
		roid->SetPosAndOrient(AuAtomPos(Id(), troid));
		ether->AddAtom(roid);

        g_Au.AtomAdded(roid->Id());
        roid->SetNormal();
        roid->SetInMotion();
		roid->TickTock();					// Get it moving right away.
    }
    Kill();
}

float DaWorldRing::AsteroidBeltDensityRecursive(const AuPoint& envLoc) const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	AuPoint loc = parent->EnvTransform().UnProject(envLoc);	// Into the space of my parent world.

	double inside, outside;
	Radii(inside, outside);

	double separate = ::Radius100km(10);	// 1/2 the maximum possible separation between ring layers.
	double sepFactor = (inside + outside) / FeetPerMoonOrbit() / 5;
	separate *= sepFactor;
	separate += 1000*FeetPerMeter();		// Let gravel spread farther above/below than the visible ring.
	if(fabs(loc.z) > separate)
		return 0;						// loc is above or below the maximum thickness of this ring.

	AuPoint fs = Location();
	double rad = fs.Length();
	double radi = rad - inside;
	double rado = rad + outside;
	double lrad = loc.Length_2D();
	if(lrad < radi || lrad > rado)
		return 0;						// loc is inside my inner radius or outside my outer radius.

	// Calculate density.
	return fabs(loc.z)/separate;
}

