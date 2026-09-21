//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatorgps.cpp,v 1.7 2022/02/19 18:16:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulocatorgps.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include <daperson.h>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

void AuLocatorGPSDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleLocatorGPSUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLocatorGPS::AuLocatorGPS(QWidget* parent) : QWidget(parent)
{
	// Solar System
	QLabel* systemLabel = new QLabel(tr("Solar System Coordinate"));

	QLabel* systemIcon = new QLabel();
	systemIcon->setPixmap(QPixmap(":data/RangeSystem.png"));

	QLabel* systemUnitAbr = new QLabel(tr("au"));

	m_SystemPos = new QLineEdit(this);
	m_SystemPos->setReadOnly(true);
	m_SystemPos->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_SystemPos->setToolTip(tr("<html>Your location relative to the center of the primary star in the current solar system, "
				"measured in astronomical units (au).<br>"
				"%1 +X is east in the ecliptic plane.<br>"
				"%1 -X is west in the ecliptic plane.<br>"
				"%1 +Y is north in the ecliptic plane.<br>"
				"%1 -Y is south in the ecliptic plane.<br>"
				"%1 +Z is above the ecliptic plane.<br>"
				"%1 -Z is below the ecliptic plane."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	// Sector
	QLabel* sectorLabel = new QLabel(tr("Sector Coordinate"));

	QLabel* sectorIcon = new QLabel();
	sectorIcon->setPixmap(QPixmap(":data/RangeSector.png"));

	QLabel* sectorUnitAbr = new QLabel(tr("pc"));

	m_SectorPos = new QLineEdit(this);
	m_SectorPos->setReadOnly(true);
	m_SectorPos->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_SectorPos->setToolTip(tr("<html>Your location relative to the center of the current sector, "
				"measured in parsecs (pc).<br>"
				"%1 +X is east in the galactic plane.<br>"
				"%1 -X is west in the galactic plane.<br>"
				"%1 +Y is north in the galactic plane.<br>"
				"%1 -Y is south in the galactic plane.<br>"
				"%1 +Z is above the galactic plane.<br>"
				"%1 -Z is below the galactic plane."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	// Galaxy
	QLabel* galaxyLabel = new QLabel(tr("Galaxy Coordinate"));

	QLabel* galaxyIcon = new QLabel();
	galaxyIcon->setPixmap(QPixmap(":data/RangeGalaxy.png"));

	QLabel* galaxyUnitAbr = new QLabel(tr("pc"));

	m_GalaxyPos = new QLineEdit(this);
	m_GalaxyPos->setReadOnly(true);
	m_GalaxyPos->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_GalaxyPos->setToolTip(tr("<html>Your location relative to the center of the current galaxy, "
				"measured in parsecs (pc).<br>"
				"%1 +X is east in the galactic plane.<br>"
				"%1 -X is west in the galactic plane.<br>"
				"%1 +Y is north in the galactic plane.<br>"
				"%1 -Y is south in the galactic plane.<br>"
				"%1 +Z is above the galactic plane.<br>"
				"%1 -Z is below the galactic plane."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	// Layout
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(systemIcon, row,0, 1,1);
	lay->addWidget(systemLabel, row,1, 1,9);
	row++;
	lay->addWidget(m_SystemPos, row,0, 1,9);
	lay->addWidget(systemUnitAbr, row,9, 1,1);
	row++;
	lay->addWidget(sectorIcon, row,0, 1,1);
	lay->addWidget(sectorLabel, row,1, 1,9);
	row++;
	lay->addWidget(m_SectorPos, row,0, 1,9);
	lay->addWidget(sectorUnitAbr, row,9, 1,1);
	row++;
	lay->addWidget(galaxyIcon, row,0, 1,1);
	lay->addWidget(galaxyLabel, row,1, 1,9);
	row++;
	lay->addWidget(m_GalaxyPos, row,0, 1,9);
	lay->addWidget(galaxyUnitAbr, row,9, 1,1);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	m_timer = 0;
}

AuLocatorGPS::~AuLocatorGPS()
{
}

QSize AuLocatorGPS::sizeHint() const
{
	return QSize(300,200);
}

void AuLocatorGPS::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(250);
}

void AuLocatorGPS::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* pView = auApp->Frame()->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	pView->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc,
			secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	if(m_GalaxyPos->text() != galPos)
		m_GalaxyPos->setText(galPos);

	if(m_SectorPos->text() != secPos)
		m_SectorPos->setText(secPos);

	if(m_SystemPos->text() != sysPos)
		m_SystemPos->setText(sysPos);
}
