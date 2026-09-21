//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailsettings.cpp,v 1.9 2022/02/19 18:36:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailsettings.h"
//
#include "auglobal.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

AuMailSettings::AuMailSettings(QWidget* parent) : AuDialog(parent)
{
	m_IconIndex = 0;
	m_timer = 0;

	setAuDlgTitle(tr("Hazeron Mail Settings"));
	setAuDlgIcon(QIcon(":data/Settings.png"));

	ckmail = new QComboBox(this);
	ckmail->addItem(QIcon(":data/MailInactive.png"), tr("Do Not Check for New Messages"));
	ckmail->addItem(QIcon(":data/MailActive2.png"), tr("Check for New Message at Startup"));
	ckmail->addItem(QIcon(":data/MailActive3.png"), tr("Check for New Messages Periodically"));
	ckmail->setCurrentIndex(g_Au.MailNotificationMode());
	ckmail->setToolTip(tr("<html>Specifies when mail should be checked for new messages, "
				"when the mail window is not visible.<br><br>"
				"Mail is checked periodically when the mail window is visible, "
				"regardless of this setting.<br><br>"
				"New mail produces a message on the Thoughts comm channel."
				"</html>"));

	//--City
	citycb = new QCheckBox(tr("Automatically delete routine city reports."), this);
	citycb->setCheckState(g_Au.MailCityReportAutoDelete() ? Qt::Checked : Qt::Unchecked);
	citycb->setToolTip(tr("<html>This excludes the final decay messages of dying cities. "
				"Those messages are not automatically deleted.</html>"));

	QLabel* citylb = new QLabel(tr("Age to delete city reports, in hours. "
				"Cities create a report about every 13 minutes."), this);
	citylb->setWordWrap(true);

	cityhr = new QSpinBox;
	cityhr->setMinimum(1);
	cityhr->setMaximum(100);
	cityhr->setValue(g_Au.MailCityReportOldAge());

	//--Fleet
	fleetcb = new QCheckBox(tr("Automatically delete routine fleet contacts."), this);
	fleetcb->setCheckState(g_Au.MailFleetReportAutoDelete() ? Qt::Checked : Qt::Unchecked);
	fleetcb->setToolTip(tr("<html>This excludes the final death messages of dying officers and destroyed spacecraft. "
				"This also excludes scan reports. "
				"Those messages are not automatically deleted.</html>"));

	QLabel* fleetlb = new QLabel(tr("Age to delete fleet contacts, in hours. "
				"Fleet officers send a ship log or contact report every six hours."), this);
	fleetlb->setWordWrap(true);

	fleethr = new QSpinBox;
	fleethr->setMinimum(1);
	fleethr->setMaximum(100);
	fleethr->setValue(g_Au.MailFleetReportOldAge());

	//--Officer
	officercb = new QCheckBox(tr("Automatically delete cadet assignment requests."), this);
	officercb->setCheckState(g_Au.MailOfficerReportAutoDelete() ? Qt::Checked : Qt::Unchecked);

	QLabel* officerlb = new QLabel(tr("Age to delete assignment requests, in hours. "
				"Cadets who are ready for assignment send an assignment request hourly."), this);
	officerlb->setWordWrap(true);

	officerhr = new QSpinBox;
	officerhr->setMinimum(1);
	officerhr->setMaximum(100);
	officerhr->setValue(g_Au.MailOfficerReportOldAge());

	QPushButton* defaultbutton = new QPushButton(tr("Restore Defaults"), this);
	connect(defaultbutton, SIGNAL(clicked()), SLOT(RestoreDefaults()));

	QPushButton* okbutton = new QPushButton(tr("Ok"), this);
	okbutton->setAutoDefault(true);
	okbutton->setDefault(true);
	connect(okbutton, SIGNAL(clicked()), SLOT(Accept()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(ckmail, row,0, 1,3);
	row++;
	lay->addWidget(citycb, row,0, 1,3);
	row++;
	lay->addWidget(citylb, row,1, 1,1);
	lay->addWidget(cityhr, row,2, 1,1);
	row++;
	lay->addWidget(fleetcb, row,0, 1,3);
	row++;
	lay->addWidget(fleetlb, row,1, 1,1);
	lay->addWidget(fleethr, row,2, 1,1);
	row++;
	lay->addWidget(officercb, row,0, 1,3);
	row++;
	lay->addWidget(officerlb, row,1, 1,1);
	lay->addWidget(officerhr, row,2, 1,1);
	row++;
	lay->setRowStretch(row, 1);
	row++;
	lay->addWidget(defaultbutton, row,1, 1,1);
	lay->addWidget(okbutton, row,2, 1,1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuMailSettings::~AuMailSettings()
{
}

void AuMailSettings::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		//RestoreGeometry();
	}
	else if(isVisible())
	{
		//SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuMailSettings::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuMailSettings/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuMailSettings/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuMailSettings/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuMailSettings/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuMailSettings::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuMailSettings/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuMailSettings/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuMailSettings/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuMailSettings/SizeY",sz.height());
}

void AuMailSettings::RestoreDefaults()
{
	ckmail->setCurrentIndex(1);

	citycb->setCheckState(Qt::Checked);
	cityhr->setValue(4);

	fleetcb->setCheckState(Qt::Checked);
	fleethr->setValue(6);

	officercb->setCheckState(Qt::Checked);
	officerhr->setValue(1);
}

void AuMailSettings::Accept()
{
	g_Au.SetMailCityReportAutoDelete(citycb->checkState() == Qt::Checked);
	g_Au.SetMailCityReportOldAge(cityhr->value());
	g_Au.SetMailFleetReportAutoDelete(fleetcb->checkState() == Qt::Checked);
	g_Au.SetMailFleetReportOldAge(fleethr->value());
	g_Au.SetMailNotificationMode(ckmail->currentIndex());
	g_Au.SetMailOfficerReportAutoDelete(officercb->checkState() == Qt::Checked);
	g_Au.SetMailOfficerReportOldAge(officerhr->value());
	accept();
}

void AuMailSettings::showEvent(QShowEvent* event)
{
	AuDialog::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(250);

	UpdateContents();
}

void AuMailSettings::hideEvent(QHideEvent* event)
{
	AuDialog::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();
}

void AuMailSettings::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	m_IconIndex = (m_IconIndex + 1) % 6;
	if(!m_IconIndex || m_IconIndex > 3)
	{
		QIcon icon(QString(":data/MailActive0.png"));
		ckmail->setItemIcon(2, icon);
	}
	else
	{
		QIcon icon(QString(":data/MailActive%1.png").arg(m_IconIndex));
		ckmail->setItemIcon(2, icon);
	}
}
