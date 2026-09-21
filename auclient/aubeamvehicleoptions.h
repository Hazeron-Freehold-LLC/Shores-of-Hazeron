//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubeamvehicleoptions.h,v 1.7 2022/02/19 17:50:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBeamVehicleOptions_H
#define AuBeamVehicleOptions_H

#include "auclientlib.h"
#include "audef.h"
#include "audialog.h"
#include "auglobell.h"
#include "autransportertarget.h"

class AuId;
class AuToolButtonValue;
class QComboBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

class AUCLIENT_DLL AuBeamVehicleOptions : public AuDialog
{
	Q_OBJECT;
private:
	QList<AuTransporterTarget>			m_Targets;

	// Widgets
	QPushButton*						m_BeamDown;
	QRadioButton*						m_BeamToCity;			// Town square of a city
	QRadioButton*						m_BeamToCoord;			// 3D coordinate
	QRadioButton*						m_BeamToPlanet;			// Longitude/latitude of current planet.
	//QRadioButton*						m_BeamToShip;			// Somewhere aboard ship.
	QRadioButton*						m_BeamToTransporter;	// Transporter room aboard ship.
	QComboBox*							m_Cities;
	QLineEdit*							m_Coord;
	QLineEdit*							m_LongLat;
	QComboBox*							m_Spacecraft;
	QComboBox*							m_Transporters;
	QMap<TRANSPORTER_TARGET, AuToolButtonValue*>	m_TTFilter;

public:
	AuBeamVehicleOptions(QWidget* parent=0, const AuGlobeLLF& ll=AuGlobeLLF(0,0));
	virtual ~AuBeamVehicleOptions();

	// Returns 0=city, 1=coord, 2=planet, 3=ship, 4=transporter, 5=none selected
	unsigned int BeamDestination() const;

	AuId CityId() const;

	QString CityName() const;

	float CoordX() const;

	float CoordY() const;

	float CoordZ() const;

	float Latitude() const;

	float Longitude() const;

	AuId ShipId() const;

	QString ShipName() const;

	TRANSPORTER_TARGET Target();

	AuId TransporterId() const;

	QString TransporterName() const;

	void TransporterPlace(const AuTransporterTarget& tt);

	void TransporterShip(const AuId& id, const QString& name, const QString& desc);

	void TransporterTransporter(const AuId& id, const QString& name, const QString& desc, quint16 whichTransporter);

	quint16 TransporterWhich() const;

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void BeamDown();
	void DestinationCity();
	void DestinationCoord();
	void DestinationPlanet();
	void DestinationShip();
	void DestinationTransporter();
	void PlanetLongLatChanged(const QString& longLat);
	void TransporterFilterClicked(int targetType);

protected:
	void ListTransporterPlace(const AuTransporterTarget& tt);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuBeamVehicleOptions_H
