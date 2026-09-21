//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscpaint.cpp,v 1.6 2022/02/19 18:27:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscpaint.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscpaintdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScPaint::DdScPaint(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScPaint::WikiTopic() const
{
	return "Designer_Menu#Paint";
}

void DdScPaint::Start()
{
	DdCommand::Start();

	QList<QColor> paintInUse;
	m_pWnd->DesignerPaintInUse(paintInUse);

	/*
	// Sort the colors
	QMultiMap<QString, QColor> sorted;
	for(QList<QColor>::const_iterator pi = paintInUse.cbegin(); pi != paintInUse.cend(); pi++)
	{
		int v = (*pi).value();
		QString key = QString("%1%2")
			.arg(v, 3,10,QLatin1Char('0'))
			.arg((*pi).name());
		//qDebug() << key;
		sorted.insert(key, *pi);
	}
	paintInUse.clear();
	for(QMultiMap<QString, QColor>::const_iterator si = sorted.cend()-1; si != sorted.cbegin()-1; si--)
		paintInUse.append(*si);
		*/

	AuScPaintDlg dlg(paintInUse, m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		if(dlg.UseTheme())
		{
			quint8 theme = dlg.Theme();
			quint8 opacity = dlg.Opacity();
			auApp->DesignerPaintFaces(m_pWnd->DesignerPartObjectSelection(), theme, opacity);
		}
		else
		{
			QColor paint = dlg.Paint();
			auApp->DesignerPaintFaces(m_pWnd->DesignerPartObjectSelection(), paint);
		}
	}

	// We only take one shot at this.
	Terminate();
}
