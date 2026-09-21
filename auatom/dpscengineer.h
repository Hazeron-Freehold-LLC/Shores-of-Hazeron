//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpscengineer.h,v 1.5 2022/02/19 17:37:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpScEngineer_H
#define DpScEngineer_H

#include "auatom.h"
#include "auquad.h"
#include "dpstationengineer.h"

class AuScStation;

class AUATOM_DLL DpScEngineer : public DpStationEngineer
{
	AuQuad								m_Screen;

	// OpenGL
	AuGLVAO								m_ScreenVAO;

public:
	DpScEngineer(quint16 whichFTL);
	virtual ~DpScEngineer();
	DpScEngineer(const DpScEngineer& c);

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
		return AuStation::Integrated_Engine;
	}

protected:
	virtual void PanelDimensions(double& width, double& height) const;
};

#endif // DpScEngineer_H
