//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscexportstl.cpp,v 1.6 2022/02/19 18:26:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscexportstl.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscexportstldlg.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScExportSTL::DdScExportSTL(AuDesignerView* view) : DdCommand(view)
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

QString DdScExportSTL::WikiTopic() const
{
	return "Designer_Menu#Export_STL";
}

void DdScExportSTL::Start()
{
	DdCommand::Start();

	AuScExportSTLDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		QString saveFile = dlg.FileName();
		if(!saveFile.isEmpty())
		{
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

			// Collect all visible faces into a single face list.
			AuScFaceList faces;
			m_pWnd->DesignerVisibleFaces(faces);
			if(!faces.empty())
			{
				double scale = dlg.ScaleToFeet();
				/*
				faces.Transform(m_pWnd->DesignPlane().Inverse());	// Transform the result to the design plane.
				if(scale != 1)
				{
					// Apply unit scaling.
					AuMatrix4x4 scaleM;
					scaleM.Scale(1./scale, 1./scale, 1./scale);
					faces.Transform(scaleM);
				}
				*/
				AuMatrix4x4 exportM;
				if(scale != 1)
					exportM.Scale(1/scale,1/scale,1/scale);		// Apply unit of measure scaling.
				exportM *= m_pWnd->DesignPlane().Inverse();		// Transform the result to the design plane.
				faces.Transform(exportM);

				// Export the faces to an STL file.
				faces.STLWriteBinary(saveFile);
			}

			QApplication::restoreOverrideCursor();
		}
	}

	// We only take one shot at this.
	Terminate();
}
