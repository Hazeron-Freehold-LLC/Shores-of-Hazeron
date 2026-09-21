//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctexturecylindrical.cpp,v 1.6 2022/02/19 18:27:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsctexturecylindrical.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscdesign.h"
#include "ausceffectcylindricaldlg.h"
#include "ausctexturecylindricaldlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScTextureCylindrical::DdScTextureCylindrical(int layer, AuDesignerView* view) : DdCommand(view)
{
	m_Layer = layer;
}

QString DdScTextureCylindrical::WikiTopic() const
{
	return "Designer#Cylindrical_Projection";
}

void DdScTextureCylindrical::Start()
{
	DdCommand::Start();

	if(m_Layer == 1)
	{
		// Effects layer.
		AuScEffectCylindricalDlg dlg(m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			SURFACE_EFFECT effect = dlg.Effect();
			quint16 texReps = dlg.TextureReps();
			double texSize = dlg.TextureSize();
			auApp->DesignerPartsEffectFacesCylindrical(m_pWnd->DesignerPartObjectSelection(), effect, texReps, texSize,
					m_pWnd->DesignPlane());
		}
	}
	else
	{
		// Base layer.
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

		AuScTextureCylindricalDlg dlg(hullModule, palette, m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			SC_TEXTURE_INDEX texIdx = dlg.TextureIndex();
			quint16 texReps = dlg.TextureReps();
			double texSize = dlg.TextureSize();
			auApp->DesignerPartsTextureFacesCylindrical(m_pWnd->DesignerPartObjectSelection(), texIdx,
					texReps, texSize, m_pWnd->DesignPlane());
		}
	}

	// We only take one shot at this.
	Terminate();
}
