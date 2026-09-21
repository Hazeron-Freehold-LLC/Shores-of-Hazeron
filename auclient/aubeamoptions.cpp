//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubeamoptions.cpp,v 1.11 2022/02/19 18:07:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubeamoptions.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "autoolbutton.h"
#include "auview.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuBeamOptions::AuBeamOptions(QWidget* parent, const AuGlobeLLF& ll) : AuDialog(parent)
{
	setObjectName("BeamOptions");
	setAuDlgTitle(tr("Request Transport"));
	setAuDlgIcon(QIcon(":data/MsgTransport.png"));

	QLabel* lb1 = new QLabel(tr("How many to transport?"));

	m_BeamDirection = 0;

	m_BeamCountSpin = new QSpinBox(this);
	m_BeamCountSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BeamCountSpin->selectAll();
	m_BeamCountSpin->setMinimum(1);
	m_BeamCountSpin->setMaximum(255);
	m_BeamCountSpin->setValue(1);
	m_BeamCountSpin->setToolTip(tr("<html>Enter the number of transporter beams to energize.</html>"));

	QLabel* lb2 = new QLabel(tr("Destination"));

	m_BeamToCity = new QRadioButton(tr("Places and People"), this);
	m_BeamToCity->setToolTip(tr("<html>Destination is a person or place within transporter range. "
				"</html>"));
	connect(m_BeamToCity, SIGNAL(clicked()), SLOT(DestinationCity()));

	m_BeamToCoord = new QRadioButton(tr("XYZ Coordinate"), this);			// 3D coordinate
	m_BeamToCoord->setToolTip(tr("<html>Destination is a coordinate location relative to this spacecraft.</html>"));
	connect(m_BeamToCoord, SIGNAL(clicked()), SLOT(DestinationCoord()));

	m_BeamToPlanet = new QRadioButton(tr("World Longitude/Latitude"), this);			// Longitude/latitude of current planet.
	m_BeamToPlanet->setToolTip(tr("<html>Destination is a longitude and latitude location on the world orbited by this "
				"spacecraft.</html>"));
	connect(m_BeamToPlanet, SIGNAL(clicked()), SLOT(DestinationPlanet()));

	m_BeamToShip = new QRadioButton(tr("Spacecraft"), this);			// Somewhere aboard ship.
	m_BeamToShip->setToolTip(tr("<html>Destination is inside the hull of a spacecraft. "
				"A void space is located at random.</html>"));
	connect(m_BeamToShip, SIGNAL(clicked()), SLOT(DestinationShip()));

	m_BeamToTransporter = new QRadioButton(tr("Transporter"), this);	// Transporter
	m_BeamToTransporter->setToolTip(tr("<html>Destination is a transporter.</html>"));
	connect(m_BeamToTransporter, SIGNAL(clicked()), SLOT(DestinationTransporter()));

	m_Cities = new QComboBox(this);
	m_Cities->setToolTip(tr("<html>Places that are in transporter range.<br>"
				"Click the <i>Places</i> radio button to update the list of places, based on current transporter range.</html>"));

	m_Coord = new QLineEdit("0,0,0", this);
	m_Coord->setToolTip(tr("<html>Offset relative to this spacecraft. "
				"Cartesian or polar coordinates can be entered. "
				"Units of measure are meters and degrees as appropriate.<br>"
				"<b>Cartesian coordinates</b> are expressed as <i>X,Y</i> or <i>X,Y,Z</i> values, "
				"representing distances from the center of the ship.<br>"
				"%1 Positive <i>X</i> is starboard; negative <i>X</i> is port.<br>"
				"%1 Positive <i>Y</i> is forward; negative <i>Y</i> is aft.<br>"
				"%1 Positive <i>Z</i> is aloft; negative <i>Z</i> is below.<br>"
				"%1 e.g. <i>23,4</i> is 23m starboard and 4m fore of the ship's center and <i>-3.4,92,-12.321</i> is 3.4m port "
				"92m fore and 12.321m below the ship.<br>"
				"<b>Polar coordinates</b> are expressed with <i>p</i> followed by <i>Distance,Angle</i> or "
				"<i>Distance,Angle,Incline</i> values.<br>"
				"%1 <i>Distance</i> is the distance from the center of the ship.<br>"
				"%1 <i>Angle</i> is the angle direction in the XY plane, where 0 is starboard, 90 is fore, 180 is port, "
				"and 270 is aft.<br>"
				"%1 <i>Incline</i> is the angle above or below the XY plane, where 90 is aloft and -90 is below.<br>"
				"%1 e.g. <i>p12,90</i> is 12m foreward of the ship and <i>p100.3,-90,45</i> is 100.3m behind and above the ship." 
				"</html>").arg(g_Au.MiddleDotSymbol()));

	m_LongLat = new QLineEdit(this);
	if(ll.Longitude() != 0 || ll.Latitude() != 0)
		m_LongLat->setText(ll.ToString());
	m_LongLat->setToolTip(tr("<html>Longitude and latitude location on the world this spacecraft is orbiting. "
				"Degree symbols are optional. e.g. '123.4E 32.1N'.<br>"
				"Click the <i>World</i> radio button to update longitude and latitude based on your current location.</html>"));
	connect(m_LongLat, SIGNAL(textEdited(const QString&)), SLOT(PlanetLongLatChanged(const QString&)));

	m_Spacecraft = new QComboBox(this);
	m_Spacecraft->setToolTip(tr("<html>Spacecraft that are in transporter range.<br>"
				"Click the <i>Spacecraft</i> radio button to update spacecraft list contents based on current transporter range.</html>"));

	m_Transporters = new QComboBox(this);
	m_Transporters->setToolTip(tr("<html>Transporters that are in transporter range.<br>"
				"Click the <i>Transporter</i> radio button to update transporter list contents based on current transporter range.</html>"));

	m_BeamDown = new QPushButton(tr("Beam Down"), this);
	m_BeamDown->setToolTip(tr("<html>Request transporter beam <b>to</b> the destination. "
				"The request is transmitted on the current comm channel.</html>"));
	connect(m_BeamDown, SIGNAL(clicked()), SLOT(BeamDown()));

	m_BeamUp = new QPushButton(tr("Beam Up"), this);
	m_BeamUp->setToolTip(tr("<html>Request transporter beam <b>from</b> the destination. "
				"The request is transmitted on the current comm channel.</html>"));
	connect(m_BeamUp, SIGNAL(clicked()), SLOT(BeamUp()));

	// Filter buttons for things in the people and places list.
	QHBoxLayout* flay = new QHBoxLayout;
	flay->addWidget(m_Cities, 1);
	for(int tt=TT_Bd; tt <= TT_GeologicalFeatureFirst; tt++)
	{
		AuToolButtonValue* tb = new AuToolButtonValue(TTIcon((TRANSPORTER_TARGET)tt), tt, this);
		tb->setCheckable(true);
		tb->setChecked(true);
		tb->hide();
		connect(tb, SIGNAL(ClickedValue(int)), SLOT(TransporterFilterClicked(int)));
		flay->addWidget(tb);
		m_TTFilter.insert((TRANSPORTER_TARGET)tt, tb);
	}

	// Layout the controls.
	QGridLayout* lay = new QGridLayout();
	int row = 0;
	lay->addWidget(lb1, row, 0);
	lay->addWidget(m_BeamCountSpin, row,1);
	row++;
	lay->addWidget(lb2, row, 0);
	row++;
	lay->addWidget(m_BeamToCity, row, 0);
	lay->addLayout(flay, row,1, 1,6);
	row++;
	lay->addWidget(m_BeamToCoord, row, 0);
	lay->addWidget(m_Coord, row,1, 1,6);
	row++;
	lay->addWidget(m_BeamToPlanet, row,0);
	lay->addWidget(m_LongLat, row,1, 1,6);
	row++;
	lay->addWidget(m_BeamToShip, row,0);
	lay->addWidget(m_Spacecraft, row,1, 1,6);
	row++;
	lay->addWidget(m_BeamToTransporter, row,0);
	lay->addWidget(m_Transporters, row,1, 1,6);
	row++;
	lay->addWidget(m_BeamUp, row, 1, 1,3);
	lay->addWidget(m_BeamDown, row, 4, 1,3);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	// Sub controls are hidden initially. Radio button selection shows the ones that are needed.
	m_Cities->hide();
	m_Coord->hide();
	m_Spacecraft->hide();
	m_Transporters->hide();
	if(ll.Longitude() != 0 || ll.Latitude() != 0)
	{
		// Beam to planet mode is implied when longitude and latitude are supplied.
		m_BeamToPlanet->setChecked(true);
	}
	else
	{
		m_LongLat->hide();

		// Disable beam commands until a mode is selected.
		m_BeamDown->setEnabled(false);
		m_BeamUp->setEnabled(false);
	}
}

AuBeamOptions::~AuBeamOptions()
{
}

void AuBeamOptions::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BeamOptions/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BeamOptions/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BeamOptions/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BeamOptions/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);

	int beamCount = g_Au.Settings().readNumEntry("Widget/BeamOptions/BeamCount", 1);
	if(beamCount >= 1 && beamCount <= 255)
		m_BeamCountSpin->setValue(beamCount);
}

void AuBeamOptions::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BeamOptions/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BeamOptions/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BeamOptions/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BeamOptions/SizeY",sz.height());

	g_Au.Settings().writeEntry("Widget/BeamOptions/BeamCount", m_BeamCountSpin->value());
}

void AuBeamOptions::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

unsigned int AuBeamOptions::BeamCount() const
{
	// Returns the final value the user entered if the dialog is accepted.
	return m_BeamCountSpin->value();
}

void AuBeamOptions::BeamDown()
{
	m_BeamDirection = 0;
	accept();
}

void AuBeamOptions::BeamUp()
{
	m_BeamDirection = 1;
	accept();
}

void AuBeamOptions::DestinationCity()
{
	m_Coord->hide();
	m_LongLat->hide();
	m_Spacecraft->hide();
	m_Transporters->hide();

	m_Cities->show();
	for(QMap<TRANSPORTER_TARGET, AuToolButtonValue*>::iterator ti = m_TTFilter.begin(); ti != m_TTFilter.end(); ti++)
		(*ti)->hide();
	m_Cities->clear();
	m_Targets.clear();

	// Disable beam buttons until a city is selected.
	m_BeamDown->setEnabled(false);
	m_BeamUp->setEnabled(false);

	auApp->TransporterPlaces();	// Request updated list of places from server.
}

void AuBeamOptions::DestinationCoord()
{
	m_Cities->hide();
	for(QMap<TRANSPORTER_TARGET, AuToolButtonValue*>::iterator ti = m_TTFilter.begin(); ti != m_TTFilter.end(); ti++)
		(*ti)->hide();
	m_LongLat->hide();
	m_Spacecraft->hide();
	m_Transporters->hide();

	m_Coord->show();

	m_BeamDown->setEnabled(true);
	m_BeamUp->setEnabled(true);
}

void AuBeamOptions::DestinationPlanet()
{
	m_Cities->hide();
	for(QMap<TRANSPORTER_TARGET, AuToolButtonValue*>::iterator ti = m_TTFilter.begin(); ti != m_TTFilter.end(); ti++)
		(*ti)->hide();
	m_Coord->hide();
	m_Spacecraft->hide();
	m_Transporters->hide();

	m_LongLat->show();

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* view = auApp->Frame()->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	view->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc,
			secName, oat, wx, wind, sysPos, secPos, galPos, galName);
	if(!slonglat.isEmpty())
	{
		m_LongLat->setText(slonglat);
		m_LongLat->setEnabled(true);

		m_BeamDown->setEnabled(true);
		m_BeamUp->setEnabled(true);
	}
	else
	{
		m_LongLat->setText(tr("Not orbiting a world."));
		m_LongLat->setEnabled(false);

		m_BeamDown->setEnabled(false);
		m_BeamUp->setEnabled(false);
	}
}

void AuBeamOptions::DestinationShip()
{
	m_Cities->hide();
	for(QMap<TRANSPORTER_TARGET, AuToolButtonValue*>::iterator ti = m_TTFilter.begin(); ti != m_TTFilter.end(); ti++)
		(*ti)->hide();
	m_Coord->hide();
	m_LongLat->hide();
	m_Transporters->hide();

	m_Spacecraft->show();
	m_Spacecraft->clear();

	// Disable beam buttons until a ship is selected.
	m_BeamDown->setEnabled(false);
	m_BeamUp->setEnabled(false);

	auApp->TransporterSpacecraft();	// Request updated list of spacecraft from server.
}

void AuBeamOptions::DestinationTransporter()
{
	m_Cities->hide();
	for(QMap<TRANSPORTER_TARGET, AuToolButtonValue*>::iterator ti = m_TTFilter.begin(); ti != m_TTFilter.end(); ti++)
		(*ti)->hide();
	m_Coord->hide();
	m_LongLat->hide();
	m_Spacecraft->hide();

	m_Transporters->show();
	m_Transporters->clear();

	// Disable beam buttons until a ship is selected.
	m_BeamDown->setEnabled(false);
	m_BeamUp->setEnabled(false);

	auApp->TransporterTransporters();
}

void AuBeamOptions::TransporterPlace(const AuTransporterTarget& tt)
{
	// Add the target to our list of known places.
	m_Targets.append(tt);
	ListTransporterPlace(tt);
}

void AuBeamOptions::ListTransporterPlace(const AuTransporterTarget& tt)
{
	// Insure the appropriate filter button is visible for the target type. Get visibility from its state.
	bool listIt = true;
	QMap<TRANSPORTER_TARGET, AuToolButtonValue*>::iterator ti = m_TTFilter.find(tt.TargetType());
	if(ti != m_TTFilter.end())
	{
		(*ti)->show();
		listIt = (*ti)->isChecked();
	}

	// If the target is visible according to the filter, add it to the cities combo box.
	if(listIt)
	{
		QString sid = QString("%1.%2").arg(tt.Id().GetID()).arg(tt.Target());
		m_Cities->addItem(QIcon(tt.Icon()), tt.Name(), sid);
		int index = m_Cities->findData(sid);
		m_Cities->setItemData(index, tt.Id().GetID(), Qt::UserRole+1);
		m_Cities->setItemData(index, (quint8)tt.Target(), Qt::UserRole+2);
	}

	// Enable controls.
	m_BeamDown->setEnabled(m_Cities->currentIndex() >= 0);
	m_BeamUp->setEnabled(m_Cities->currentIndex() >= 0);
}

void AuBeamOptions::TransporterShip(const AuId& id, const QString& name, const QString& desc)
{
	m_Spacecraft->addItem(QIcon(":data/c_Spacecraft.png"), desc, id.GetID());
	int index = m_Spacecraft->findData(id.GetID());
	m_Spacecraft->setItemData(index, name, Qt::UserRole+1);

	m_BeamDown->setEnabled(m_Spacecraft->currentIndex() >= 0);
	m_BeamUp->setEnabled(m_Spacecraft->currentIndex() >= 0);
}

void AuBeamOptions::TransporterTransporter(const AuId& id, const QString& name, const QString& desc, quint16 whichTransporter)
{
	QString descT = tr("%1, Transporter %2").arg(desc).arg(whichTransporter);
	QString key = QString("%1:%2").arg(id.GetID()).arg(whichTransporter);
	m_Transporters->addItem(QIcon(":data/MsgTransport.png"), descT, key);
	int index = m_Transporters->findData(key);
	m_Transporters->setItemData(index, name, Qt::UserRole+1);
	m_Transporters->setItemData(index, whichTransporter, Qt::UserRole+3);
	m_Transporters->setItemData(index, id.GetID(), Qt::UserRole+4);

	m_BeamDown->setEnabled(m_Transporters->currentIndex() >= 0);
	m_BeamUp->setEnabled(m_Transporters->currentIndex() >= 0);
}

unsigned int AuBeamOptions::BeamDestination() const
{
	if(m_BeamToCity->isChecked())
		return 0;
	else if(m_BeamToCoord->isChecked())
		return 1;
	else if(m_BeamToPlanet->isChecked())
		return 2;
	else if(m_BeamToShip->isChecked())
		return 3;
	else if(m_BeamToTransporter->isChecked())
		return 4;

	// Return an error response because no destination is selected.
	return 5;
}

unsigned int AuBeamOptions::BeamDirection() const
{
	return m_BeamDirection;
}

AuId AuBeamOptions::CityId() const
{
	if(m_Cities->currentIndex() >= 0)
		return AuId(m_Cities->itemData(m_Cities->currentIndex(), Qt::UserRole+1).toUInt());
	return AuId(0);
}

TRANSPORTER_TARGET AuBeamOptions::Target()
{
	if(m_Cities->currentIndex() >= 0)
		return (TRANSPORTER_TARGET)m_Cities->itemData(m_Cities->currentIndex(), Qt::UserRole+2).toUInt();
	return TT_Invalid;
}

QString AuBeamOptions::CityName() const
{
	if(m_Cities->currentIndex() >= 0)
		return m_Cities->itemText(m_Cities->currentIndex());
	return QString("");
}

float AuBeamOptions::CoordX() const
{
	AuPoint coord(m_Coord->text());
	return coord.x;
}

float AuBeamOptions::CoordY() const
{
	AuPoint coord(m_Coord->text());
	return coord.y;
}

float AuBeamOptions::CoordZ() const
{
	AuPoint coord(m_Coord->text());
	return coord.z;
}

float AuBeamOptions::Latitude() const
{
	float longitude, latitude;
	if(AuGlobeLLF::ParseLongitudeLatitude(m_LongLat->text(), longitude, latitude))
		return latitude;
	return 0;
}

float AuBeamOptions::Longitude() const
{
	float longitude, latitude;
	if(AuGlobeLLF::ParseLongitudeLatitude(m_LongLat->text(), longitude, latitude))
		return longitude;
	return 0;
}

AuId AuBeamOptions::ShipId() const
{
	if(m_Spacecraft->currentIndex() >= 0)
		return AuId(m_Spacecraft->itemData(m_Spacecraft->currentIndex()).toUInt());
	return AuId(0);
}

QString AuBeamOptions::ShipName() const
{
	if(m_Spacecraft->currentIndex() >= 0)
		return m_Spacecraft->itemData(m_Spacecraft->currentIndex(), Qt::UserRole+1).toString();
	return QString("");
}

AuId AuBeamOptions::TransporterId() const
{
	if(m_Transporters->currentIndex() >= 0)
		return AuId(m_Transporters->itemData(m_Transporters->currentIndex(), Qt::UserRole+4).toUInt());
	return AuId(0);
}

QString AuBeamOptions::TransporterName() const
{
	if(m_Transporters->currentIndex() >= 0)
		return m_Transporters->itemData(m_Transporters->currentIndex(), Qt::UserRole+1).toString();
	return QString("");
}

quint16 AuBeamOptions::TransporterWhich() const
{
	if(m_Transporters->currentIndex() >= 0)
		return m_Transporters->itemData(m_Transporters->currentIndex(), Qt::UserRole+3).toUInt();
	return 0;
}

void AuBeamOptions::PlanetLongLatChanged(const QString& longLat)
{
	bool isValid = AuGlobeLL::IsLongLatStringValid(longLat);
	m_BeamDown->setEnabled(isValid);
	m_BeamUp->setEnabled(isValid);
}

void AuBeamOptions::TransporterFilterClicked(int targetType)
{
	QMap<TRANSPORTER_TARGET, AuToolButtonValue*>::iterator ti = m_TTFilter.find((TRANSPORTER_TARGET)targetType);
	if(ti == m_TTFilter.end())
		return;

	// Update the list of visible targets
	m_Cities->clear();
	for(QList<AuTransporterTarget>::const_iterator ti = m_Targets.cbegin(); ti != m_Targets.cend(); ti++)
		ListTransporterPlace(*ti);
}
