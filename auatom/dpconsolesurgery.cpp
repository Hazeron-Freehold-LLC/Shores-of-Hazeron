//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsolesurgery.cpp,v 1.99 2022/04/14 16:43:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpconsolesurgery.h"
//
#include "dacreature.h"

DpConsoleSurgery::DpConsoleSurgery(quint16 whichSurgery) : DpStationSurgery(whichSurgery)
{
}

DpConsoleSurgery::~DpConsoleSurgery()
{
}

DpConsoleSurgery::DpConsoleSurgery(const DpConsoleSurgery& c) : DpStationSurgery(c), DpConsole(c)
{
}

DpStation* DpConsoleSurgery::Clone() const
{
	return new DpConsoleSurgery(*this);
}

void DpConsoleSurgery::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStationSurgery::CreateDeviceObjects(ari, parent, inBuilding);
	DpConsole::CreateDeviceObjects(ari, parent, inBuilding);
}

// Called between rendering scene frames that contains this object.
void DpConsoleSurgery::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	DpStationSurgery::AnimateDeviceObjects(re, ship, last, location, sceneAtom);	// Primary display screen.
	DpConsole::AnimateDeviceObjects(re, ship, last, sceneAtom);					// Secondary display screen.
}

void DpConsoleSurgery::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	DpStationSurgery::Render(projection, farProjection, tPanel, sceneAtom, seatedCreature, ari);
	DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
}

void DpConsoleSurgery::RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
		AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
}

void DpConsoleSurgery::PanelDimensions(double& width, double& height) const
{
	DpConsole::PanelDimensions(0, width, height);
}

void DpConsoleSurgery::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaCreature& seatedCreature) const
{
	AuMatrix4x4 tPanel(tModel);
	tPanel.Translate(0,0,-seatedCreature.GroundHeightAboveCG()/seatedCreature.GrowthScale());
	DpConsole::EnumControlPanels(aControlPanels, tPanel, idPanels);
}
