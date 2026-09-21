//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatorpos.cpp,v 1.24 2022/05/04 21:50:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulocatorpos.h"
//
#include "auapp.h"
#include "aucompasswidget.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

void AuLocatorCompassDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleLocatorCompassUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLocatorPos::AuLocatorPos(QWidget* parent) : QWidget(parent)
{
	m_Timer = 0;

	m_City = new QLabel(this);
	m_City->setToolTip(tr("<html>"
				"City and Building"
				"</html"));

	m_CityIcon = new QLabel(this);
	m_CityIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_CityIcon->setPixmap(QPixmap(":data/City.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_CityIcon->setToolTip(tr("<html>"
				"City and Building"
				"</html"));

	m_Compass = new AuCompassWidget(this);
	m_Compass->setToolTip(tr("<html>"
				"Facing Direction"
				"</html"));

	m_Galaxy = new QLabel(this);
	m_Galaxy->setToolTip(tr("<html>"
				"Galaxy"
				"</html"));

	m_GalaxyIcon = new QLabel(this);
	m_GalaxyIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_GalaxyIcon->setPixmap(QPixmap(":data/RangeGalaxy.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_GalaxyIcon->setToolTip(tr("<html>"
				"Galaxy"
				"</html"));

	m_LongLat = new QLineEdit(this);
	m_LongLat->setReadOnly(true);
	m_LongLat->setToolTip(tr("<html>"
				"Longitude and Latitude"
				"</html"));

	m_Sector = new QLabel(this);
	m_Sector->setToolTip(tr("<html>"
				"Sector"
				"</html"));

	m_SectorIcon = new QLabel(this);
	m_SectorIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_SectorIcon->setPixmap(QPixmap(":data/RangeSector.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_SectorIcon->setToolTip(tr("<html>"
				"Sector"
				"</html"));

	m_Ship = new QLabel(this);
	m_Ship->setToolTip(tr("<html>"
				"Spacecraft"
				"</html"));

	m_ShipIcon = new QLabel(this);
	m_ShipIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_ShipIcon->setPixmap(QPixmap(":data/c_Spacecraft.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_ShipIcon->setToolTip(tr("<html>"
				"Spacecraft"
				"</html"));

	m_System = new QLabel(this);
	m_System->setToolTip(tr("<html>"
				"Solar System"
				"</html"));

	m_SystemIcon = new QLabel(this);
	m_SystemIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_SystemIcon->setPixmap(QPixmap(":data/RangeSystem.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_SystemIcon->setToolTip(tr("<html>"
				"Solar System"
				"</html"));

	m_Weather = new QLabel(this);
	m_Weather->setToolTip(tr("<html>"
				"Weather Conditions Outside"
				"</html"));

	m_WeatherIcon = new QLabel(this);
	m_WeatherIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_WeatherIcon->setPixmap(QPixmap(":data/Anemometer.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_WeatherIcon->setToolTip(tr("<html>"
				"Weather Conditions Outside"
				"</html"));

	m_World = new QLabel(this);
	m_World->setWordWrap(true);	// This can get long with the 'claimed by' part.
	m_World->setToolTip(tr("<html>"
				"World or Star"
				"</html"));

	m_WorldIcon = new QLabel(this);
	m_WorldIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_WorldIcon->setPixmap(QPixmap(":data/RangeHail.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_WorldIcon->setToolTip(tr("<html>"
				"World or Star"
				"</html"));

	QGridLayout* vlay = new QGridLayout;
	int row=0;
	vlay->addWidget(m_ShipIcon, row,0);
	vlay->addWidget(m_Ship, row,1);
	row++;
	vlay->addWidget(m_CityIcon, row,0);
	vlay->addWidget(m_City, row,1);
	row++;
	vlay->setRowStretch(row,1);
	row++;
	vlay->addWidget(m_WeatherIcon, row,0);
	vlay->addWidget(m_Weather, row,1);
	row++;
	vlay->setRowStretch(row,1);
	row++;
	vlay->addWidget(m_WorldIcon, row,0);
	vlay->addWidget(m_World, row,1);
	row++;
	vlay->addWidget(m_LongLat, row,1);
	row++;
	vlay->addWidget(m_SystemIcon, row,0);
	vlay->addWidget(m_System, row,1);
	row++;
	vlay->addWidget(m_SectorIcon, row,0);
	vlay->addWidget(m_Sector, row,1);
	row++;
	vlay->addWidget(m_GalaxyIcon, row,0);
	vlay->addWidget(m_Galaxy, row,1);
	row++;
	vlay->setColumnStretch(1,1);
	vlay->setSpacing(2);
	vlay->setMargin(0);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(vlay, 2);
	lay->addWidget(m_Compass, 1);
	lay->setMargin(2);
	setLayout(lay);
}

AuLocatorPos::~AuLocatorPos()
{
}

QSize AuLocatorPos::sizeHint() const
{
	return QSize(500,300);
}

void AuLocatorPos::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(250);
}

void AuLocatorPos::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* pView = auApp->Frame()->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	double rheading = pView->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct,
			sysNameDesc, secDesc, secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	m_Compass->SetHeading(rheading);
	m_Compass->setVisible(!secName.isEmpty());

	// Ship info
	QString ship;
	if(!shipname.isEmpty())
	{
		ship = shipname;
		if(!shipinfo.isEmpty())
			ship = QString("<b>%1</b><br>%2").arg(shipname).arg(shipinfo);
		else
			ship = QString("<b>%1</b>").arg(shipname);
	}
	if(m_Ship->text() != ship)
	{
		m_Ship->setText(ship);

		// Update the icon too.
		if(!SpacecraftStructureIcon(shipStruct).isEmpty())
		{
			QPixmap px = QPixmap(SpacecraftStructureIcon(shipStruct)).scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			m_ShipIcon->setPixmap(px);
			m_ShipIcon->setToolTip(tr("<html>"
						"%1"
						"</html")
					.arg(SpacecraftStructureName(shipStruct)));
		}
	}
	m_Ship->setVisible(!ship.isEmpty());
	m_ShipIcon->setVisible(!ship.isEmpty());

	// City location
	QString city;
	if(!scity.isEmpty())
		city = QString("<b>%1</b><br>%2").arg(scity).arg(sdev);
	if(m_City->text() != city)
	{
		m_City->setText(city);

		const DgData& gd = g_Au.LockDataForRead();
		QString iname = gd.IndustryName(iid);
		g_Au.UnlockData();

		// Update the icon too.
		if(DbIndustry::BitmapName(iid))
		{
			QPixmap px = QPixmap(DbIndustry::BitmapName(iid)).scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			m_CityIcon->setPixmap(px);
			m_CityIcon->setToolTip(tr("<html>"
						"%1"
						"</html")
					.arg(iname));
		}
	}
	m_City->setVisible(!city.isEmpty());
	m_CityIcon->setVisible(!city.isEmpty());

	// Weather conditions
	QString weather(wx);
	if(!wind.isEmpty())
		weather += "<br>" + wind;
	if(m_Weather->text() != weather)
		m_Weather->setText(weather);
	m_Weather->setVisible(!weather.isEmpty());
	m_WeatherIcon->setVisible(!weather.isEmpty());

	// Longitude/latitude coordinate
	if(m_LongLat->text() != slonglat)
		m_LongLat->setText(slonglat);
	m_LongLat->setVisible(!slonglat.isEmpty());

	// World
	if(m_World->text() != worldName)
	{
		m_World->setText(worldName);

		// Update the icon too.
		if(!BodyTypeIcon(bodyType).isEmpty())
		{
			QPixmap px = QPixmap(BodyTypeIcon(bodyType)).scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			m_WorldIcon->setPixmap(px);
			m_WorldIcon->setToolTip(tr("<html>"
						"%1"
						"</html")
					.arg(BodyTypeName(bodyType)));
			m_World->setToolTip(tr("<html>"
						"%1"
						"</html")
					.arg(BodyTypeName(bodyType)));
		}
		else
		{
			QPixmap px = QPixmap(":data/EmptySpace.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			m_WorldIcon->setPixmap(px);
			m_WorldIcon->setToolTip(tr("<html>"
						"Empty Space"
						"</html"));
			m_World->setToolTip(tr("<html>"
						"Empty Space"
						"</html"));
		}
	}
	m_World->setVisible(!worldName.isEmpty());
	m_WorldIcon->setVisible(!worldName.isEmpty());

	// System
	if(m_System->text() != sysNameDesc)
		m_System->setText(sysNameDesc);
	m_System->setVisible(!sysNameDesc.isEmpty());
	m_SystemIcon->setVisible(!sysNameDesc.isEmpty());

	// Sector
	QString sector;
	if(!secName.isEmpty())
		sector = QString("%1 %2").arg(secName).arg(secDesc);
	if(m_Sector->text() != sector)
		m_Sector->setText(sector);
	m_Sector->setVisible(!sector.isEmpty());
	m_SectorIcon->setVisible(!sector.isEmpty());

	// Galaxy
	if(m_Galaxy->text() != galName)
		m_Galaxy->setText(galName);
	m_Galaxy->setVisible(!galName.isEmpty());
	m_GalaxyIcon->setVisible(!galName.isEmpty());
}

