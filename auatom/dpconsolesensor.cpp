//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsolesensor.cpp,v 1.148 2022/04/14 16:43:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpconsolesensor.h"
//
#include "dacreature.h"

DpConsoleSensor::DpConsoleSensor()
{
}

DpConsoleSensor::~DpConsoleSensor()
{
}

DpConsoleSensor::DpConsoleSensor(const DpConsoleSensor& c) : DpStationSensor(c), DpConsole(c)
{
}

DpStation* DpConsoleSensor::Clone() const
{
	return new DpConsoleSensor(*this);
}

void DpConsoleSensor::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStationSensor::CreateDeviceObjects(ari, parent, inBuilding);
	DpConsole::CreateDeviceObjects(ari, parent, inBuilding);
}

// Called between rendering scene frames that contains this object.
void DpConsoleSensor::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	DpStationSensor::AnimateDeviceObjects(re, ship, last, location, sceneAtom);	// Primary display screen.
	DpConsole::AnimateDeviceObjects(re, ship, last, sceneAtom);					// Secondary display screen.
}

void DpConsoleSensor::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	DpStationSensor::Render(projection, farProjection, tPanel, sceneAtom, seatedCreature, ari);
	DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
}

void DpConsoleSensor::RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
		AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
}

void DpConsoleSensor::PanelDimensions(double& width, double& height) const
{
	DpConsole::PanelDimensions(0, width, height);
}

void DpConsoleSensor::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaCreature& seatedCreature) const
{
	AuMatrix4x4 tPanel(tModel);
	tPanel.Translate(0,0,-seatedCreature.GroundHeightAboveCG()/seatedCreature.GrowthScale());
	DpConsole::EnumControlPanels(aControlPanels, tPanel, idPanels);
}
