//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dadesignbase.cpp,v 1.14 2022/05/17 15:57:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dadesignbase.h"
//

DaDesignBase::DaDesignBase(quint16 studioId, DESIGNMODE mode)
{
	m_DesignMode = mode;
	m_StudioId = studioId;
}

DaDesignBase::DaDesignBase()
{
	m_DesignMode = DESIGN_Unknown;
	m_StudioId = 0;
}

DaDesignBase::~DaDesignBase()
{
}

DaDesignBase::DaDesignBase(const DaDesignBase &rhs) : DaAtom(rhs)
{
	m_DesignMode = rhs.m_DesignMode;
	m_StudioId = rhs.m_StudioId;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaDesignBase::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, 
		const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	toClient << m_DesignMode;
	toClient << m_StudioId;
}

// Receive updates to this atom from a server.
void DaDesignBase::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);

	fromServer >> m_DesignMode;
	fromServer >> m_StudioId;
}

// Refresh data from valid counterpart.
bool DaDesignBase::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaDesignBase* pA = (const DaDesignBase*)pAtom;
	m_DesignMode = pA->m_DesignMode;
	m_StudioId = pA->m_StudioId;

	return false;
}

// Setup light sources for lighting calculations to render a given object.
void DaDesignBase::SetupLights(DaPerson *pSceneAtom, const AuMatrix4x4 &tModel, AuGLEffects& re)
{
	QList<AuGLSun> suns;
	QList<AuGLLight> lights;
	GetRenderLightsRecursive(*pSceneAtom, tModel, suns, lights);
	SetRenderLights(re, suns, lights, AuGLLight::MaxAmbientStarlight());	// Bright starlight gives good ambient lighting.
}

void DaDesignBase::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(!FindStar())
	{
		// Set up some sun lights.
		const double ltdist = 10000.0;		// Light distance from model.
		double atten = 1.0/(ltdist*ltdist);
		AuPoint vl(0,0,ltdist);
		vl.Unit(ltdist);
		vl = model.Project(vl);
		suns.append(AuGLSun(atten, QColor(220,220,220), vl, 1.0));

		vl = AuPoint(-669,-743,-1);
		vl.Unit(ltdist);
		vl = model.Project(vl);
		suns.append(AuGLSun(atten, QColor(192,192,192), vl, 1.0));

		vl = AuPoint(697,627,-348);
		vl.Unit(ltdist);
		vl = model.Project(vl);
		suns.append(AuGLSun(atten, QColor(160,160,160), vl, 1.0));
	}

	DaAtom::GetRenderLightsRecursive(person, model, suns, lights);
}

quint64 DaDesignBase::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaDesignBase) - sizeof(DaAtom);
	return mem;
}

DaBd* DaDesignBase::FindParentBdInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaBd* DaDesignBase::FindParentBdInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaCapsule* DaDesignBase::FindParentCapsuleInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaCapsule* DaDesignBase::FindParentCapsuleInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaDesignBase* DaDesignBase::FindParentDesignBaseInEnv()
{
	return this;
}

const DaDesignBase* DaDesignBase::FindParentDesignBaseInEnv() const
{
	return this;
}

DaDesigner* DaDesignBase::FindParentDesignerInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaDesigner* DaDesignBase::FindParentDesignerInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaWorld* DaDesignBase::FindParentPlanetInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaWorld* DaDesignBase::FindParentPlanetInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaSolarSystem* DaDesignBase::FindParentSolarSystemInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaSolarSystem* DaDesignBase::FindParentSolarSystemInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaStar* DaDesignBase::FindParentStarInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaStar* DaDesignBase::FindParentStarInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaVessel* DaDesignBase::FindParentVesselInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaVessel* DaDesignBase::FindParentVesselInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaWorldLL* DaDesignBase::FindParentWorldInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaWorldLL* DaDesignBase::FindParentWorldInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

bool DaDesignBase::IsTicker() const
{
	return false;
}

void DaDesignBase::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << m_DesignMode;
	out << m_StudioId;
}

void DaDesignBase::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	in >> m_DesignMode;
	in >> m_StudioId;
}

