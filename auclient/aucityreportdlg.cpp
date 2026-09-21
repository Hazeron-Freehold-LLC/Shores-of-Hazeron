//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucityreportdlg.cpp,v 1.6 2022/05/05 19:33:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucityreportdlg.h"
//
#include "aucityreport.h"
#include "auapp.h"
#include "auglobal.h"
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>

AuCityReportDlg::AuCityReportDlg(const AuId& cityId, const AuCityReport& report, QWidget* parent) : AuDialog(parent)
{
	m_CityId = cityId;
	setAuDlgTitle(tr("%1 at %2")
			.arg(report.m_Name)
			.arg(report.m_WhenCompiled.UTC().toLocalTime().toString(Qt::SystemLocaleShortDate)));
	switch(report.m_MessageType)
	{
	case AuMailMsg::MSG_CityStatusReport:
	default:
		setAuDlgIcon(QIcon(":data/MailCityStatus.png"));
		break;
	case AuMailMsg::MSG_CityStatusReportInfo:
		setAuDlgIcon(QIcon(":data/MailCityStatusInfo.png"));
		break;
	case AuMailMsg::MSG_CityDistressReport:
		setAuDlgIcon(QIcon(":data/MailCityDistress.png"));
		break;
	case AuMailMsg::MSG_CityFinalDecayReport:
		setAuDlgIcon(QIcon(":data/MailCityDecayed.png"));
		break;
	}

	m_NextReport = new QToolButton(this);
	m_NextReport->setIcon(QIcon(":data/MailCityStatus.png"));
	m_NextReport->setToolTip(tr("<html>Requests the most recent city status report. "
				"</html>"));
	connect(m_NextReport, SIGNAL(clicked()), SLOT(ReportCityLast()));

	QTextBrowser* browser = new QTextBrowser;
	browser->setReadOnly(true);
	browser->setHtml(report.ReportHtml(report, false));

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addStretch();
	hlay->addWidget(m_NextReport);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(hlay);
	lay->addWidget(browser);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCityReportDlg::~AuCityReportDlg()
{
}

void AuCityReportDlg::setVisible(bool visible)
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
		deleteLater();
	}
}

void AuCityReportDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuCityReportDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuCityReportDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuCityReportDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuCityReportDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCityReportDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuCityReportDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuCityReportDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuCityReportDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuCityReportDlg/SizeY",sz.height());
}

void AuCityReportDlg::ReportCityLast()
{
	if(m_CityId.IsNull())
		return;
	auApp->ReportCityLast(m_CityId);
	accept();
}

QSize AuCityReportDlg::sizeHint() const
{
	return QSize(800,600);
}

