//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsoleweapon.h,v 1.72 2022/04/14 16:41:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpConsoleWeapon_H
#define DpConsoleWeapon_H

#include "auatom.h"
#include "dpconsole.h"
#include "dpstationweapon.h"

class AUATOM_DLL DpConsoleWeapon : public DpStationWeapon, public DpConsole
{
public:
	DpConsoleWeapon(quint16 whichWeapon);
	virtual ~DpConsoleWeapon();
	DpConsoleWeapon(const DpConsoleWeapon& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const AuPoint& location,
			const DaCreature& sceneAtom);

	virtual DpStation* Clone() const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaCreature& seatedCreature) const;

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
			AuAtomRenderInfo& ari);

	virtual AuStation::TYPE Type() const
	{
		return AuStation::Console_FireControl;
	}

protected:
	virtual void PanelDimensions(double& width, double& height) const;
};

#endif // DpConsoleWeapon_H
