//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscimport3ds.cpp,v 1.6 2022/02/19 18:26:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscimport3ds.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscimport3dsdlg.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScImport3ds::DdScImport3ds(AuDesignerView* view) : DdCommand(view)
{
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TexturePalette(m_Palette);
	}
	view->UnlockSceneAtom();
}

QString DdScImport3ds::WikiTopic() const
{
	return "Designer_Menu#3DS";
}

void DdScImport3ds::Start()
{
	DdCommand::Start();

	AuScImport3dsDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		QString loadFile = dlg.FileName();
		if(!loadFile.isEmpty())
		{
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

			double scale = dlg.ScaleToFeet();

			// Import data from the 3ds file.
			AuScParts parts;
			parts.Import3DS(loadFile, scale);
			parts.Transform(m_pWnd->DesignPlane());
			if(!parts.empty())
			{
				// We got parts.
				auApp->DesignerAddParts(parts);

				// Do they want a texture too?
				if(dlg.TextureToo())
				{
					QString texFileName = dlg.TextureFileName();
					QImage imTex;
					if(imTex.load(texFileName))
					{
						QString name = QFileInfo(texFileName).fileName();
						// Texture file loaded successfully.
						if(imTex.width() > AuDesignTex::MaxImageDimension() || imTex.height() > AuDesignTex::MaxImageDimension())
							imTex = imTex.scaled(AuDesignTex::MaxImageDimension(),AuDesignTex::MaxImageDimension(),
									Qt::KeepAspectRatio, Qt::SmoothTransformation);
						else if(imTex.width() < 1 || imTex.height() < 1)
							imTex = QImage();
						m_Palette[SCTEXI_User0].SetImage(name, imTex);
						auApp->DesignerSetTexturePalette(m_Palette);
					}
				}
			}

			QApplication::restoreOverrideCursor();
		}
	}

	// We only take one shot at this.
	Terminate();
}
