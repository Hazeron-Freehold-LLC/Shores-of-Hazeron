//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auwormhole.cpp,v 1.60 2022/02/19 17:41:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auwormhole.h"
//
#include "au3dsglsl.h"
#include "auglobal.h"
#include "augleffects.h"
#include "aumatrix4x4.h"
#include "auplane.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "pswormhole.h"
#include <QTime>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuWormholeMetaType = qRegisterMetaType<AuWormhole>();

//static GLuint g_Models = 0;	// The GL models and textures can be reused by every
static GLuint g_TexPos = 0;		// instance so the values are stored statically.
static GLuint g_TexNeg = 0;
static GLuint g_TexInt = 0;
static GLuint g_TexParticles;	// Id of texture for particles
PsWormhole*	s_pParticles[AuWormhole::POLARITY_Count-1];			// Particle systems for wormhole effects
Au3dsGLSLModelInstance*	s_Wormhole;		// Model of wormhole
float falloffDist = .00075f;

static double g_Rot = 0;

void AuWormhole::operator<<(QDataStream& in)
{
	in >> m_ExploredBy;		// Empire ids that have explored this wormhole.
	in >> m_Name;			// Name of wormhole if explored.
	qint8 i8;
	in >> i8;
	m_Polarity = (POLARITY)i8;
	in >> m_ToSystem;		// Id of destination solar system.
	in >> m_VecToDest;		// Vector to destination in parsecs.
}

void AuWormhole::operator>>(QDataStream& out) const
{
	out << m_ExploredBy;	// Empire ids that have explored this wormhole.
	out << m_Name;			// Name of wormhole if explored.
	out << (qint8)m_Polarity;
	out << m_ToSystem;		// Id of destination solar system.
	out << m_VecToDest;		// Vector to destination in parsecs.
}

void AuWormhole::Rx(QDataStream& in)
{
	in >> m_ExploredBy;		// Empire ids that have explored this wormhole.
	in >> m_Loc;
	in >> m_Name;			// Name of wormhole if explored.
	qint8 i8;
	in >> i8;
	m_Polarity = (POLARITY)i8;
	in >> m_ToSystem;		// Id of destination solar system.
	in >> m_VecToDest;		// Vector to destination in parsecs.
}

void AuWormhole::Tx(QDataStream& out) const
{
	out << m_ExploredBy;	// Empire ids that have explored this wormhole.
	out << m_Loc;
	out << m_Name;			// Name of wormhole if explored.
	out << (qint8)m_Polarity;
	out << m_ToSystem;		// Id of destination solar system.
	out << m_VecToDest;		// Vector to destination in parsecs.
}

// Returns angle of entry off perpendicular, 0 to PI. 0 is straight in. PI is
// exactly backward. -1 is returned if the segment does not intersect this
// wormhole.
float AuWormhole::CheckEntry(const DaSolarSystem* system, const AuPoint &oldloc, const AuPoint &nuloc) const
{
	// Check distance above/below the plane of the wormhole.
	AuPoint sysLoc = Location(system);
	AuPlane plane(sysLoc, -m_Loc);
	AuPoint intersect;
	if(!plane.SegmentIntersection(oldloc, nuloc, intersect))
	{
		//if(IsStargate())
			//qDebug() << "Move segment does not intersect plane of" <<
				//(m_Polarity == StargatePositive ? "Positive Stargate":"Negative Stargate")
				//<< "oldloc" << oldloc << "nuloc" << nuloc << "sysLoc" << sysLoc << "-m_Loc" << -m_Loc
				//<< "plane" << plane.N << plane.d;
		// Sometimes we miss the plane intersection at wormholes around suns orbiting large black holes.
		// This code fixes that.
		if(Polarity() == Positive || Polarity() == Negative || Polarity() == StargatePositive || Polarity() == StargateNegative ||
				Polarity() == IntergalacticOneWay)
		{
			AuPoint movevec = nuloc - oldloc;
			movevec.Unit();
			float entryAngle = movevec.AngleTo(plane.N);
			if(entryAngle < 45.f*DEG2RADf)
			{
				double rd0 = sysLoc.Distance(oldloc);
				double rd1 = sysLoc.Distance(nuloc);
				double obstacleRad = ObstacleRadius();
				if(rd0 < obstacleRad && rd1 < obstacleRad)
				{
					//qDebug() << "AuWormhole::CheckEntry no segment intersection but this is close enough" << rd0 << rd1
					//<< "entryAngle" << (entryAngle*RAD2DEGf);
					return entryAngle;
				}
			}
		}
		return -1;	// Move segment does not intersect plane.
	}

	// Check distance from center of wormhole.
	// Limits are based on wormhole polarity.
	double rd = sysLoc.Distance(intersect);
	if(rd > ObstacleRadius())
	{
		//if(IsStargate())
		//qDebug("Too far from center of %s stargate to %d at %g, obstacle rad %g",
				//m_Polarity == StargatePositive ? "Positive":"Negative",
				//m_ToSystem.GetID(),
				//rd,
				//ObstacleRadius());
		return -1;				// Too far from center of wormhole.
	}

	// Calculate the angle the vehicle is moving through the wormhole.
	AuPoint movevec = nuloc - oldloc;
	movevec.Unit();
	float entryAngle = movevec.AngleTo(plane.N);
	//if(IsStargate())
	//qDebug("Entered %s stargate at %g degree angle.", m_Polarity==StargateNegative?"Negative":"Positive", entryAngle*RAD2DEG);
	return entryAngle;
}

//static
void AuWormhole::CreateDeviceObjects(AuGLEffects& re, AuGLModelCache& mc)
{
	if(!s_Wormhole)
	{
		// Start loading the 3ds model of the commodity using a worker thread.
		QString modelFile(":data/mWormhole.3dsN");
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		s_Wormhole = dynamic_cast<Au3dsGLSLModelInstance*>(mc.LoadModelThreaded(modelFile, true, 1.));
	}

	if(!g_TexPos)
		g_TexPos = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "w+", ":data/txWormholePos.png");
	Q_ASSERT(g_TexPos);

	if(!g_TexNeg)
		g_TexNeg = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "w-", ":data/txWormholeNeg.png");
	Q_ASSERT(g_TexNeg);

	if(!g_TexInt)
		g_TexInt = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "w=", ":data/txWormholeInt.png");
	Q_ASSERT(g_TexInt);

	if(!g_TexParticles)
		g_TexParticles = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");
	Q_ASSERT(g_TexParticles);

	for(int i=0; i<POLARITY_Count-1; i++)
	{
		if(!s_pParticles[i])
		{
			s_pParticles[i] = new PsWormhole();
			s_pParticles[i]->SetMaxParticles(500);
			s_pParticles[i]->SetNumToRelease(3);
			s_pParticles[i]->SetReleaseInterval(0.00f);
			s_pParticles[i]->SetLifeCycle(3.5f);
			s_pParticles[i]->SetEndReleaseTime(0);
			s_pParticles[i]->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
			s_pParticles[i]->SetTextureID(g_TexParticles);
			s_pParticles[i]->SetVelocityVar(100.5f);
			QColor partCol;
			if(i == 0)
			{
				// Positive
				partCol.setRgbF(0.045f, 0.15f, 0.95f);
				s_pParticles[i]->SetOrigin(true);
				s_pParticles[i]->SetSize(600.0f * 250);
				s_pParticles[i]->SetScale(3600);
				s_pParticles[i]->SetHeight(7500);
			}
			else if(i == 1)
			{
				// Negative
				partCol.setRgbF(0.9f, 0.2f, 0.15f);
				s_pParticles[i]->SetOrigin(false);
				s_pParticles[i]->SetSize(250.0f * 250);
				s_pParticles[i]->SetScale(1800);
				s_pParticles[i]->SetHeight(5000);
			}
			else if(i == 2)
			{
				// Intergalactic
				partCol.setRgb(8, 0, 230);
				s_pParticles[i]->SetOrigin(false);
				s_pParticles[i]->SetSize(2400.0f * 250);
				s_pParticles[i]->SetScale(15000);
				s_pParticles[i]->SetHeight(100000);
			}
			else if(i == 3)
			{
				// StargatePositive
				partCol.setRgb(207, 11, 242);
				s_pParticles[i]->SetOrigin(true);
				s_pParticles[i]->SetSize(1200.0f * 250);
				s_pParticles[i]->SetScale(7200);
				s_pParticles[i]->SetHeight(15000);
			}
			else if(i == 4)
			{
				// StargateNegative
				partCol.setRgb(229, 191, 38);
				s_pParticles[i]->SetOrigin(false);
				s_pParticles[i]->SetSize(500.0f * 250);
				s_pParticles[i]->SetScale(3600);
				s_pParticles[i]->SetHeight(10000);
			}
			else if(i == 5)
			{
				// IntergalacticOneWay
				partCol.setRgb(8, 0, 230);
				s_pParticles[i]->SetOrigin(true);
				s_pParticles[i]->SetSize(2400.0f * 250 * .75);
				s_pParticles[i]->SetScale(15000 * .75);
				s_pParticles[i]->SetHeight(100000 * .75);
			}
			s_pParticles[i]->SetColor(partCol);
			s_pParticles[i]->Init(re);
		}
	}
}

// Location is relative to solar system.
AuPoint AuWormhole::Location(const DaSolarSystem* system) const
{
	switch(m_Polarity)
	{
	case Invalid:
	case POLARITY_Count:
	case Intergalactic:
	case IntergalacticOneWay:
		break;

	case Positive:
	case Negative:
		if(system->HasBlackHole())
		{
			const DaStar* star = system->FindPrimaryStarNotBlackHole();
			if(star)
				return star->EnvLocation() + m_Loc;
			else
			{
				qWarning() << "warning: AuWormhole::Location blackhole system" << system->Id() << system->Name()
					<< "has no primary star";
				return AuPoint(FeetPerSolarSystem(),0,0);
			}
		}
		break;

	case StargatePositive:
		{
			const DaStar* star = system->FindStargateStar(true);
			if(star)
				return star->Transform().Project(m_Loc);
		}
		break;

	case StargateNegative:
		{
			const DaStar* star = system->FindStargateStar(false);
			if(star)
				return star->Transform().Project(m_Loc);
		}
		break;
	}

	return m_Loc;
}

double AuWormhole::ObstacleRadius() const
{
	// Sizes are based on wormhole polarity.
	if(m_Polarity == Positive)
		return 2000.0;					// Distance allowed from center of wormhole.
	else if(m_Polarity == Negative)
		return 1000.0;					// Distance allowed from center of wormhole.
	else if(m_Polarity == Intergalactic)
		return 8000.0;					// Distance allowed from center of wormhole.
	else if(m_Polarity == IntergalacticOneWay)
		return 8000.0 * .75;					// Distance allowed from center of wormhole.
	else if(m_Polarity == StargatePositive)
		return 4000.0;					// Distance allowed from center of wormhole.
	else if(m_Polarity == StargateNegative)
		return 2000.0;					// Distance allowed from center of wormhole.
	else
		return -1;      // Invalid polarity
}

double AuWormhole::PilotAvoidRadius() const
{
	return ObstacleRadius() + OrbitAltAboveAtm();
}

// Renders this object in a scene.
// View transformation has already been set up.
void AuWormhole::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc)
{
	if(!s_Wormhole)
		CreateDeviceObjects(re, mc);
	if(!s_Wormhole->GLModel(re))
		return;

	re.GLSL().ModelTexShader().Select(projection, re);

	// Perform some calculations
	float fCol = 0;
	if(s_pParticles[0])
		g_Rot += s_pParticles[0]->GetElapsedTime();
	double scale, scaleZ;
	AuMatrix4x4 trot;
	if(m_Polarity == Positive)
	{
		fCol = MAX(1-(AuTime::CurrentMSecsSinceEpoch()%360 * DEG2RAD)/6.0, .6);
		trot.RotateZ(g_Rot);
		re.GLSL().ModelTexShader().SetTexture(g_TexPos);
		scale = 500;
		scaleZ = 500;
	}
	else if(m_Polarity == Negative)
	{
		fCol = MAX((AuTime::CurrentMSecsSinceEpoch()%360 * DEG2RAD)/6.0, .6);
		trot.RotateZ(-g_Rot);
		re.GLSL().ModelTexShader().SetTexture(g_TexNeg);
		scale = 250;
		scaleZ = 500;
	}
	else if(m_Polarity == Intergalactic)
	{
		fCol = MAX((AuTime::CurrentMSecsSinceEpoch()%360 * DEG2RAD)/6.0, .6);
		trot.RotateZ(-g_Rot);
		re.GLSL().ModelTexShader().SetTexture(g_TexInt);
		scale = 2000;
		scaleZ = 1000;
	}
	else if(m_Polarity == IntergalacticOneWay)
	{
		fCol = MAX((AuTime::CurrentMSecsSinceEpoch()%360 * DEG2RAD)/6.0, .6);
		trot.RotateZ(-g_Rot);
		re.GLSL().ModelTexShader().SetTexture(g_TexInt);
		scale = 2000 * .75;
		scaleZ = 1000 * .75;
	}
	else if(m_Polarity == StargatePositive)
	{
		fCol = MAX(1-(AuTime::CurrentMSecsSinceEpoch()%360 * DEG2RAD)/6.0, .6);
		trot.RotateZ(g_Rot);
		re.GLSL().ModelTexShader().SetTexture(g_TexPos);
		scale = 1000;
		scaleZ = 1000;
	}
	else if(m_Polarity == StargateNegative)
	{
		fCol = MAX((AuTime::CurrentMSecsSinceEpoch()%360 * DEG2RAD)/6.0, .6);
		trot.RotateZ(-g_Rot);
		re.GLSL().ModelTexShader().SetTexture(g_TexNeg);
		scale = 500;
		scaleZ = 1000;
	}
	else
	{
		// This should stand out.
		scale = 100000;
		scaleZ = 100000;
	}

	re.GLSL().ModelTexShader().SetGlowAlpha(fCol, fCol, fCol, 1.f);

	AuMatrix4x4 tThis = model * Transform();
	AuMatrix4x4 tScaled(tThis);
	tScaled.Scale(scale, scale, scaleZ);

	re.GLSL().ModelTexShader().SetModelView(tScaled);

	// Ambient light
	re.GLSL().ModelTexShader().SetAmbientLight(0, 0, 0);
	re.GLSL().ModelTexShader().SetAmbientLightUp(0, 0, 1);

	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	s_Wormhole->Render(re, false);
	re.glDisable(GL_BLEND);

	re.GLSL().ModelTexShader().Deselect();

	tThis *= trot;
	RenderWormholeParticles(projection, tThis, re, m_Polarity);
}

void AuWormhole::RenderWormholeParticles(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		POLARITY polarity)
{
	int iParticleSystem = polarity-1;
	if(s_pParticles[iParticleSystem])
	{
		if(s_pParticles[iParticleSystem]->Update())
			s_pParticles[iParticleSystem]->Render(projection, model, re);
	}
}

AuMatrix4x4 AuWormhole::Transform() const
{
	AuMatrix4x4 tw;
	tw.Translate(m_Loc);
	tw.Rotate(AuPoint::CalculateRotation(-m_Loc));
	return tw;
}

bool AuWormhole::SetStarDistance(double distance, const AuPoint& sysRot)
{
	if(m_Polarity == StargatePositive)
	{
		if(m_Loc != AuPoint(0, 0, distance))
		{
			m_Loc = AuPoint(0, 0, distance);
			return true;
		}
	}
	else if(m_Polarity == StargateNegative)
	{
		if(m_Loc != AuPoint(0, 0, -distance))
		{
			m_Loc = AuPoint(0, 0, -distance);
			return true;
		}
	}
	else
	{
		// Wormhole is on the opposite side of the sun from the destination star.
		// Wormhole vector is relative to the sector, not to the rotated solar system.
		// Calculate wormhole location relative to solar system.
		AuMatrix4x4 trot;
		trot.UnRotate(sysRot);
		AuPoint nuLoc = trot.Project(-m_VecToDest);
		nuLoc.Unit(distance);
		if(m_Loc != nuLoc)
		{
			m_Loc = nuLoc;
			return true;
		}
	}
	return false;
}

// Returns true if any db values change.
bool AuWormhole::Repair(const AuPoint& vecToDest, double starDistance, const AuPoint& sysRot)
{
	// Track changes.
	bool anyChanges = false;

	// Vec to destination, in parsecs relative to the sector.
	// Wormhole vector is relative to the sector, not to the rotated solar system.
	if(!m_VecToDest.FuzzyEquals(vecToDest))
	{
		qDebug() << "AuWormhole::Repair m_VecToDest" << m_VecToDest << "vecToDest" << vecToDest;
		m_VecToDest = vecToDest;
		anyChanges = true;
	}

	// Wormhole is on the opposite side of the sun from the destination star.
	if(starDistance > 0)
		if(SetStarDistance(starDistance, sysRot))
			anyChanges = true;

	return anyChanges;
}

quint64 AuWormhole::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(quint32) * m_ExploredBy.capacity();
	mem += StringMemoryUsage(m_Name);
	mem += sizeof(m_Polarity);
	mem += sizeof(m_ToSystem);
	mem += sizeof(m_VecToDest);
	mem += sizeof(m_Loc);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuWormholeArray::operator<<(QDataStream& in)
{
	clear();
	quint32 count;
	in >> count;
	for(;count;count--)
	{
		AuWormhole wh;
		wh << in;
		append(wh);
	}
}

void AuWormholeArray::operator>>(QDataStream& out) const
{
	out << (quint32)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		*it >> out;
}

void AuWormholeArray::Rx(QDataStream& in)
{
	clear();
	quint8 count;
	in >> count;
	for(;count;count--)
	{
		AuWormhole wh;
		wh.Rx(in);
		append(wh);
	}
}

void AuWormholeArray::Tx(QDataStream& out) const
{
	out << (quint8)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it).Tx(out);
}

bool AuWormholeArray::HasWormhole(const AuId& toSystem, AuWormhole::POLARITY polarity) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).Polarity() == polarity && (*it).ToSystem() == toSystem)
			return true;
	return false;
}

quint64 AuWormholeArray::MemoryUsage() const
{
	quint64 mem = sizeof(AuWormholeArray);
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		mem += (*it).MemoryUsage();
	return mem;
}
