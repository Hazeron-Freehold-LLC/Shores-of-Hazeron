//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsoleengineer.cpp,v 1.100 2022/04/14 16:43:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpconsoleengineer.h"
//
#include "dacreature.h"

DpConsoleEngineer::DpConsoleEngineer(quint16 whichFTL) : DpStationEngineer(whichFTL)
{
}

DpConsoleEngineer::~DpConsoleEngineer()
{
}

DpConsoleEngineer::DpConsoleEngineer(const DpConsoleEngineer& c) : DpStationEngineer(c), DpConsole(c)
{
}

DpStation* DpConsoleEngineer::Clone() const
{
	return new DpConsoleEngineer(*this);
}

void DpConsoleEngineer::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStationEngineer::CreateDeviceObjects(ari, parent, inBuilding);
	DpConsole::CreateDeviceObjects(ari, parent, inBuilding);
}

// Called between rendering scene frames that contains this object.
void DpConsoleEngineer::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	DpStationEngineer::AnimateDeviceObjects(re, ship, last, location, sceneAtom);	// Primary display screen.
	DpConsole::AnimateDeviceObjects(re, ship, last, sceneAtom);					// Secondary display screen.
}

void DpConsoleEngineer::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	DpStationEngineer::Render(projection, farProjection, tPanel, sceneAtom, seatedCreature, ari);
	DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
}

void DpConsoleEngineer::RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
		AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
}

void DpConsoleEngineer::PanelDimensions(double& width, double& height) const
{
	DpConsole::PanelDimensions(0, width, height);
}

void DpConsoleEngineer::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaCreature& seatedCreature) const
{
	AuMatrix4x4 tPanel(tModel);
	tPanel.Translate(0,0,-seatedCreature.GroundHeightAboveCG()/seatedCreature.GrowthScale());
	DpConsole::EnumControlPanels(aControlPanels, tPanel, idPanels);
}
