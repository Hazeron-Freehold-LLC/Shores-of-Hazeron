//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsoleempty.h,v 1.43 2022/04/14 16:41:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpConsoleEmpty_H
#define DpConsoleEmpty_H

#include "auatom.h"
#include "dpconsole.h"
#include "dpstationempty.h"

class AUATOM_DLL DpConsoleEmpty : public DpStationEmpty, public DpConsole
{
public:
	DpConsoleEmpty();
	virtual ~DpConsoleEmpty();
	DpConsoleEmpty(const DpConsoleEmpty& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const AuPoint& location,
			const DaCreature& sceneAtom);

	virtual DpStation* Clone() const;

	virtual void CreateDeviceObjectsEmpty(AuAtomRenderInfo& ari, const DaAtom& parent, const QString& screenName,
			const char* screenFile, bool inBuilding);

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaCreature& seatedCreature) const;

	virtual quint64 MemoryUsage() const;

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
			AuAtomRenderInfo& ari);

	virtual AuStation::TYPE Type() const
	{
		// When the return type is CommandConsole, the screen is not drawn in DpConsole::Render, only the chair.
		// A screen texture is only set when this console is unoccupied, representing an unmanned console in
		// the ship (other than a command console).
		// Returning none as the type causes unoccupied consoles to draw their screens.
		if(m_ScreenFile)
			return AuStation::TYPE_Count;
		else
			return AuStation::Console_Command;
	}

protected:
	virtual void PanelDimensions(double& width, double& height) const;
};

#endif // DpConsoleHelm_H
