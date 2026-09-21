//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctexturepalette.cpp,v 1.6 2022/02/19 18:27:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsctexturepalette.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscdesign.h"
#include "auscpalettedlg.h"
#include "ausctexturechoose.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScTexturePalette::DdScTexturePalette(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScTexturePalette::WikiTopic() const
{
	return "Designer#Palette";
}

void DdScTexturePalette::Start()
{
	DdCommand::Start();

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

	AuScPaletteDlg dlg(hullModule, palette, m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		if(dlg.TexturePaletteChanged())
			auApp->DesignerSetTexturePalette(dlg.TexturePalette());

	// We only take one shot at this.
	Terminate();
}
