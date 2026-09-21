//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucityhistorydlg.cpp,v 1.4 2022/02/19 18:10:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucityhistorydlg.h"
//
#include "auapp.h"
#include "auglobal.h"
#include <QScrollBar>
#include <QSlider>
#include <QTextBrowser>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

AuCityHistoryDlg::AuCityHistoryDlg(const AuId& cityId, const AuCityReportHistory& report, QWidget* parent) : AuDialog(parent)
{
	m_CityId = cityId;
	m_ReportHistory = report;

	setAuDlgTitle(tr("City Report History"));
	setAuDlgIcon(QIcon(":data/MailCityStatus.png"));

	m_NextReport = new QToolButton(this);
	m_NextReport->setIcon(QIcon(":data/MailCityStatus.png"));
	m_NextReport->setToolTip(tr("<html>Requests the most recent city status report. "
				"</html>"));
	connect(m_NextReport, SIGNAL(clicked()), SLOT(ReportCityLast()));

	QSlider* timeline = new QSlider(Qt::Horizontal);
	timeline->setRange(0, m_ReportHistory.Count()-1);
	timeline->setSingleStep(1);
	timeline->setPageStep(1);
	timeline->setValue(m_ReportHistory.Count()-1);
	timeline->setTickPosition(QSlider::TicksBelow);
	timeline->setTickInterval(1);
	timeline->setToolTip(tr("<html>Selects which <i>cycle report</i> to display.<p>"
				"Cities cycle every %1 minutes; that is %2 cycles per day. "
				"This is when citizens consume food and air, when population and morale change, when taxes are levied, "
				"and when other periodic events happen.<p>"
				"Why this odd interval? "
				"There are 28 <i>game days</i> in each <i>real day</i>. "
				"Citizens eat 4 times per game day. "
				"A city cycle occurs at each meal.<p>"
				"Cities store a report at each cycle. "
				"Inventory details are discarded after 2 hours. "
				"Reports older than 48 hours are discarded completely."
				"</html>").arg(d_CityReportInterval/60).arg((24*60*60)/d_CityReportInterval));
	connect(timeline, SIGNAL(valueChanged(int)), SLOT(SelectReport(int)));

	m_ReportWidget = new QTextBrowser;
	m_ReportWidget->setReadOnly(true);

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addWidget(timeline);
	hlay->addWidget(m_NextReport);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(hlay);
	lay->addWidget(m_ReportWidget);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	SelectReport(m_ReportHistory.Count()-1);
}

AuCityHistoryDlg::~AuCityHistoryDlg()
{
}

void AuCityHistoryDlg::setVisible(bool visible)
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

void AuCityHistoryDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuCityHistoryDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuCityHistoryDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuCityHistoryDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuCityHistoryDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCityHistoryDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuCityHistoryDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuCityHistoryDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuCityHistoryDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuCityHistoryDlg/SizeY",sz.height());
}

void AuCityHistoryDlg::SelectReport(int reportIndex)
{
	if(reportIndex < 0 || reportIndex > m_ReportHistory.Count()-1)
		return;

	const AuCityReport& report = m_ReportHistory.At(reportIndex);
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

	int vscrollPos = 0;
	if(m_ReportWidget->verticalScrollBar())
		vscrollPos = m_ReportWidget->verticalScrollBar()->value();

	if(reportIndex > 0)
		m_ReportWidget->setHtml(report.ReportHtml(m_ReportHistory.At(reportIndex-1), false));
	else
		m_ReportWidget->setHtml(report.ReportHtml(report, false));

	if(m_ReportWidget->verticalScrollBar())
		m_ReportWidget->verticalScrollBar()->setValue(vscrollPos);
}

void AuCityHistoryDlg::ReportCityLast()
{
	if(m_CityId.IsNull())
		return;
	auApp->ReportCityLast(m_CityId);
	accept();
}

