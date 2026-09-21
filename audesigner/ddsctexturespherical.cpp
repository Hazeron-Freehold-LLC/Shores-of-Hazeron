//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctexturespherical.cpp,v 1.6 2022/02/19 18:27:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsctexturespherical.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscdesign.h"
#include "ausceffectsphericaldlg.h"
#include "ausctexturesphericaldlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScTextureSpherical::DdScTextureSpherical(int layer, AuDesignerView* view) : DdCommand(view)
{
	m_Layer = layer;
}

QString DdScTextureSpherical::WikiTopic() const
{
	return "Designer#Spherical_Projection";
}

void DdScTextureSpherical::Start()
{
	DdCommand::Start();

	if(m_Layer == 1)
	{
		AuScEffectSphericalDlg dlg(m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			SURFACE_EFFECT effect = dlg.Effect();
			quint16 texReps = dlg.TextureReps();
			auApp->DesignerPartsEffectFacesSpherical(m_pWnd->DesignerPartObjectSelection(), effect, texReps, m_pWnd->DesignPlane());
		}
	}
	else
	{
		COMMODITYID hullModule = CID_Invalid;
		AuDesignTex palette[SCTEXI_Count];
		{
			DaPerson* person;
			m_pWnd->LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* pde = person->FindParentDesignerInEnv();
				if(pde)
				{
					pde->TexturePalette(palette);
					const AuDesign* design = pde->LockDesignForRead();
					if(design)
						hullModule = design->HullModule();
					pde->UnlockDesign();

				}
			}
			m_pWnd->UnlockSceneAtom();
		}

		AuScTextureSphericalDlg dlg(hullModule, palette, m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			SC_TEXTURE_INDEX texIdx = dlg.TextureIndex();
			quint16 texReps = dlg.TextureReps();
			auApp->DesignerPartsTextureFacesSpherical(m_pWnd->DesignerPartObjectSelection(), texIdx, texReps,
				m_pWnd->DesignPlane());
		}
	}

	// We only take one shot at this.
	Terminate();
}
