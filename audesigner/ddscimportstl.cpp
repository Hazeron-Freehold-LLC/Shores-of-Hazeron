//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscimportstl.cpp,v 1.6 2022/02/19 18:26:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscimportstl.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscimportstldlg.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScImportSTL::DdScImportSTL(AuDesignerView* view) : DdCommand(view)
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

QString DdScImportSTL::WikiTopic() const
{
	return "Designer_Menu#STL";
}

void DdScImportSTL::Start()
{
	DdCommand::Start();

	AuScImportSTLDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		QString loadFile = dlg.FileName();
		if(!loadFile.isEmpty())
		{
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

			AuScFaceList faces;
			int result = faces.STLReadASCII(loadFile);
			if(result == 2)
				result = faces.STLReadBinary(loadFile);	// ASCII load failed so try binary instead. User probably doesn't know.
			switch(result)
			{
			case 0:	// success
				break;
			case 1:	// File open failed.
				QMessageBox::critical(m_pWnd, g_Au.ApplicationName(),
						QObject::tr("<html>"
							"<p>"
							"The file could not be opened. "
							"</p>"
							"</html>"),
						QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				break;
			case 2:	// File read failed.
				QMessageBox::critical(m_pWnd, g_Au.ApplicationName(),
						QObject::tr("<html>"
							"<p>"
							"The file could not be read. "
							"The contents of the file were not recognized as ASCII or binary STL data. "
							"</p>"
							"</html>"),
						QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				break;
			default:
				break;
			}
			AuScMesh mesh;
			mesh.AddFaces(faces);
			if(!mesh.IsValid())
				return;

			double scale = dlg.ScaleToFeet();
			/*
			if(scale != 1)
			{
				// Apply unit scaling.
				AuMatrix4x4 scaleM;
				scaleM.Scale(scale, scale, scale);		// Apply unit of measure scaling.
				mesh.Transform(scaleM);
			}
			mesh.Transform(m_pWnd->DesignPlane());		// Transform the result to the design plane.
			*/
			AuMatrix4x4 importM(m_pWnd->DesignPlane());
			if(scale != 1)
				importM.Scale(scale,scale,scale);	// Apply unit of measure scaling.
			mesh.Transform(importM);				// Transform the result to the design plane.	

			auApp->DesignerPartAdd(mesh);

			QApplication::restoreOverrideCursor();
		}
	}

	// We only take one shot at this.
	Terminate();
}
