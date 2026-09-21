//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpscshield.h,v 1.5 2022/02/19 17:37:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpScShield_H
#define DpScShield_H

#include "auatom.h"
#include "auglsl.h"
#include "auquad.h"
#include "dpstationshield.h"

class AuScStation;

class AUATOM_DLL DpScShield : public DpStationShield
{
	AuQuad								m_Screen;

	// OpenGL
	AuGLVAO								m_ScreenVAO;

public:
	DpScShield();
	virtual ~DpScShield();
	DpScShield(const DpScShield& c);

public:
	virtual DpStation* Clone() const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaCreature& seatedCreature) const;

	virtual quint64 MemoryUsage() const;

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this station, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	static void RenderEmpty(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, AuAtomRenderInfo& ari,
		const DaAtom& parent, bool inBuilding, const AuQuad& screen);

	virtual AuStation::TYPE Type() const
	{
		return AuStation::Integrated_Shield;
	}

protected:
	virtual void PanelDimensions(double& width, double& height) const;
};

#endif // DpScShield_H
