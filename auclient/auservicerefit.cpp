//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservicerefit.cpp,v 1.10 2022/02/19 18:16:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auservicerefit.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "auframe.h"
#include "aulineedit.h"
#include "auservicedialog.h"
#include "auslider.h"
#include "auview.h"
#include "daperson.h"
#include "dasc.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTextBrowser>
#include <QVariant>

AuServiceRefit::AuServiceRefit(AuServiceDialog* parent) : QWidget(parent)
{
	// Get spaceship information.
	quint32 cargoVol = 0;
	quint32 cryoTrav = 0;
	quint8 qmade = 0;
	AuView* pView = auApp->Frame()->View();
	if(pView)
	{
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person)
		{
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
			{
				m_ShipId = ship->Id();
				DaSc* sc = dynamic_cast<DaSc*>(ship);
				if(sc)
				{
					m_DesignReport = sc->LockDesignAnalysis();
					sc->UnlockDesignAnalysis();
					cargoVol = sc->CargoTotalVolume();
					cryoTrav = sc->CryoBerthsOccupied();
					qmade = sc->QualityMade();
				}
			}
		}
		pView->UnlockSceneAtom();
	}

	// Refit is available.
	quint32 hullVol = 0;
	quint32 armorVol = 0;
	quint32 capVol = 0;
	quint32 cryoVol = 0;
	quint32 fuelVol = 0;
	quint32 holdVol = 0;
	quint32 lsVol = 0;
	quint32 drvVol = 0;
	quint32 ppVol = 0;
	quint32 sensVol = 0;
	quint32 shVol = 0;
	quint32 skVol = 0;	// skeleton/framework
	bool hasSensors = false;
	bool hasShields = false;
	QMap<quint16, AuScReportSysVol> ftlDrives, bays;
	m_DesignReport.HullVolumeDistribution(hullVol, armorVol, capVol, cryoVol, fuelVol, holdVol, lsVol, drvVol, ppVol, sensVol,
			hasSensors, shVol, hasShields, ftlDrives, bays, skVol);
	quint32 totftl = 0;
	for(QMap<quint16, AuScReportSysVol>::const_iterator pi = ftlDrives.cbegin(); pi != ftlDrives.cend(); pi++)
		totftl += (*pi).Volume();
	quint32 totbays = 0;
	for(QMap<quint16, AuScReportSysVol>::const_iterator pi = bays.cbegin(); pi != bays.cend(); pi++)
		totbays += (*pi).Volume();
	m_Info = new QLabel(tr("<html><p>Adjust the amount of hull volume allocated to each system. "
				"</p>"
				"<p>"
				"The spacecraft must be rigged for repairs: stopped, sensors off, shields off, power plant off, "
				"maneuver drive off and FTL drive off. "
				"</p>"
				"<p>"
				"Cargo aboard will impose a minimum on the <i>Hold</i> volume. "
				"Cryo travellers aboard will impose a minimum on the <i>Cryo Berths</i> volume. "
				"Excess fuel is jettisoned if it does not fit into the resulting <i>Fuel</i> volume. "
				"Excess power is lost if it cannot be stored in the resulting <i>Capacitor</i> volume. "
				"</p>"
				"<p>"
				"<i>Minimum quality</i> affects systems that <u>increase</u> in volume. "
				"The module quality is reduced to the minimum quality, on modules associated with those systems. "
				"Module quality is not affected on systems that decrease in volume. "
				"</p>"
				"</html>"), this);
	m_Info->setWordWrap(true);

	m_Analysis = new QTextBrowser(this);
	m_Analysis->setReadOnly(true);
	m_Analysis->setHtml(m_DesignReport.ReportAnalysis(AuDesign::Edit));	// Edit state for warnings.

	QLabel* lbHullVol = new QLabel(tr("<html><b>System Hull Volume</b></html>"), this);
	lbHullVol->setAlignment(Qt::AlignBottom|Qt::AlignLeft);

	QLabel* lbHullVolTot = new QLabel(tr("<html><b>%L1m%2 Total</b></html>").arg(hullVol).arg(g_Au.CubedSymbol()), this);
	lbHullVolTot->setAlignment(Qt::AlignBottom|Qt::AlignLeft);

	// Armor
	QLabel* lbArmorIcon;
	QLabel* lbArmor;
	{
		lbArmorIcon = new QLabel(this);
		lbArmorIcon->setPixmap(QPixmap(SystemIcon(SYS_Hull)).scaled(16,16));
		lbArmorIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_Hull)).arg(m_DesignReport.HullQ()));

		lbArmor = new QLabel(tr("Armor"), this);

		m_Armor = new QSlider(Qt::Horizontal, this);
		m_Armor->setRange(0, hullVol);						// Armor can be set to zero.
		m_Armor->setSingleStep(1);
		m_Armor->setPageStep(1);
		m_Armor->setValue(armorVol);
		m_Armor->setToolTip(tr("<b>Armor</b><br>"
					"Adjusts the volume of hull allocated to armor. "
					"<br>"
					"Hull armor improves defense against physical damage and increases mass."));
		connect(m_Armor, SIGNAL(valueChanged(int)), SLOT(SetArmor(int)));

		m_ArmorLb = new QLineEdit(QString("%L1m%2").arg(armorVol).arg(g_Au.CubedSymbol()), this);
		connect(m_ArmorLb, SIGNAL(editingFinished()), SLOT(ArmorEdited()));
	}

	QLabel* lbCapacitorIcon;
	QLabel* lbCapacitor;
	{
		lbCapacitorIcon = new QLabel(this);
		lbCapacitorIcon->setPixmap(QPixmap(SystemIcon(SYS_Capacitor)).scaled(16,16));
		lbCapacitorIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_Capacitor)).arg(m_DesignReport.CapacitorQ()));

		lbCapacitor = new QLabel(tr("Capacitor"), this);

		m_Capacitor = new QSlider(Qt::Horizontal, this);
		m_Capacitor->setRange(1, hullVol);					// Capacitor cannot be zero.
		m_Capacitor->setSingleStep(1);
		m_Capacitor->setPageStep(1);
		m_Capacitor->setValue(capVol);
		m_Capacitor->setToolTip(tr("<html>"
					"<b>Capacitor</b><br>"
					"Adjusts the volume of hull allocated to capacitor. "
					"<br>"
					"Capacitor stores energy for ready use by systems of the spacecraft. "
					"Energy is produced by the power plant. "
					"</html>"));
		connect(m_Capacitor, SIGNAL(valueChanged(int)), SLOT(SetCapacitor(int)));

		m_CapacitorLb = new QLineEdit(QString("%L1m%2").arg(capVol).arg(g_Au.CubedSymbol()), this);
		connect(m_CapacitorLb, SIGNAL(editingFinished()), SLOT(CapacitorEdited()));
	}

	// Cryo Berths
	QLabel* lbCryoIcon;
	QLabel* lbCryo;
	{
		lbCryoIcon = new QLabel(this);
		lbCryoIcon->setToolTip(tr("<html>"
				"No system module is associated with cryo berths. "
				"Increasing the volume of cryo berths does not reduce the quality of a system module. "
				"</html>"));

		int cryoUsed = cryoTrav * 4;						// 4 cubic meters per cryo berth
		lbCryo = new QLabel(tr("Cryo Berths"), this);

		m_Cryo = new QSlider(Qt::Horizontal, this);
		m_Cryo->setRange(cryoUsed, hullVol);				// Cryo can be set to zero.
		m_Cryo->setSingleStep(1);
		m_Cryo->setPageStep(1);
		m_Cryo->setValue(cryoVol);
		if(cryoUsed)
			m_Cryo->setToolTip(tr("<b>Cryo Berths</b><br>"
						"Adjusts the volume of hull allocated to cryo berths. "
						"Every 4 cubic meters of volume adds one cryo berth. "
						"Cryo berths do not require additional life support. "
						"<br>"
						"Cryo berths provide a means for carrying large numbers of travellers. "
						"Cryo travellers can be offloaded to cities or revived individually as crew, troops or passengers. "
						"Avatars cannot enter cryo berths."
						"<br>"
						"<b>%L1 cryo travellers prevent the volume of cryo berths from being reduced below %L2.</b>")
					.arg(cryoTrav)
					.arg(cryoUsed));
		else
			m_Cryo->setToolTip(tr("<b>Cryo Berths</b><br>"
						"Adjusts the volume of hull allocated to cryo berths. "
						"Each cubic meter of volume adds one cryo berth. "
						"Cryo berths do not require additional life support. "
						"<br>"
						"Cryo berths provide a means for carrying large numbers of travellers. "
						"Cryo travellers can be offloaded to cities or revived individually as crew, troops or passengers. "
						"Avatars cannot enter cryo berths."));
		connect(m_Cryo, SIGNAL(valueChanged(int)), SLOT(SetCryo(int)));

		m_CryoLb = new QLineEdit(QString("%L1m%2").arg(cryoVol).arg(g_Au.CubedSymbol()), this);
		connect(m_CryoLb, SIGNAL(editingFinished()), SLOT(CryoEdited()));
	}

	QLabel* lbFuelIcon;
	QLabel* lbFuel;
	{
		lbFuelIcon = new QLabel(this);
		lbFuelIcon->setPixmap(QPixmap(SystemIcon(SYS_Hull)).scaled(16,16));
		lbFuelIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_Hull)).arg(m_DesignReport.HullQ()));

		lbFuel = new QLabel(tr("Fuel"), this);

		m_Fuel = new QSlider(Qt::Horizontal, this);
		m_Fuel->setRange(1, hullVol);						// Fuel cannot be zero.
		m_Fuel->setSingleStep(1);
		m_Fuel->setPageStep(1);
		m_Fuel->setValue(fuelVol);
		m_Fuel->setToolTip(tr("<html>"
					"<b>Fuel Cells</b><br>"
					"Adjusts the volume of hull allocated to fuel cells. "
					"<br>"
					"Fuel cells store fuel. "
					"Fuel is converted to energy by the power plant. "
					"Fuel is burned for propulsion in rocket driven spacecraft. "
					"</html>"));
		connect(m_Fuel, SIGNAL(valueChanged(int)), SLOT(SetFuel(int)));

		m_FuelLb = new QLineEdit(QString("%L1m%2").arg(fuelVol).arg(g_Au.CubedSymbol()), this);
		connect(m_FuelLb, SIGNAL(editingFinished()), SLOT(FuelEdited()));
	}

	QLabel* lbHoldIcon;
	QLabel* lbHold;
	{
		lbHoldIcon = new QLabel(this);
		lbHoldIcon->setPixmap(QPixmap(SystemIcon(SYS_Hull)).scaled(16,16));
		lbHoldIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_Hull)).arg(m_DesignReport.HullQ()));

		lbHold = new QLabel(tr("Hold"), this);

		m_Hold = new QSlider(Qt::Horizontal, this);
		m_Hold->setRange(cargoVol, hullVol);						// Hold can be set to zero.
		m_Hold->setSingleStep(1);
		m_Hold->setPageStep(1);
		m_Hold->setValue(holdVol);
		if(cargoVol)
			m_Hold->setToolTip(tr("<html>"
						"<b>Hold</b><br>"
						"Adjusts the volume of hull allocated to hold. "
						"<br>"
						"Hold stores cargo. "
						"<br>"
						"<b>Cargo prevents hold volume from being reduced below %L1.</b>"
						"</html>")
					.arg(cargoVol));
		else
			m_Hold->setToolTip(tr("<html>"
						"<b>Hold</b><br>"
						"Adjusts the volume of hull allocated to hold. "
						"<br>"
						"Hold stores cargo. "
						"</html>"));
		connect(m_Hold, SIGNAL(valueChanged(int)), SLOT(SetHold(int)));

		m_HoldLb = new QLineEdit(QString("%L1m%2").arg(holdVol).arg(g_Au.CubedSymbol()), this);
		connect(m_HoldLb, SIGNAL(editingFinished()), SLOT(HoldEdited()));
	}

	QLabel* lbLifeSupportIcon;
	QLabel* lbLifeSupport;
	{
		lbLifeSupportIcon = new QLabel(this);
		lbLifeSupportIcon->setPixmap(QPixmap(SystemIcon(SYS_LifeSupport)).scaled(16,16));
		lbLifeSupportIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_LifeSupport)).arg(m_DesignReport.LifeSupportQ()));

		lbLifeSupport = new QLabel(tr("Life Support"), this);

		m_LifeSupport = new QSlider(Qt::Horizontal, this);
		m_LifeSupport->setRange(1, hullVol);				// Life support cannot be zero.
		m_LifeSupport->setSingleStep(1);
		m_LifeSupport->setPageStep(1);
		m_LifeSupport->setValue(lsVol);
		m_LifeSupport->setToolTip(tr("<html>"
					"<b>Life Support</b><br>"
					"Adjusts the volume of hull allocated to life support. "
					"<br>"
					"Life support maintains a habitable environment inside the spacecraft. "
					"</html>"));
		connect(m_LifeSupport, SIGNAL(valueChanged(int)), SLOT(SetLifeSupport(int)));

		m_LifeSupportLb = new QLineEdit(QString("%L1m%2").arg(lsVol).arg(g_Au.CubedSymbol()), this);
		connect(m_LifeSupportLb, SIGNAL(editingFinished()), SLOT(LifeSupportEdited()));
	}

	QLabel* lbManeuverDriveIcon;
	QLabel* lbManeuverDrive;
	{
		lbManeuverDriveIcon = new QLabel(this);
		lbManeuverDriveIcon->setPixmap(QPixmap(SystemIcon(SYS_ManeuverDrive)).scaled(16,16));
		lbManeuverDriveIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_ManeuverDrive)).arg(m_DesignReport.ManeuverDriveQ()));

		lbManeuverDrive = new QLabel(tr("Maneuver Drive"), this);

		m_ManeuverDrive = new QSlider(Qt::Horizontal, this);
		m_ManeuverDrive->setRange(1, hullVol);				// Maneuver drive cannot be zero.
		m_ManeuverDrive->setSingleStep(1);
		m_ManeuverDrive->setPageStep(1);
		m_ManeuverDrive->setValue(drvVol);
		m_ManeuverDrive->setToolTip(tr("<html>"
					"<b>Maneuver Drive</b><br>"
					"Adjusts the volume of hull allocated to maneuver drives. "
					"<br>"
					"Maneuver drives move the spacecraft through space. "
					"Maneuver drives consume power from the capacitor, except rocket drives which burn fuel directly. "
					"</html>"));
		connect(m_ManeuverDrive, SIGNAL(valueChanged(int)), SLOT(SetManeuverDrive(int)));

		m_ManeuverDriveLb = new QLineEdit(QString("%L1m%2").arg(drvVol).arg(g_Au.CubedSymbol()), this);
		connect(m_ManeuverDriveLb, SIGNAL(editingFinished()), SLOT(ManeuverDriveEdited()));
	}

	QLabel* lbPowerPlantIcon;
	QLabel* lbPowerPlant;
	{
		lbPowerPlantIcon = new QLabel(this);
		lbPowerPlantIcon->setPixmap(QPixmap(SystemIcon(SYS_PowerPlant)).scaled(16,16));
		lbPowerPlantIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_PowerPlant)).arg(m_DesignReport.PowerPlantQ()));

		lbPowerPlant = new QLabel(tr("Power Plant"), this);

		m_PowerPlant = new QSlider(Qt::Horizontal, this);
		m_PowerPlant->setRange(1, hullVol);				// Power plant cannot be zero.
		m_PowerPlant->setSingleStep(1);
		m_PowerPlant->setPageStep(1);
		m_PowerPlant->setValue(ppVol);
		m_PowerPlant->setToolTip(tr("<html>"
					"<b>Power Plant</b><br>"
					"Adjusts the volume of hull allocated to power plant. "
					"<br>"
					"Power plant converts fuel into energy and stores it in the capacitor. "
					"</html>"));
		connect(m_PowerPlant, SIGNAL(valueChanged(int)), SLOT(SetPowerPlant(int)));

		m_PowerPlantLb = new QLineEdit(QString("%L1m%2").arg(ppVol).arg(g_Au.CubedSymbol()), this);
		connect(m_PowerPlantLb, SIGNAL(editingFinished()), SLOT(PowerPlantEdited()));
	}

	QLabel* lbSensorsIcon = 0;
	QLabel* lbSensors = 0;
	if(hasSensors)
	{
		lbSensorsIcon = new QLabel(this);
		lbSensorsIcon->setPixmap(QPixmap(SystemIcon(SYS_Sensor)).scaled(16,16));
		lbSensorsIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_Sensor)).arg(m_DesignReport.SensorQ()));

		lbSensors = new QLabel(tr("Sensors"), this);

		m_Sensors = new QSlider(Qt::Horizontal, this);
		m_Sensors->setRange(1, hullVol);					// Sensors cannot be zero now that the system exists.
		m_Sensors->setSingleStep(1);
		m_Sensors->setPageStep(1);
		m_Sensors->setValue(sensVol);
		m_Sensors->setToolTip(tr("<html>"
					"<b>Sensors</b><br>"
					"Adjusts the volume of hull allocated to sensors. "
					"<br>"
					"Sensors detect worlds, spacecraft, cities, vehicles and other objects in the environment of the "
					"spacecraft. "
					"</html>"));
		connect(m_Sensors, SIGNAL(valueChanged(int)), SLOT(SetSensors(int)));

		m_SensorsLb = new QLineEdit(QString("%L1m%2").arg(sensVol).arg(g_Au.CubedSymbol()), this);
		connect(m_SensorsLb, SIGNAL(editingFinished()), SLOT(SensorsEdited()));
	}
	else
	{
		m_Sensors = 0;
		m_SensorsLb = 0;
	}

	QLabel* lbShieldsIcon = 0;
	QLabel* lbShields = 0;
	if(hasShields)
	{
		lbShieldsIcon = new QLabel(this);
		lbShieldsIcon->setPixmap(QPixmap(SystemIcon(SYS_Shield)).scaled(16,16));
		lbShieldsIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_Shield)).arg(m_DesignReport.ShieldQ()));

		lbShields = new QLabel(tr("Shields"), this);

		m_Shields = new QSlider(Qt::Horizontal, this);
		m_Shields->setRange(1, hullVol);					// Shields cannot be zero now that the system exists.
		m_Shields->setSingleStep(1);
		m_Shields->setPageStep(1);
		m_Shields->setValue(shVol);
		m_Shields->setToolTip(tr("<html>"
					"<b>Shields</b><br>"
					"Adjusts the volume of hull allocated to shields. "
					"<br>"
					"Shields create a protective field around the spacecraft. "
					"</html>"));
		connect(m_Shields, SIGNAL(valueChanged(int)), SLOT(SetShields(int)));

		m_ShieldsLb = new QLineEdit(QString("%L1m%2").arg(shVol).arg(g_Au.CubedSymbol()), this);
		connect(m_ShieldsLb, SIGNAL(editingFinished()), SLOT(ShieldsEdited()));
	}
	else
	{
		m_Shields = 0;
		m_ShieldsLb = 0;
	}

	QHash<quint16, QLabel*> lbFTLDriveIcons;
	QHash<quint16, QLabel*> lbFTLDrives;
	{
		for(QMap<quint16, AuScReportSysVol>::const_iterator pi = ftlDrives.cbegin(); pi != ftlDrives.cend(); pi++)
		{
			QLabel* lbIcon = new QLabel(this);
			lbIcon->setPixmap(QPixmap(SystemIcon(SYS_FTLDrive)).scaled(16,16));
			lbIcon->setToolTip(tr("<html>"
					"<b>%1 Module Q%2</b><br>"
					"Increasing the volume of this component will reduce the %1 module quality, "
					"if its current Q%2 is higher than the minimum quality. "
					"</html>").arg(ScSystemName(SYS_FTLDrive)).arg(m_DesignReport.FTLDriveQuality(pi.key())));

			QString n = QString("%1. %2").arg(pi.key()).arg(g_Au.CommodityName((*pi).Module()));
			QLabel* lb = new QLabel(n, this);

			AuSliderValue* sv = new AuSliderValue(Qt::Horizontal, pi.key(), this);
			sv->setRange(1, hullVol);						// FTL drive cannot be zero now that the system exists.
			sv->setSingleStep(1);
			sv->setPageStep(1);
			sv->setValue((*pi).Volume());
			sv->setToolTip(tr("<html>"
						"<b>FTL Drive</b><br>"
						"Adjusts the volume of hull allocated to this FTL drive. "
						"<br>"
						"FTL drives enable a spacecraft to travel Faster Than Light. "
						"</html>"));
			connect(sv, SIGNAL(ValueChanged(int, int)), SLOT(SetFTLDrive(int, int)));

			AuLineEditValue* svl =
				new AuLineEditValue(QString("%L1m%2").arg((*pi).Volume()).arg(g_Au.CubedSymbol()), pi.key(), this);
			connect(svl, SIGNAL(EditingFinished(int)), SLOT(FTLDriveEdited(int)));

			lbFTLDriveIcons.insert(pi.key(), lbIcon);
			lbFTLDrives.insert(pi.key(), lb);
			m_FTLDrives.insert(pi.key(), sv);
			m_FTLDriveLbs.insert(pi.key(), svl);
		}
	}

	QHash<quint16, QLabel*> lbBayIcons;
	QHash<quint16, QLabel*> lbBays;
	{
		for(QMap<quint16, AuScReportSysVol>::const_iterator pi = bays.cbegin(); pi != bays.cend(); pi++)
		{
			QLabel* lbIcon = new QLabel(this);
			lbIcon->setPixmap(QPixmap(SystemIcon(SYS_Weapon)).scaled(16,16));
			lbIcon->setToolTip(tr("<html>"
					"<b>%1 Module Q%2</b><br>"
					"Increasing the volume of this component will reduce the %1 module quality, "
					"if its current Q%2 is higher than the minimum quality. "
					"</html>").arg(ScSystemName(SYS_Weapon)).arg(m_DesignReport.WeaponBayQuality(pi.key())));

			QString n = QString("%1. %2").arg(pi.key()).arg(g_Au.CommodityName((*pi).Module()));
			QLabel* lb = new QLabel(n, this);

			AuSliderValue* sv = new AuSliderValue(Qt::Horizontal, pi.key(), this);
			sv->setRange(1, hullVol);						// Bay cannot be zero now that the system exists.
			sv->setSingleStep(1);
			sv->setPageStep(1);
			sv->setValue((*pi).Volume());
			sv->setToolTip(tr("<html>"
						"<b>Bay</b><br>"
						"Adjusts the volume of hull allocated to this bay. "
						"<br>"
						"Bays provide equipment for large scale weapons, resource harvesting equipment, tractor beams, etc. "
						"Bay equipment is directionless and it is not blocked by the hull or other parts of the spacecraft. "
						"</html>"));
			connect(sv, SIGNAL(ValueChanged(int, int)), SLOT(SetBay(int, int)));

			AuLineEditValue* svl =
				new AuLineEditValue(QString("%L1m%2").arg((*pi).Volume()).arg(g_Au.CubedSymbol()), pi.key(), this);
			connect(svl, SIGNAL(EditingFinished(int)), SLOT(BayEdited(int)));

			lbBayIcons.insert(pi.key(), lbIcon);
			lbBays.insert(pi.key(), lb);
			m_Bays.insert(pi.key(), sv);
			m_BayLbs.insert(pi.key(), svl);
		}
	}

	QLabel* lbSkeletonIcon;
	QLabel* lbSkeleton;
	{
		lbSkeletonIcon = new QLabel(this);
		lbSkeletonIcon->setPixmap(QPixmap(SystemIcon(SYS_Hull)).scaled(16,16));
		lbSkeletonIcon->setToolTip(tr("<html>"
				"<b>%1 Module Q%2</b><br>"
				"Increasing the volume of this component will reduce the %1 module quality, "
				"if its current Q%2 is higher than the minimum quality. "
				"</html>").arg(ScSystemName(SYS_Hull)).arg(m_DesignReport.HullQ()));

		lbSkeleton = new QLabel(tr("Framework"), this);

		m_Skeleton = new QSlider(Qt::Horizontal, this);
		m_Skeleton->setRange(0, hullVol);						// Skeleton can be set to zero.
		m_Skeleton->setSingleStep(1);
		m_Skeleton->setPageStep(1);
		m_Skeleton->setValue(skVol);
		m_Skeleton->setToolTip(tr("<html>"
					"<b>Framework</b><br>"
					"Adjusts the volume of hull allocated to framework. "
					"Framework is walls, floors, ceilings, beams, columns and other support structures. "
					"<br>"
					"Framework volume adds 4x the structural hit points of other volumes. "
					"</html>"));
		connect(m_Skeleton, SIGNAL(valueChanged(int)), SLOT(SetSkeleton(int)));

		m_SkeletonLb = new QLineEdit(QString("%L1m%2").arg(skVol).arg(g_Au.CubedSymbol()), this);
		connect(m_SkeletonLb, SIGNAL(editingFinished()), SLOT(SkeletonEdited()));
	}

	QLabel* lbMinQ = new QLabel(tr("Minimum Quality of Parts"), this);

	m_MinQ = new QSpinBox(this);
	m_MinQ->setRange(1,255);
	m_MinQ->setValue(qmade);
	m_MinQ->setToolTip(tr("<html>"
				"Minimum quality of parts to be installed. "
				"Initially, this value is the quality of manufacture of the spacecraft. "
				"<br>"
				"<b>The system module quality is reduced to this minimum, in every system that <u>increases</u> in size.</b>"
				"<br>"
				"A tool tip on each volume's icon reports the affected system module and its current quality. "
				"</html>"));

	QPushButton* pbreset = new QPushButton(tr("Reset System Volumes..."));
	pbreset->setAutoDefault(true);
	pbreset->setDefault(true);
	pbreset->setToolTip(tr("<html>"
				"<p>"
				"Resets the volume sliders and analysis to match the spacecraft you are currently aboard. "
				"Changes could be lost so a confirmation dialog is presented. "
				"</p>"
				"</html>"));
	connect(pbreset, SIGNAL(clicked()), SLOT(ResetVolumes()));

	m_RefitButton = new QPushButton(tr("Request Refit Estimate"));
	m_RefitButton->setAutoDefault(true);
	m_RefitButton->setDefault(true);
	m_RefitButton->setToolTip(tr("<html>"
				"<p>"
				"Request an estimate to refit the spacecraft. "
				"Cities will respond with an estimate, if they can perform the refit. "
				"You can choose to accept or reject the estimate. "
				"</p>"
				"<p>"
				"<b>Refit is not available on old style spacecraft designs.</b> "
				"</p>"
				"</html>"));
	connect(m_RefitButton, SIGNAL(clicked()), SLOT(RequestRefit()));

	QGridLayout* slay = new QGridLayout;
	int row=0;
	slay->addWidget(m_Info, row,0, 1,4);
	row++;
	slay->setRowStretch(row, 1);
	row++;
	slay->addWidget(lbHullVol, row,0, 1,2);
	slay->addWidget(lbHullVolTot, row,3);
	row++;
	slay->addWidget(lbSkeletonIcon, row,0);
	slay->addWidget(lbSkeleton, row,1);
	slay->addWidget(m_Skeleton, row,2);
	slay->addWidget(m_SkeletonLb, row,3);
	row++;
	slay->addWidget(lbArmorIcon, row,0);
	slay->addWidget(lbArmor, row,1);
	slay->addWidget(m_Armor, row,2);
	slay->addWidget(m_ArmorLb, row,3);
	row++;
	slay->addWidget(lbCapacitorIcon, row,0);
	slay->addWidget(lbCapacitor, row,1);
	slay->addWidget(m_Capacitor, row,2);
	slay->addWidget(m_CapacitorLb, row,3);
	row++;
	slay->addWidget(lbCryoIcon, row,0);
	slay->addWidget(lbCryo, row,1);
	slay->addWidget(m_Cryo, row,2);
	slay->addWidget(m_CryoLb, row,3);
	row++;
	slay->addWidget(lbFuelIcon, row,0);
	slay->addWidget(lbFuel, row,1);
	slay->addWidget(m_Fuel, row,2);
	slay->addWidget(m_FuelLb, row,3);
	row++;
	slay->addWidget(lbHoldIcon, row,0);
	slay->addWidget(lbHold, row,1);
	slay->addWidget(m_Hold, row,2);
	slay->addWidget(m_HoldLb, row,3);
	row++;
	slay->addWidget(lbLifeSupportIcon, row,0);
	slay->addWidget(lbLifeSupport, row,1);
	slay->addWidget(m_LifeSupport, row,2);
	slay->addWidget(m_LifeSupportLb, row,3);
	row++;
	slay->addWidget(lbManeuverDriveIcon, row,0);
	slay->addWidget(lbManeuverDrive, row,1);
	slay->addWidget(m_ManeuverDrive, row,2);
	slay->addWidget(m_ManeuverDriveLb, row,3);
	row++;
	slay->addWidget(lbPowerPlantIcon, row,0);
	slay->addWidget(lbPowerPlant, row,1);
	slay->addWidget(m_PowerPlant, row,2);
	slay->addWidget(m_PowerPlantLb, row,3);
	row++;
	if(lbSensors)
	{
		slay->addWidget(lbSensorsIcon, row,0);
		slay->addWidget(lbSensors, row,1);
		slay->addWidget(m_Sensors, row,2);
		slay->addWidget(m_SensorsLb, row,3);
		row++;
	}
	if(lbShields)
	{
		slay->addWidget(lbShieldsIcon, row,0);
		slay->addWidget(lbShields, row,1);
		slay->addWidget(m_Shields, row,2);
		slay->addWidget(m_ShieldsLb, row,3);
		row++;
	}
	if(!lbFTLDrives.empty())
	{
		QHash<quint16, QLabel*>::const_iterator ii = lbFTLDriveIcons.cbegin();
		QHash<quint16, QLabel*>::const_iterator ni = lbFTLDrives.cbegin();
		QHash<quint16, AuSliderValue*>::const_iterator si = m_FTLDrives.cbegin();
		QHash<quint16, AuLineEditValue*>::const_iterator li = m_FTLDriveLbs.cbegin();
		while(ii != lbFTLDriveIcons.cend() && ni != lbFTLDrives.cend() && si != m_FTLDrives.cend() &&
				li != m_FTLDriveLbs.cend())
		{
			slay->addWidget(*ii, row,0);
			slay->addWidget(*ni, row,1);
			slay->addWidget(*si, row,2);
			slay->addWidget(*li, row,3);
			row++;

			ii++;
			ni++;
			si++;
			li++;
		}
	}
	if(!lbBays.empty())
	{
		QHash<quint16, QLabel*>::const_iterator ii = lbBayIcons.cbegin();
		QHash<quint16, QLabel*>::const_iterator ni = lbBays.cbegin();
		QHash<quint16, AuSliderValue*>::const_iterator si = m_Bays.cbegin();
		QHash<quint16, AuLineEditValue*>::const_iterator li = m_BayLbs.cbegin();
		while(ii != lbBayIcons.cend() && ni != lbBays.cend() && si != m_Bays.cend() && li != m_BayLbs.cend())
		{
			slay->addWidget(*ii, row,0);
			slay->addWidget(*ni, row,1);
			slay->addWidget(*si, row,2);
			slay->addWidget(*li, row,3);
			row++;

			ii++;
			ni++;
			si++;
			li++;
		}
	}
	slay->setColumnStretch(2,1);
	slay->setSpacing(2);
	slay->setMargin(0);

	QWidget* controls = new QWidget(this);
	controls->setLayout(slay);

	m_Splitter = new QSplitter;
	m_Splitter->addWidget(controls);
	m_Splitter->addWidget(m_Analysis);
	m_Splitter->setChildrenCollapsible(false);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addWidget(pbreset);
	blay->addStretch();
	blay->addWidget(lbMinQ);
	blay->addWidget(m_MinQ);
	blay->addWidget(m_RefitButton);

	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(m_Splitter);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/ServiceRefit/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

void AuServiceRefit::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ServiceRefit/Splitter", m_Splitter->saveState());
}

void AuServiceRefit::RequestRefit()
{
	AuServiceDialog* svcdlg = dynamic_cast<AuServiceDialog*>(parent());
	if(!svcdlg)
		return;
	AuCommChannel* channel = svcdlg->Channel();
	if(!channel)
		return;
	const AuView* view = auApp->Frame()->View();
	if(!view)
		return;
	if(m_ShipId.IsNull())
		return;

	// Build refit volumes from our controls.
	AuRefitShip refit;
	refit.m_Armor = m_Armor->value();
	for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		refit.m_Bays.insert(bi.key(), (*bi)->value());
	refit.m_Capacitor = m_Capacitor->value();
	refit.m_CryoBerths = m_Cryo->value();
	for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_FTLDrives.cbegin(); bi != m_FTLDrives.cend(); bi++)
		refit.m_FTLDrives.insert(bi.key(), (*bi)->value());
	refit.m_Fuel = m_Fuel->value();
	refit.m_Hold = m_Hold->value();
	refit.m_LifeSupport = m_LifeSupport->value();
	refit.m_ManeuverDrive = m_ManeuverDrive->value();
	refit.m_MinQ = m_MinQ->value();
	refit.m_PowerPlant = m_PowerPlant->value();
	if(m_Sensors)
		refit.m_Sensor = m_Sensors->value();
	if(m_Shields)
		refit.m_Shield = m_Shields->value();;
	refit.m_Skeleton = m_Skeleton->value();
	if(refit.Volume() != m_DesignReport.HullVolume())
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("The spacecraft cannot be refit using those specifications. "
					"The sum of the slide controls does not equal the total hull volume. "),
				QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	// Insure the result could be manufactured.
	bool isOk = false;
	AuView* pView = auApp->Frame()->View();
	if(pView)
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person)
		{
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship && ship->Is(m_ShipId))
			{
				DaSc* sc = dynamic_cast<DaSc*>(ship);
				if(sc)
				{
					const DgData& sdd = g_Au.LockScDesignsForRead();
					AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find(sc->DesignId());
					if(di != sdd.m_ScDesigns->cend())
					{
						AuScReport refitDesign(m_DesignReport);
						refitDesign.RefitArmorVolume(refit.m_Armor, refit.m_MinQ);
						refitDesign.RefitBayVolumes(refit.m_Bays, refit.m_MinQ);
						refitDesign.RefitCapacitorVolume(refit.m_Capacitor, refit.m_MinQ);
						refitDesign.RefitCryoBerthVolume(refit.m_CryoBerths, refit.m_MinQ);
						refitDesign.RefitFTLDriveVolumes(refit.m_FTLDrives, refit.m_MinQ);
						refitDesign.RefitFuelVolume(refit.m_Fuel, refit.m_MinQ);
						refitDesign.RefitHoldVolume(refit.m_Hold, refit.m_MinQ);
						refitDesign.RefitLifeSupportVolume(refit.m_LifeSupport, refit.m_MinQ);
						refitDesign.RefitManeuverDriveVolume(refit.m_ManeuverDrive, refit.m_MinQ);
						refitDesign.RefitPowerPlantVolume(refit.m_PowerPlant, refit.m_MinQ);
						if(refitDesign.HasSensor())
							refitDesign.RefitSensorVolume(refit.m_Sensor, refit.m_MinQ);
						if(refitDesign.HasShield())
							refitDesign.RefitShieldVolume(refit.m_Shield, refit.m_MinQ);
						refitDesign.RefitSkeletonVolume(refit.m_Skeleton, refit.m_MinQ);
						refitDesign.UpdateCalculatedValues();

						QStringList msgs;
						isOk = refitDesign.OkToManufacture(msgs, &(*di).second);
					}
					g_Au.UnlockScDesigns();
				}
			}
		}
		pView->UnlockSceneAtom();
		QApplication::restoreOverrideCursor();
	}
	if(!isOk)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("The spacecraft cannot be refit using those specifications. "
					"The resulting design configuration is not valid for manufacture. "
					"The design analysis report should show the reason in red text. "),
			QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	AuCommRequestRefit msg(m_ShipId, refit, channel->objectName(), view->SceneAtomId(), view->SceneAtomName(),
			view->SceneSector(), view->SceneGalaxy(), channel->Range(), channel->Scope());
	channel->SendChat(msg);

	// Close the parent dialog.
	svcdlg->accept();
}

quint32 AuServiceRefit::SumVolumeSliders() const
{
	quint32 cursum = m_Armor->value();
	for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		cursum += (*bi)->value();
	cursum += m_Capacitor->value();
	cursum += m_Cryo->value();
	for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		cursum += (*fi)->value();
	cursum += m_Fuel->value();
	cursum += m_Hold->value();
	cursum += m_LifeSupport->value();
	cursum += m_ManeuverDrive->value();
	cursum += m_PowerPlant->value();
	if(m_Sensors)
		cursum += m_Sensors->value();
	if(m_Shields)
		cursum += m_Shields->value();
	cursum += m_Skeleton->value();
	return cursum;
}

void AuServiceRefit::LimitVolumeSliders(QSlider* control)
{
	// Get the sum of the volume sliders.
	quint32 cursum = SumVolumeSliders();
	if(cursum > m_DesignReport.HullVolume())
	{
		// We must make adjustments so the sum exactly matches the hull volume.
		// Block widget signals so we don't die a recursive death here.
		m_Armor->blockSignals(true);
		for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
			(*bi)->blockSignals(true);
		m_Capacitor->blockSignals(true);
		m_Cryo->blockSignals(true);
		for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
			(*fi)->blockSignals(true);
		m_Fuel->blockSignals(true);
		m_Hold->blockSignals(true);
		m_LifeSupport->blockSignals(true);
		m_ManeuverDrive->blockSignals(true);
		m_PowerPlant->blockSignals(true);
		if(m_Sensors)
			m_Sensors->blockSignals(true);
		if(m_Shields)
			m_Shields->blockSignals(true);
		m_Skeleton->blockSignals(true);

		// The amount to remove determines our strategy for balancing the other sliders.
		int remove = cursum - m_DesignReport.HullVolume();
		//qDebug() << "remove" << remove << "cursum" << cursum << "hullVol" << m_DesignReport.HullVolume();
		if(m_Skeleton != control && m_Skeleton->value() - remove >= m_Skeleton->minimum())
		{
			// We can take it all from Skeleton.
			//qDebug() << "remove from Skeleton" << remove;
			m_Skeleton->setValue(m_Skeleton->value() - remove);
		}
		else if(remove > 250)
		{
			// Removing enough to take a ratio from each control. The else leg is way slow when adjusting millions.
			// Get the total removable cubic meters so we can calculate each slider's share of the amount to be removed.
			int totalRemovable = 0;
			if(m_Armor != control)
				totalRemovable += m_Armor->value() - m_Armor->minimum();
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
				if(*bi != control)
					totalRemovable += (*bi)->value() - (*bi)->minimum();
			if(m_Capacitor != control)
				totalRemovable += m_Capacitor->value() - m_Capacitor->minimum();
			if(m_Cryo != control)
				totalRemovable += m_Cryo->value() - m_Cryo->minimum();
			for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
				if(*fi != control)
					totalRemovable += (*fi)->value() - (*fi)->minimum();
			if(m_Fuel != control)
				totalRemovable += m_Fuel->value() - m_Fuel->minimum();
			if(m_Hold != control)
				totalRemovable += m_Hold->value() - m_Hold->minimum();
			if(m_LifeSupport != control)
				totalRemovable += m_LifeSupport->value() - m_LifeSupport->minimum();
			if(m_ManeuverDrive != control)
				totalRemovable += m_ManeuverDrive->value() - m_ManeuverDrive->minimum();
			if(m_PowerPlant != control)
				totalRemovable += m_PowerPlant->value() - m_PowerPlant->minimum();
			if(m_Sensors && m_Sensors != control)
				totalRemovable += m_Sensors->value() - m_Sensors->minimum();
			if(m_Shields && m_Shields != control)
				totalRemovable += m_Shields->value() - m_Shields->minimum();
			if(m_Skeleton != control)
				totalRemovable += m_Skeleton->value() - m_Skeleton->minimum();

			// Remove each control's share of the total that can be removed.
			int removed = 0;
			if(removed < remove && m_Armor != control && m_Armor->value() > m_Armor->minimum())
			{
				int v = m_Armor->value() - m_Armor->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Armor->setValue(v - r + m_Armor->minimum());
				removed += r;
			}
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); removed < remove && bi != m_Bays.cend(); bi++)
			{
				if(*bi == control)
					continue;

				int v = (*bi)->value() - (*bi)->minimum();
				if(!v)
					continue;

				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				(*bi)->setValue(v - r + (*bi)->minimum());
				removed += r;
			}
			if(removed < remove && m_Capacitor != control && m_Capacitor->value() > m_Capacitor->minimum())
			{
				int v = m_Capacitor->value() - m_Capacitor->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Capacitor->setValue(v - r + m_Capacitor->minimum());
				removed += r;
			}
			if(removed < remove && m_Cryo != control && m_Cryo->value() > m_Cryo->minimum())
			{
				int v = m_Cryo->value() - m_Cryo->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Cryo->setValue(v - r + m_Cryo->minimum());
				removed += r;
			}
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_FTLDrives.cbegin();
					removed < remove && bi != m_FTLDrives.cend(); bi++)
			{
				if(*bi == control)
					continue;

				int v = (*bi)->value() - (*bi)->minimum();
				if(!v)
					continue;

				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				(*bi)->setValue(v - r + (*bi)->minimum());
				removed += r;
			}
			if(removed < remove && m_Fuel != control && m_Fuel->value() > m_Fuel->minimum())
			{
				int v = m_Fuel->value() - m_Fuel->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Fuel->setValue(v - r + m_Fuel->minimum());
				removed += r;
			}
			if(removed < remove && m_Hold != control && m_Hold->value() > m_Hold->minimum())
			{
				int v = m_Hold->value() - m_Hold->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Hold->setValue(v - r + m_Hold->minimum());
				removed += r;
			}
			if(removed < remove && m_Skeleton != control && m_Skeleton->value() > m_Skeleton->minimum())
			{
				int v = m_Skeleton->value() - m_Skeleton->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Skeleton->setValue(v - r + m_Skeleton->minimum());
				removed += r;
			}
			if(removed < remove && m_LifeSupport != control && m_LifeSupport->value() > m_LifeSupport->minimum())
			{
				int v = m_LifeSupport->value() - m_LifeSupport->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_LifeSupport->setValue(v - r + m_LifeSupport->minimum());
				removed += r;
			}
			if(removed < remove && m_ManeuverDrive != control && m_ManeuverDrive->value() > m_ManeuverDrive->minimum())
			{
				int v = m_ManeuverDrive->value() - m_ManeuverDrive->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_ManeuverDrive->setValue(v - r + m_ManeuverDrive->minimum());
				removed += r;
			}
			if(removed < remove && m_PowerPlant != control && m_PowerPlant->value() > m_PowerPlant->minimum())
			{
				int v = m_PowerPlant->value() - m_PowerPlant->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_PowerPlant->setValue(v - r + m_PowerPlant->minimum());
				removed += r;
			}
			if(removed < remove && m_Sensors && m_Sensors != control && m_Sensors->value() > m_Sensors->minimum())
			{
				int v = m_Sensors->value() - m_Sensors->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Sensors->setValue(v - r + m_Sensors->minimum());
				removed += r;
			}
			if(removed < remove && m_Shields && m_Shields != control && m_Shields->value() > m_Shields->minimum())
			{
				int v = m_Shields->value() - m_Shields->minimum();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Shields->setValue(v - r + m_Shields->minimum());
				removed += r;
			}
		}
		else
		{
			// Removing a small amount.
			//qDebug() << "remove a small amount" << remove;
			// Adjust sliders by taking one off the highest slider that is not the control slider, until we are in limits.
			while(cursum > m_DesignReport.HullVolume())
			{
				// Take one off the highest slider, except the 'control' slider.
				QSlider* highest = 0;
				qint32 highestDiff = 0;

				if(m_Armor != control && (!highest || m_Armor->value() - m_Armor->minimum() > highestDiff))
				{
					highest = m_Armor;
					highestDiff = m_Armor->value() - m_Armor->minimum();
				}

				for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
					if(*bi != control && (!highest || (*bi)->value() - (*bi)->minimum() > highestDiff))
					{
						highest = *bi;
						highestDiff = (*bi)->value() - (*bi)->minimum();
					}

				if(m_Capacitor != control && (!highest || m_Capacitor->value() - m_Capacitor->minimum() > highestDiff))
				{
					highest = m_Capacitor;
					highestDiff = m_Capacitor->value() - m_Capacitor->minimum();
				}

				if(m_Cryo != control && (!highest || m_Cryo->value() - m_Cryo->minimum() > highestDiff))
				{
					highest = m_Cryo;
					highestDiff = m_Cryo->value() - m_Cryo->minimum();
				}

				for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
					if(*fi != control && (!highest || (*fi)->value() - (*fi)->minimum() > highestDiff))
					{
						highest = *fi;
						highestDiff = (*fi)->value() - (*fi)->minimum();
					}

				if(m_Fuel != control && (!highest || m_Fuel->value() - m_Fuel->minimum() > highestDiff))
				{
					highest = m_Fuel;
					highestDiff = m_Fuel->value() - m_Fuel->minimum();
				}

				if(m_Hold != control && (!highest || m_Hold->value() - m_Hold->minimum() > highestDiff))
				{
					highest = m_Hold;
					highestDiff = m_Hold->value() - m_Hold->minimum();
				}

				if(m_Skeleton != control && (!highest || m_Skeleton->value() - m_Skeleton->minimum() > highestDiff))
				{
					highest = m_Skeleton;
					highestDiff = m_Skeleton->value() - m_Skeleton->minimum();
				}

				if(m_LifeSupport != control && (!highest || m_LifeSupport->value() - m_LifeSupport->minimum() > highestDiff))
				{
					highest = m_LifeSupport;
					highestDiff = m_LifeSupport->value() - m_LifeSupport->minimum();
				}

				if(m_ManeuverDrive != control &&
						(!highest || m_ManeuverDrive->value() - m_ManeuverDrive->minimum() > highestDiff))
				{
					highest = m_ManeuverDrive;
					highestDiff = m_ManeuverDrive->value() - m_ManeuverDrive->minimum();
				}

				if(m_PowerPlant != control && (!highest || m_PowerPlant->value() - m_PowerPlant->minimum() > highestDiff))
				{
					highest = m_PowerPlant;
					highestDiff = m_PowerPlant->value() - m_PowerPlant->minimum();
				}

				if(m_Sensors && m_Sensors != control && (!highest || m_Sensors->value() - m_Sensors->minimum() > highestDiff))
				{
					highest = m_Sensors;
					highestDiff = m_Sensors->value() - m_Sensors->minimum();
				}

				if(m_Shields && m_Shields != control && (!highest || m_Shields->value() - m_Shields->minimum() > highestDiff))
				{
					highest = m_Shields;
					highestDiff = m_Shields->value() - m_Shields->minimum();
				}

				if(!highest)
					break;	// wtf?

				highest->setValue(highest->value()-1);
				cursum--;
			}
		}

		// Limit the control that was dragged.
		cursum = SumVolumeSliders();
		if(cursum > m_DesignReport.HullVolume())
			control->setValue(control->value() - (cursum - m_DesignReport.HullVolume()));

		// Unblock widget signals.
		m_Armor->blockSignals(false);
		for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
			(*bi)->blockSignals(false);
		m_Capacitor->blockSignals(false);
		m_Cryo->blockSignals(false);
		for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
			(*fi)->blockSignals(false);
		m_Fuel->blockSignals(false);
		m_Hold->blockSignals(false);
		m_LifeSupport->blockSignals(false);
		m_ManeuverDrive->blockSignals(false);
		m_PowerPlant->blockSignals(false);
		if(m_Sensors)
			m_Sensors->blockSignals(false);
		if(m_Shields)
			m_Shields->blockSignals(false);
		m_Skeleton->blockSignals(false);

		// Update values in design record to match sliders.
		m_DesignReport.SetArmorVolume(m_Armor->value());
		for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
			m_DesignReport.SetBayVolume(bi.key(), (*bi)->value());
		m_DesignReport.SetCapacitorVolume(m_Capacitor->value());
		m_DesignReport.SetCryoBerthVolume(m_Cryo->value());
		for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
			m_DesignReport.SetFTLDriveVolume(fi.key(), (*fi)->value());
		m_DesignReport.SetFuelVolume(m_Fuel->value());
		m_DesignReport.SetHoldVolume(m_Hold->value());
		m_DesignReport.SetLifeSupportVolume(m_LifeSupport->value());
		m_DesignReport.SetManeuverDriveVolume(m_ManeuverDrive->value());
		m_DesignReport.SetPowerPlantVolume(m_PowerPlant->value());
		if(m_Sensors)
			m_DesignReport.SetSensorVolume(m_Sensors->value());
		if(m_Shields)
			m_DesignReport.SetShieldVolume(m_Shields->value());
		m_DesignReport.SetSkeletonVolume(m_Skeleton->value());

		// Update text values.
		m_ArmorLb->setText(QString("%L1m%2").arg(m_DesignReport.ArmorVolume()).arg(g_Au.CubedSymbol()));
		for(QHash<quint16, AuLineEditValue*>::iterator li = m_BayLbs.begin(); li != m_BayLbs.end(); li++)
			(*li)->setText(QString("%L1m%2").arg(m_DesignReport.BayVolume(li.key())).arg(g_Au.CubedSymbol()));
		m_CapacitorLb->setText(QString("%L1m%2").arg(m_DesignReport.CapacitorVolume()).arg(g_Au.CubedSymbol()));
		m_CryoLb->setText(QString("%L1m%2").arg(m_DesignReport.CryoBerthVolume()).arg(g_Au.CubedSymbol()));
		for(QHash<quint16, AuLineEditValue*>::iterator li = m_FTLDriveLbs.begin(); li != m_FTLDriveLbs.end(); li++)
			(*li)->setText(QString("%L1m%2").arg(m_DesignReport.FTLDriveVolume(li.key())).arg(g_Au.CubedSymbol()));
		m_FuelLb->setText(QString("%L1m%2").arg(m_DesignReport.FuelVolume()).arg(g_Au.CubedSymbol()));
		m_HoldLb->setText(QString("%L1m%2").arg(m_DesignReport.HoldVolume()).arg(g_Au.CubedSymbol()));
		m_LifeSupportLb->setText(QString("%L1m%2").arg(m_DesignReport.LifeSupportVolume()).arg(g_Au.CubedSymbol()));
		m_ManeuverDriveLb->setText(QString("%L1m%2").arg(m_DesignReport.ManeuverDriveVolume()).arg(g_Au.CubedSymbol()));
		m_PowerPlantLb->setText(QString("%L1m%2").arg(m_DesignReport.PowerPlantVolume()).arg(g_Au.CubedSymbol()));
		if(m_SensorsLb)
			m_SensorsLb->setText(QString("%L1m%2").arg(m_DesignReport.SensorVolume()).arg(g_Au.CubedSymbol()));
		if(m_ShieldsLb)
			m_ShieldsLb->setText(QString("%L1m%2").arg(m_DesignReport.ShieldVolume()).arg(g_Au.CubedSymbol()));
		m_SkeletonLb->setText(QString("%L1m%2").arg(m_DesignReport.SkeletonVolume()).arg(g_Au.CubedSymbol()));
	}
	else if(cursum < m_DesignReport.HullVolume())
	{
		// Adjust skeleton to use all unallocated hull volume.
		int skeleton = m_Skeleton->value() + (m_DesignReport.HullVolume() - cursum);
		m_Skeleton->blockSignals(true);
		m_Skeleton->setValue(skeleton);
		m_Skeleton->blockSignals(false);
		m_DesignReport.SetSkeletonVolume(m_Skeleton->value());
		m_SkeletonLb->setText(QString("%L1m%2").arg(m_DesignReport.SkeletonVolume()).arg(g_Au.CubedSymbol()));
	}

	m_DesignReport.UpdateCalculatedValues();
	//qDebug() << "sumSliders" << SumVolumeSliders() << "hullVol" << m_DesignReport.HullVolume();
	SetInfoText(SumVolumeSliders() != m_DesignReport.HullVolume());
	m_Analysis->setHtml(m_DesignReport.ReportAnalysis(AuDesign::Edit));	// Edit state for warnings.
}

quint32 AuServiceRefit::VolumeValue(const QString& s) const
{
	QString s1(s);
	s1.replace("+", "");
	s1.replace("-", "");
	s1.replace(".", "");
	s1.replace(",", "");
	s1.replace("m", "");
	s1.replace(g_Au.CubedSymbol(), "");
	quint32 vol = s1.toUInt();
	if(vol > m_DesignReport.HullVolume())
		vol = m_DesignReport.HullVolume();
	return vol;
}

void AuServiceRefit::ArmorEdited()
{
	quint32 value = VolumeValue(m_ArmorLb->text());
	m_Armor->setValue(value);
}

void AuServiceRefit::BayEdited(int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_Bays.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_BayLbs.find(partId);
	if(bi == m_Bays.cend() || li == m_BayLbs.cend())
		return;

	quint32 value = VolumeValue((*li)->text());
	(*bi)->setValue(value);
}

void AuServiceRefit::CapacitorEdited()
{
	quint32 value = VolumeValue(m_CapacitorLb->text());
	m_Capacitor->setValue(value);
}

void AuServiceRefit::CryoEdited()
{
	quint32 value = VolumeValue(m_CryoLb->text());
	m_Cryo->setValue(value);
}

void AuServiceRefit::FTLDriveEdited(int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_FTLDrives.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_FTLDriveLbs.find(partId);
	if(bi == m_FTLDrives.cend() || li == m_FTLDriveLbs.cend())
		return;

	quint32 value = VolumeValue((*li)->text());
	(*bi)->setValue(value);
}

void AuServiceRefit::FuelEdited()
{
	quint32 value = VolumeValue(m_FuelLb->text());
	m_Fuel->setValue(value);
}

void AuServiceRefit::SkeletonEdited()
{
	quint32 value = VolumeValue(m_SkeletonLb->text());
	m_Skeleton->setValue(value);
}

void AuServiceRefit::HoldEdited()
{
	quint32 value = VolumeValue(m_HoldLb->text());
	m_Hold->setValue(value);
}

void AuServiceRefit::LifeSupportEdited()
{
	quint32 value = VolumeValue(m_LifeSupportLb->text());
	m_LifeSupport->setValue(value);
}

void AuServiceRefit::ManeuverDriveEdited()
{
	quint32 value = VolumeValue(m_ManeuverDriveLb->text());
	m_ManeuverDrive->setValue(value);
}

void AuServiceRefit::PowerPlantEdited()
{
	quint32 value = VolumeValue(m_PowerPlantLb->text());
	m_PowerPlant->setValue(value);
}

void AuServiceRefit::SensorsEdited()
{
	quint32 value = VolumeValue(m_SensorsLb->text());
	m_Sensors->setValue(value);
}

void AuServiceRefit::ShieldsEdited()
{
	quint32 value = VolumeValue(m_ShieldsLb->text());
	m_Shields->setValue(value);
}

void AuServiceRefit::SetArmor(int value)
{
	m_DesignReport.SetArmorVolume(value);
	m_ArmorLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Armor);
}

void AuServiceRefit::SetBay(int value, int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_Bays.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_BayLbs.find(partId);
	if(bi == m_Bays.cend() || li == m_BayLbs.cend())
		return;

	m_DesignReport.SetBayVolume(partId, value);
	(*li)->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(*bi);
}

void AuServiceRefit::SetCapacitor(int value)
{
	m_DesignReport.SetCapacitorVolume(value);
	m_CapacitorLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Capacitor);
}

void AuServiceRefit::SetCryo(int value)
{
	m_DesignReport.SetCryoBerthVolume(value);
	m_CryoLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Cryo);
}

void AuServiceRefit::SetFTLDrive(int value, int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_FTLDrives.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_FTLDriveLbs.find(partId);
	if(bi == m_FTLDrives.cend() || li == m_FTLDriveLbs.cend())
		return;

	m_DesignReport.SetFTLDriveVolume(partId, value);
	(*li)->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(*bi);
}

void AuServiceRefit::SetFuel(int value)
{
	m_DesignReport.SetFuelVolume(value);
	m_FuelLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Fuel);
}

void AuServiceRefit::SetHold(int value)
{
	m_DesignReport.SetHoldVolume(value);
	m_HoldLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Hold);
}

void AuServiceRefit::SetSkeleton(int value)
{
	m_DesignReport.SetSkeletonVolume(value);
	m_SkeletonLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Skeleton);
}

void AuServiceRefit::SetLifeSupport(int value)
{
	m_DesignReport.SetLifeSupportVolume(value);
	m_LifeSupportLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_LifeSupport);
}

void AuServiceRefit::SetManeuverDrive(int value)
{
	m_DesignReport.SetManeuverDriveVolume(value);
	m_ManeuverDriveLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_ManeuverDrive);
}

void AuServiceRefit::SetPowerPlant(int value)
{
	m_DesignReport.SetPowerPlantVolume(value);
	m_PowerPlantLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_PowerPlant);
}

void AuServiceRefit::SetSensors(int value)
{
	m_DesignReport.SetSensorVolume(value);
	m_SensorsLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Sensors);
}

void AuServiceRefit::SetShields(int value)
{
	m_DesignReport.SetShieldVolume(value);
	m_ShieldsLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Shields);
}

void AuServiceRefit::ResetVolumes()
{
	QMessageBox mb(g_Au.ApplicationName(),
			tr("All system volumes will be reset to match the configuration of the current spacecraft."),
		QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
		return;

	// Get spaceship information.
	AuView* pView = auApp->Frame()->View();
	if(pView)
	{
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person)
		{
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship && ship->Is(m_ShipId))
			{
				DaSc* sc = dynamic_cast<DaSc*>(ship);
				if(sc)
				{
					m_DesignReport = sc->LockDesignAnalysis();
					sc->UnlockDesignAnalysis();
					m_MinQ->setValue(sc->QualityMade());
				}
			}
		}
		pView->UnlockSceneAtom();
	}

	// Refit is available.
	quint32 hullVol = 0;
	quint32 armorVol = 0;
	quint32 capVol = 0;
	quint32 cryoVol = 0;
	quint32 fuelVol = 0;
	quint32 holdVol = 0;
	quint32 lsVol = 0;
	quint32 drvVol = 0;
	quint32 ppVol = 0;
	quint32 sensVol = 0;
	quint32 shVol = 0;
	quint32 skVol = 0;	// skeleton/framework
	bool hasSensors = false;
	bool hasShields = false;
	QMap<quint16, AuScReportSysVol> ftlDrives, bays;
	m_DesignReport.HullVolumeDistribution(hullVol, armorVol, capVol, cryoVol, fuelVol, holdVol, lsVol, drvVol, ppVol, sensVol,
			hasSensors, shVol, hasShields, ftlDrives, bays, skVol);
	quint32 totftl = 0;
	for(QMap<quint16, AuScReportSysVol>::const_iterator pi = ftlDrives.cbegin(); pi != ftlDrives.cend(); pi++)
		totftl += (*pi).Volume();
	quint32 totbays = 0;
	for(QMap<quint16, AuScReportSysVol>::const_iterator pi = bays.cbegin(); pi != bays.cend(); pi++)
		totbays += (*pi).Volume();
	SetInfoText(hullVol !=
			armorVol + capVol + cryoVol + fuelVol + holdVol + lsVol + drvVol + ppVol + sensVol + shVol + totftl + totbays + skVol);
	//if(hullVol !=
			//armorVol + capVol + cryoVol + fuelVol + holdVol + lsVol + drvVol + ppVol + sensVol + shVol + totftl + totbays + skVol)
	//{
		//qDebug() << "hullVol" << hullVol << "sum" <<
			//(armorVol + capVol + cryoVol + fuelVol + holdVol + lsVol + drvVol + ppVol + sensVol + shVol + totftl + totbays + skVol);
	//}

	m_Analysis->setHtml(m_DesignReport.ReportAnalysis(AuDesign::Edit));	// Edit state for warnings.

	// Armor
	{
		m_Armor->blockSignals(true);
		m_Armor->setRange(0, hullVol);						// Armor can be set to zero.
		m_Armor->setValue(armorVol);
		m_Armor->blockSignals(false);
		m_ArmorLb->setText(QString("%L1m%2").arg(armorVol).arg(g_Au.CubedSymbol()));
	}

	// Capacitor
	{
		m_Capacitor->blockSignals(true);
		m_Capacitor->setRange(1, hullVol);					// Capacitor cannot be zero.
		m_Capacitor->setValue(capVol);
		m_Capacitor->blockSignals(false);
		m_CapacitorLb->setText(QString("%L1m%2").arg(capVol).arg(g_Au.CubedSymbol()));
	}

	// Cryo Berths
	{
		m_Cryo->blockSignals(true);
		m_Cryo->setRange(0, hullVol);						// Cryo can be set to zero.
		m_Cryo->setValue(cryoVol);
		m_Cryo->blockSignals(false);
		m_CryoLb->setText(QString("%L1m%2").arg(cryoVol).arg(g_Au.CubedSymbol()));
	}

	{
		m_Fuel->blockSignals(true);
		m_Fuel->setRange(1, hullVol);						// Fuel cannot be zero.
		m_Fuel->setValue(fuelVol);
		m_Fuel->blockSignals(false);
		m_FuelLb->setText(QString("%L1m%2").arg(fuelVol).arg(g_Au.CubedSymbol()));
	}

	{
		m_Hold->blockSignals(true);
		m_Hold->setRange(0, hullVol);						// Hold can be set to zero.
		m_Hold->setValue(holdVol);
		m_Hold->blockSignals(false);
		m_HoldLb->setText(QString("%L1m%2").arg(holdVol).arg(g_Au.CubedSymbol()));
	}

	{
		m_LifeSupport->blockSignals(true);
		m_LifeSupport->setRange(1, hullVol);				// Life support cannot be zero.
		m_LifeSupport->setValue(lsVol);
		m_LifeSupport->blockSignals(false);
		m_LifeSupportLb->setText(QString("%L1m%2").arg(lsVol).arg(g_Au.CubedSymbol()));
	}

	{
		m_ManeuverDrive->blockSignals(true);
		m_ManeuverDrive->setRange(1, hullVol);				// Maneuver drive cannot be zero.
		m_ManeuverDrive->setValue(drvVol);
		m_ManeuverDrive->blockSignals(false);
		m_ManeuverDriveLb->setText(QString("%L1m%2").arg(drvVol).arg(g_Au.CubedSymbol()));
	}

	{
		m_PowerPlant->blockSignals(true);
		m_PowerPlant->setRange(1, hullVol);				// Power plant cannot be zero.
		m_PowerPlant->setValue(ppVol);
		m_PowerPlant->blockSignals(false);
		m_PowerPlantLb->setText(QString("%L1m%2").arg(ppVol).arg(g_Au.CubedSymbol()));
	}

	if(m_Sensors && m_SensorsLb)
	{
		m_Sensors->blockSignals(true);
		m_Sensors->setRange(1, hullVol);					// Sensors cannot be zero now that the system exists.
		m_Sensors->setValue(sensVol);
		m_Sensors->blockSignals(false);
		m_SensorsLb->setText(QString("%L1m%2").arg(sensVol).arg(g_Au.CubedSymbol()));
	}

	if(m_Shields && m_ShieldsLb)
	{
		m_Shields->blockSignals(true);
		m_Shields->setRange(1, hullVol);					// Shields cannot be zero now that the system exists.
		m_Shields->setValue(shVol);
		m_Shields->blockSignals(false);
		m_ShieldsLb->setText(QString("%L1m%2").arg(shVol).arg(g_Au.CubedSymbol()));
	}

	for(QMap<quint16, AuScReportSysVol>::const_iterator pi = ftlDrives.cbegin(); pi != ftlDrives.cend(); pi++)
	{
		QHash<quint16, AuSliderValue*>::iterator si = m_FTLDrives.find(pi.key());
		AuSliderValue* sv = *si;
		sv->blockSignals(true);
		sv->setRange(1, hullVol);						// FTL drive cannot be zero now that the system exists.
		sv->setValue((*pi).Volume());
		sv->blockSignals(false);
		QHash<quint16, AuLineEditValue*>::iterator ei = m_FTLDriveLbs.find(pi.key());
		AuLineEditValue* svl = *ei;
		svl->setText(QString("%L1m%2").arg((*pi).Volume()).arg(g_Au.CubedSymbol()));
	}

	for(QMap<quint16, AuScReportSysVol>::const_iterator pi = bays.cbegin(); pi != bays.cend(); pi++)
	{
		QHash<quint16, AuSliderValue*>::iterator si = m_Bays.find(pi.key());
		AuSliderValue* sv = *si;
		sv->blockSignals(true);
		sv->setRange(1, hullVol);						// Bay cannot be zero now that the system exists.
		sv->setValue((*pi).Volume());
		sv->blockSignals(false);
		QHash<quint16, AuLineEditValue*>::iterator ei = m_BayLbs.find(pi.key());
		AuLineEditValue* svl = *ei;
		svl->setText(QString("%L1m%2").arg((*pi).Volume()).arg(g_Au.CubedSymbol()));
	}

	{
		m_Skeleton->blockSignals(true);
		m_Skeleton->setRange(0, hullVol);						// Skeleton can be zero.
		m_Skeleton->setValue(skVol);
		m_Skeleton->blockSignals(false);
		m_SkeletonLb->setText(QString("%L1m%2").arg(skVol).arg(g_Au.CubedSymbol()));
	}
}

void AuServiceRefit::SetInfoText(bool sizeMismatch)
{
	if(sizeMismatch)
		m_Info->setText(tr("<html><p>Adjust the amount of hull volume allocated to each system. "
					"</p>"
					"<p>"
					"The spacecraft must be rigged for repairs: stopped, sensors off, shields off, power plant off, "
					"maneuver drive off and FTL drive off. "
					"</p>"
					"<p>"
					"Cargo aboard will impose a minimum on the <i>Hold</i> volume. "
					"Cryo travellers aboard will impose a minimum on the <i>Cryo Berths</i> volume. "
					"Excess fuel is jettisoned if it does not fit into the resulting <i>Fuel</i> volume. "
					"Excess power is lost if it cannot be stored in the resulting <i>Capacitor</i> volume. "
					"</p>"
					"<p>"
					"<i>Minimum quality</i> affects systems that <u>increase</u> in volume. "
					"The module quality is reduced to the minimum quality, on modules associated with those systems. "
					"Module quality is not affected on systems that decrease in volume. "
					"</p>"
					"<p>"
					"<b>Current system volume distribution does not equal total hull volume.</b> "
					"A software error permitted this to happen in some designs. "
					"Any change to the system volumes will correct the error. "
					"To correct the error with as little change as possible, "
					"enter a 1 into the volume control for <i>Fuel</i>. "
					"</p>"
					"</html>"));
	else
		m_Info->setText(tr("<html><p>Adjust the amount of hull volume allocated to each system. "
					"</p>"
					"<p>"
					"The spacecraft must be rigged for repairs: stopped, sensors off, shields off, power plant off, "
					"maneuver drive off and FTL drive off. "
					"</p>"
					"<p>"
					"Cargo aboard will impose a minimum on the <i>Hold</i> volume. "
					"Cryo travellers aboard will impose a minimum on the <i>Cryo Berths</i> volume. "
					"Excess fuel is jettisoned if it does not fit into the resulting <i>Fuel</i> volume. "
					"Excess power is lost if it cannot be stored in the resulting <i>Capacitor</i> volume. "
					"</p>"
					"<p>"
					"<i>Minimum quality</i> affects systems that <u>increase</u> in volume. "
					"The module quality is reduced to the minimum quality, on modules associated with those systems. "
					"Module quality is not affected on systems that decrease in volume. "
					"</p>"
					"</html>"));
}
