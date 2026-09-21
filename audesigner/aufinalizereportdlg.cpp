//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufinalizereportdlg.cpp,v 1.4 2022/02/19 18:25:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aufinalizereportdlg.h"
//
#include "auapp.h"
#include "audesign.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextBrowser>

AuFinalizeReportDlg::AuFinalizeReportDlg(const QString& report, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Finalize Report"));
	setAuDlgIcon(QIcon(":data/ScFinish.png"));

	m_Verification = new QTextBrowser(this);
	m_Verification->setReadOnly(true);
	m_Verification->setHtml(report);
	m_Verification->setToolTip(tr("<html>"
				"Result of previous finalize process."
				"</html>"));

	QHBoxLayout* lay = new QHBoxLayout;
	lay->setMargin(4);
	lay->setSpacing(2);
	lay->addWidget(m_Verification);
	setAuDlgLayout(lay);
}

AuFinalizeReportDlg::~AuFinalizeReportDlg()
{
}

void AuFinalizeReportDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuFinalizeReportDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/FinalizeReport/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/FinalizeReport/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/FinalizeReport/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/FinalizeReport/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuFinalizeReportDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/FinalizeReport/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/FinalizeReport/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/FinalizeReport/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/FinalizeReport/SizeY",sz.height());
}
