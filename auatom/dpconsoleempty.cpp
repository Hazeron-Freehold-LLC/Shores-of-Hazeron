//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsoleempty.cpp,v 1.59 2022/04/14 16:42:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpconsoleempty.h"
//
#include "auatomrenderinfo.h"
#include "dacreature.h"

DpConsoleEmpty::DpConsoleEmpty()
{
}

DpConsoleEmpty::~DpConsoleEmpty()
{
}

DpConsoleEmpty::DpConsoleEmpty(const DpConsoleEmpty& c) : DpStationEmpty(c), DpConsole(c)
{
}

DpStation* DpConsoleEmpty::Clone() const
{
	return new DpConsoleEmpty(*this);
}

void DpConsoleEmpty::CreateDeviceObjectsEmpty(AuAtomRenderInfo& ari, const DaAtom& parent, const QString& screenName,
		const char* screenFile, bool inBuilding)
{
	DpStationEmpty::CreateDeviceObjectsEmpty(ari, parent, screenName, screenFile, inBuilding);
	DpConsole::CreateDeviceObjects(ari, parent, inBuilding);
}

// Called between rendering scene frames that contains this object.
void DpConsoleEmpty::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	DpStationEmpty::AnimateDeviceObjects(re, ship, last, location, sceneAtom);	// Primary display screen.
	DpConsole::AnimateDeviceObjects(re, ship, last, sceneAtom);					// Secondary display screen.
}

void DpConsoleEmpty::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	DpStationEmpty::Render(projection, farProjection, tPanel, sceneAtom, seatedCreature, ari);
	if(m_ScreenTex)
		DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
	else
		DpConsole::RenderChair(projection, tPanel, ari);
}

void DpConsoleEmpty::RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
		AuAtomRenderInfo& ari)
{
	AuMatrix4x4 tPanel(tModel);
	if(seatedCreature)
		tPanel.Translate(0,0,-seatedCreature->GroundHeightAboveCG()/seatedCreature->GrowthScale());
	if(m_ScreenTex)
		DpConsole::RenderConsole(projection, tPanel, ari, m_ScreenTex);
	else
		DpConsole::RenderChair(projection, tPanel, ari);
}

void DpConsoleEmpty::PanelDimensions(double& width, double& height) const
{
	DpConsole::PanelDimensions(0, width, height);
}

void DpConsoleEmpty::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaCreature& seatedCreature) const
{
	AuMatrix4x4 tPanel(tModel);
	tPanel.Translate(0,0,-seatedCreature.GroundHeightAboveCG()/seatedCreature.GrowthScale());
	DpConsole::EnumControlPanels(aControlPanels, tPanel, idPanels);
}

quint64 DpConsoleEmpty::MemoryUsage() const
{
	quint64 mem = 0;
	mem += DpStationEmpty::MemoryUsage();
	mem += DpConsole::MemoryUsage();
	return mem;
}
