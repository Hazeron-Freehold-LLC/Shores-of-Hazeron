//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctextureprojected.cpp,v 1.6 2022/02/19 18:27:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsctextureprojected.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscdesign.h"
#include "ausceffectprojectdlg.h"
#include "ausctextureprojectdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScTextureProjected::DdScTextureProjected(int layer, AuDesignerView* view) : DdCommand(view)
{
	m_Layer = layer;
}

QString DdScTextureProjected::WikiTopic() const
{
	return "Designer#Plane_Projection";
}

void DdScTextureProjected::Start()
{
	DdCommand::Start();

	if(m_Layer == 1)
	{
		// Effects layer.
		AuScEffectProjectDlg dlg(m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			SURFACE_EFFECT effect = dlg.Effect();
			double texSize = dlg.TextureSize();
			auApp->DesignerPartsEffectFacesProject(m_pWnd->DesignerPartObjectSelection(), effect, texSize, m_pWnd->DesignPlane());
		}
	}
	else
	{
		// Base layer.
		QImage custom;
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

		AuScTextureProjectDlg dlg(hullModule, palette, m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			SC_TEXTURE_INDEX texIdx = dlg.TextureIndex();
			double texSize = dlg.TextureSize();
			auApp->DesignerPartsTextureFacesProject(m_pWnd->DesignerPartObjectSelection(), texIdx, texSize,
					m_pWnd->DesignPlane());
		}
	}

	// We only take one shot at this.
	Terminate();
}
