//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdesignprops.cpp,v 1.9 2022/02/19 18:25:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscdesignprops.h"
//
#include "auglobal.h"
#include "aulineedit.h"
#include "auscparts.h"
#include "auscreport.h"
#include "auslider.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTextBrowser>

AuScDesignProps::AuScDesignProps(const AuScDesign& shipRec, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/c_ScDesignDrawing.png"));
	setAuDlgTitle("Spacecraft Design Properties");
	m_DesignRec = shipRec;

	quint32 hullVol = shipRec.HullVolume();

	// Design Name
	QLabel* lbName = new QLabel(tr("Design Name"));

	m_Name = new QLineEdit(shipRec.Name(), this);
	m_Name->setToolTip(tr("<html>"
				"The design name must be no more than %1 characters "
				"and it must not contain non-printable characters "
				"or certain reserved characters such as &lt; &gt; &amp; and |."
				"</html>")
			.arg(MaxNameLength()));
	connect(m_Name, SIGNAL(textChanged(const QString&)), SLOT(NameChanged(const QString&)));

	// Design Description
	QLabel* lbDesc = new QLabel(tr("Description"));
	lbDesc->setAlignment(Qt::AlignTop|Qt::AlignLeft);

	m_Description = new QTextEdit(shipRec.Description(), this);
	m_Description->setToolTip(tr("<html>"
				"Enter a description of the design, such as its intended purpose and noteworthy features. "
				"It is ok to summarize but don't literally repeat the design analysis. "
				"</html>"));
	connect(m_Description, SIGNAL(textChanged()), SLOT(DescriptionChanged()));

	// Structure
	QLabel* lbStruc = new QLabel(tr("Structure"), this);

	m_Structure = new QComboBox(this);
	m_Structure->addItem(SpacecraftStructureName(SC_Starship), QVariant(SC_Starship));
	m_Structure->addItem(SpacecraftStructureName(SC_Station), QVariant(SC_Station));
	m_Structure->addItem(SpacecraftStructureName(SC_Spaceship), QVariant(SC_Spaceship));
	m_Structure->setCurrentIndex(m_Structure->findData(shipRec.Structure()));
	m_Structure->setToolTip(tr("<b>Structure</b><br>"
				"Chooses a hull structure for the design.<br>"
				"<b>Starship</b> is a spacecraft with at least one FTL drive.<br>"
				"<b>Station</b> is a spacecraft with minimal maneuver drives.<br>"
				"<b>Spaceship</b> is a spacecraft with no FTL drive."));
	connect(m_Structure, SIGNAL(currentIndexChanged(int)), SLOT(StructureChanged(int)));

	// Min Quality
	QLabel* lbMinQ = new QLabel(tr("Min Quality"), this);

	m_SpinMinQ = new QSpinBox(this);
	m_SpinMinQ->setRange(1, 255);
	m_SpinMinQ->setValue(shipRec.MinQ());
	m_SpinMinQ->setToolTip(tr("<b>Min Quality</b><br>"
				"Applies a minimum quality to the design. "
				"Configures the design to be manufactured at a higher quality than the design requires "
				"because of its mass and equipment."));
	connect(m_SpinMinQ, SIGNAL(valueChanged(int)), SLOT(SetMinQ(int)));

	// Streamline
	QLabel* lbStream = new QLabel(tr("Streamlining"), this);

	m_Streamline = new QComboBox(this);
	for(int s=SLINE_None; s<SLINE_Count; s++)
		if(s != SLINE_Subterranean)
			m_Streamline->addItem(StreamlineName((STREAMLINE)s), s);
	m_Streamline->setCurrentIndex(m_Streamline->findData(shipRec.Streamline()));
	m_Streamline->setToolTip(tr("<html>"
				"<b>Streamlining</b><br>"
				"Streamlining enables greater velocity when maneuvering within an atmosphere and other conditions. "
				"Streamlining hardens the hull against corrosion. "
				"Streamlining increases mass. "
				"<br>"
				"<b>Unstreamlined</b> hulls must move slowly through atmospheres. "
				"Varying degrees of corrosion damage result from contact with atmospheres, sea water and supercooled liquid. "
				"<br>"
				"<b>Atmospheric</b> streamlining enables contact with atmosphere with minimal corrosion from contact with the air. "
				"Varying degrees of corrosion damage result from contact with sea water and supercooled liquid. "
				"<br>"
				"<b>Sea Surface</b> streamlining enables contact with sea water and supercooled liquids "
				"with minimal corrosion due to contact with atmospheres or seas. "
				"Corrosion damage results from submersion in sea water or supercooled liquid. "
				"<br>"
				"<b>Submersible</b> streamlining enables submersion under sea water and supercooled liquids "
				"with minimal corrosion due to contact with atmospheres or seas. "
				//"<br>"
				//"<b>Subterranean</b> streamlining enables low speed penetration of the ground. "
				"</html>"));
	connect(m_Streamline, SIGNAL(currentIndexChanged(int)), SLOT(StreamlineChanged(int)));

	// Landing Orientation (for non-stations)
	QLabel* lbLandingOrient = new QLabel(tr("Preferred Landing Orientation"), this);

	m_LandingOrient = new QComboBox(this);
	for(int s=LORI_Belly; s<LORI_Count; s++)
		m_LandingOrient->addItem(LandingOrientationName((LANDING_ORIENT)s), s);
	m_LandingOrient->setCurrentIndex(m_LandingOrient->findData(shipRec.LandingOrientation()));
	m_LandingOrient->setToolTip(tr("<html>"
				"<b>Landing Orientation</b><br>"
				"<br>"
				"Specifies the preferred orientation for spaceship designs, when they are landed. "
				"</html>"));
	connect(m_LandingOrient, SIGNAL(currentIndexChanged(int)), SLOT(LandingOrientationChanged(int)));

	// Orbit Orientation (for stations)
	QLabel* lbStationOrient = new QLabel(tr("Preferred Orbit Orientation"), this);

	m_StationOrient = new QComboBox(this);
	for(int s=AuScDesign::SORI_Random; s<AuScDesign::SORI_Count; s++)
		m_StationOrient->addItem(AuScDesign::StationOrientationName((AuScDesign::STATION_ORIENT)s), s);
	m_StationOrient->setCurrentIndex(m_StationOrient->findData(shipRec.StationOrientation()));
	m_StationOrient->setToolTip(tr("<html>"
				"<b>Station Orientation</b><br>"
				"<br>"
				"Specifies the preferred orientation for space station designs, when they are launched into orbit. "
				"</html>"));
	connect(m_StationOrient, SIGNAL(currentIndexChanged(int)), SLOT(StationOrientationChanged(int)));

	// System Modules
	QLabel* lbCnC = new QLabel(ScSystemName(SYS_CommandAndControl), this);

	m_CnC = new QComboBox(this);
	m_CnC->setToolTip(tr("<b>Control Stations</b><br>"
				"Control stations of a spacecraft combine into a single system. "
				"The quality of the command and control module determines the trade radius of a spacecraft. "
				"This is demarcated on a space station as a spherical rotating beacon."));
	ListModules(m_CnC, SYS_CommandAndControl, shipRec.CommandAndControlModule());
	connect(m_CnC, SIGNAL(currentIndexChanged(int)), SLOT(CnCModuleChanged(int)));

	QLabel* lbHull = new QLabel(ScSystemName(SYS_Hull), this);

	m_Hull = new QComboBox(this);
	m_Hull->setToolTip(tr("<b>Hull</b><br>"
				"Hull module determines the composition of the hull."));
	ListModules(m_Hull, SYS_Hull, shipRec.HullModule());
	connect(m_Hull, SIGNAL(currentIndexChanged(int)), SLOT(HullModuleChanged(int)));

	QLabel* lbLifeSupportMod = new QLabel(ScSystemName(SYS_LifeSupport), this);

	m_LifeSupportMod = new QComboBox(this);
	m_LifeSupportMod->setToolTip(tr("<b>Life Support</b><br>"
				"Life support module determines the capabilities of the life support system. "
				"A functioning life support system is needed for avatars to respawn in their berth.<br>"
				"<b>Life Support</b> maintains a habitable zero-G environment inside the spacecraft.<br>"
				"<b>Life Support Gravity</b> maintains a habitable environment with gravity inside the spacecraft."));
	ListModules(m_LifeSupportMod, SYS_LifeSupport, shipRec.LifeSupportModule());
	connect(m_LifeSupportMod, SIGNAL(currentIndexChanged(int)), SLOT(LifeSupportModuleChanged(int)));

	QLabel* lbManeuverDriveMod = new QLabel(ScSystemName(SYS_ManeuverDrive), this);

	m_ManeuverDriveMod = new QComboBox(this);
	m_ManeuverDriveMod->setToolTip(tr("<b>Maneuver Drive</b><br>"
				"Spacecraft use maneuver drives for landing, taking off, and movement. "
				"A spacecraft can travel to other solar systems using its maneuver drive. "
				"Maneuver drives cannot propel a spacecraft faster than the speed of light, which is six parsecs per hour.<br>"
				"<b>Rocket Drive</b> converts hydrogen fuel to kinetic energy. "
				"Fuel is consumed directly from the fuel cells. "
				"Rocket driven spacecraft are affected by inertia. "
				"Changes in heading must overcome the spacecraft's inertia on it current course.<br>"
				"<b>Gravity Drive</b> controls gravity and inertia. "
				"Gravity drives use power from the capacitor; they do not consume fuel directly. "
				"Gravity drives are not subject to inertia. "
				"Spacecraft course changes instantly to match heading changes."));
	ListModules(m_ManeuverDriveMod, SYS_ManeuverDrive, shipRec.ManeuverDriveModule());
	connect(m_ManeuverDriveMod, SIGNAL(currentIndexChanged(int)), SLOT(ManeuverDriveModuleChanged(int)));

	QLabel* lbPowerPlantMod = new QLabel(ScSystemName(SYS_PowerPlant), this);

	m_PowerPlantMod = new QComboBox(this);
	m_PowerPlantMod->setToolTip(tr("<b>Power Plant</b><br>"
				"Power plant converts fuel to energy. "
				"Energy is stored in the capacitor."));
	ListModules(m_PowerPlantMod, SYS_PowerPlant, shipRec.PowerPlantModule());
	connect(m_PowerPlantMod, SIGNAL(currentIndexChanged(int)), SLOT(PowerPlantModuleChanged(int)));

	QLabel* lbCapacitorMod = new QLabel(ScSystemName(SYS_Capacitor), this);

	m_CapacitorMod = new QComboBox(this);
	m_CapacitorMod->setToolTip(tr("<b>Capacitor</b><br>"
				"Capacitor stores energy for instantaneous use by all systems of a spacecraft. "
				"<br>"
				"Rocket drives are an exception. "
				"They consume hydrogen fuel directly instead of using energy from the capacitor."));
	ListModules(m_CapacitorMod, SYS_Capacitor, shipRec.CapacitorModule());
	connect(m_CapacitorMod, SIGNAL(currentIndexChanged(int)), SLOT(CapacitorModuleChanged(int)));

	m_Ok = new QPushButton(tr("Ok"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));
	m_Ok->setToolTip(tr("<html>Accepts changes to the spacecraft design. "
				"If this button is disabled, check <i>Design Name</i>."
				"</html>"));

	m_Analysis = new QTextBrowser(this);
	//m_Analysis->setOpenExternalLinks(true);
	m_Analysis->setReadOnly(true);

	QLabel* lbHullVol = new QLabel(tr("<html><b>System Hull Volume</b></html>"), this);
	lbHullVol->setAlignment(Qt::AlignBottom|Qt::AlignLeft);

	QLabel* lbHullVolTot = new QLabel(tr("<html><b>%L1m%2 Total</b></html>").arg(hullVol).arg(g_Au.CubedSymbol()), this);
	lbHullVolTot->setAlignment(Qt::AlignBottom|Qt::AlignLeft);

	// Armor
	QLabel* lbArmor;
	{
		int value = shipRec.ArmorVolume();

		lbArmor = new QLabel(tr("Armor"), this);

		m_Armor = new QSlider(Qt::Horizontal, this);
		m_Armor->setRange(0, hullVol);
		m_Armor->setSingleStep(1);
		m_Armor->setPageStep(1);
		m_Armor->setValue(value);
		m_Armor->setToolTip(tr("<b>Armor</b><br>"
					"Adjusts the volume of hull allocated to armor. "
					"<br>"
					"Hull armor improves defense against physical damage and increases mass."));
		connect(m_Armor, SIGNAL(valueChanged(int)), SLOT(SetArmor(int)));

		m_ArmorLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_ArmorLb, SIGNAL(editingFinished()), SLOT(ArmorEdited()));
	}

	QLabel* lbCapacitor;
	{
		int value = shipRec.CapacitorVolume();

		lbCapacitor = new QLabel(tr("Capacitor"), this);

		m_Capacitor = new QSlider(Qt::Horizontal, this);
		m_Capacitor->setRange(0, hullVol);
		m_Capacitor->setSingleStep(1);
		m_Capacitor->setPageStep(1);
		m_Capacitor->setValue(value);
		m_Capacitor->setToolTip(tr("<html>"
					"<b>Capacitor</b><br>"
					"Adjusts the volume of hull allocated to capacitor. "
					"<br>"
					"Capacitor stores energy for ready use by systems of the spacecraft. "
					"Energy is produced by the power plant. "
					"</html>"));
		connect(m_Capacitor, SIGNAL(valueChanged(int)), SLOT(SetCapacitor(int)));

		m_CapacitorLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_CapacitorLb, SIGNAL(editingFinished()), SLOT(CapacitorEdited()));
	}

	// Cryo Berths
	QLabel* lbCryo;
	{
		int value = shipRec.CryoBerthVolume();

		lbCryo = new QLabel(tr("Cryo Berths"), this);

		m_Cryo = new QSlider(Qt::Horizontal, this);
		m_Cryo->setRange(0, hullVol);
		m_Cryo->setSingleStep(1);
		m_Cryo->setPageStep(1);
		m_Cryo->setValue(value);
		m_Cryo->setToolTip(tr("<b>Cryo Berths</b><br>"
					"Adjusts the volume of hull allocated to cryo berths. "
					"Every 4 cubic meters of volume adds one cryo berth. "
					"Cryo berths do not require additional life support. "
					"<br>"
					"Cryo berths provide a means for carrying large numbers of travellers. "
					"Cryo travellers can be offloaded to cities or revived individually as crew, troops or passengers. "
					"Avatars cannot enter cryo berths."));
		connect(m_Cryo, SIGNAL(valueChanged(int)), SLOT(SetCryo(int)));

		m_CryoLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_CryoLb, SIGNAL(editingFinished()), SLOT(CryoEdited()));
	}

	QLabel* lbFuel;
	{
		int value = shipRec.FuelVolume();

		lbFuel = new QLabel(tr("Fuel"), this);

		m_Fuel = new QSlider(Qt::Horizontal, this);
		m_Fuel->setRange(0, hullVol);
		m_Fuel->setSingleStep(1);
		m_Fuel->setPageStep(1);
		m_Fuel->setValue(value);
		m_Fuel->setToolTip(tr("<html>"
					"<b>Fuel Cells</b><br>"
					"Adjusts the volume of hull allocated to fuel cells. "
					"<br>"
					"Fuel cells store fuel. "
					"Fuel is converted to energy by the power plant. "
					"Fuel is burned for propulsion in rocket driven spacecraft. "
					"</html>"));
		connect(m_Fuel, SIGNAL(valueChanged(int)), SLOT(SetFuel(int)));

		m_FuelLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_FuelLb, SIGNAL(editingFinished()), SLOT(FuelEdited()));
	}

	QLabel* lbSkeleton;
	{
		int value = shipRec.SkeletonVolume();

		lbSkeleton = new QLabel(tr("Framework"), this);

		m_Skeleton = new QSlider(Qt::Horizontal, this);
		m_Skeleton->setRange(0, hullVol);
		m_Skeleton->setSingleStep(1);
		m_Skeleton->setPageStep(1);
		m_Skeleton->setValue(value);
		m_Skeleton->setToolTip(tr("<html>"
					"<b>Framework</b><br>"
					"Adjusts the volume of hull allocated to framework. "
					"Framework is walls, floors, ceilings, beams, columns and other support structures. "
					"<br>"
					"Framework volume adds 4x the structural hit points of other volumes. "
					"</html>"));
		connect(m_Skeleton, SIGNAL(valueChanged(int)), SLOT(SetSkeleton(int)));

		m_SkeletonLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_SkeletonLb, SIGNAL(editingFinished()), SLOT(SkeletonEdited()));
	}

	QLabel* lbHold;
	{
		int value = shipRec.HoldVolume();

		lbHold = new QLabel(tr("Hold"), this);

		m_Hold = new QSlider(Qt::Horizontal, this);
		m_Hold->setRange(0, hullVol);
		m_Hold->setSingleStep(1);
		m_Hold->setPageStep(1);
		m_Hold->setValue(value);
		m_Hold->setToolTip(tr("<html>"
					"<b>Hold</b><br>"
					"Adjusts the volume of hull allocated to hold. "
					"<br>"
					"Hold stores cargo. "
					"</html>"));
		connect(m_Hold, SIGNAL(valueChanged(int)), SLOT(SetHold(int)));

		m_HoldLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_HoldLb, SIGNAL(editingFinished()), SLOT(HoldEdited()));
	}

	QLabel* lbLifeSupport;
	{
		int value = shipRec.LifeSupportVolume();

		lbLifeSupport = new QLabel(tr("Life Support"), this);

		m_LifeSupport = new QSlider(Qt::Horizontal, this);
		m_LifeSupport->setRange(0, hullVol);
		m_LifeSupport->setSingleStep(1);
		m_LifeSupport->setPageStep(1);
		m_LifeSupport->setValue(value);
		m_LifeSupport->setToolTip(tr("<html>"
					"<b>Life Support</b><br>"
					"Adjusts the volume of hull allocated to life support. "
					"<br>"
					"Life support maintains a habitable environment inside the spacecraft. "
					"</html>"));
		connect(m_LifeSupport, SIGNAL(valueChanged(int)), SLOT(SetLifeSupport(int)));

		m_LifeSupportLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_LifeSupportLb, SIGNAL(editingFinished()), SLOT(LifeSupportEdited()));
	}

	QLabel* lbManeuverDrive;
	{
		int value = shipRec.ManeuverDriveVolume();

		lbManeuverDrive = new QLabel(tr("Maneuver Drive"), this);

		m_ManeuverDrive = new QSlider(Qt::Horizontal, this);
		m_ManeuverDrive->setRange(0, hullVol);
		m_ManeuverDrive->setSingleStep(1);
		m_ManeuverDrive->setPageStep(1);
		m_ManeuverDrive->setValue(value);
		m_ManeuverDrive->setToolTip(tr("<html>"
					"<b>Maneuver Drive</b><br>"
					"Adjusts the volume of hull allocated to maneuver drives. "
					"<br>"
					"Maneuver drives move the spacecraft through space. "
					"Maneuver drives consume power from the capacitor, except rocket drives which burn fuel directly. "
					"</html>"));
		connect(m_ManeuverDrive, SIGNAL(valueChanged(int)), SLOT(SetManeuverDrive(int)));

		m_ManeuverDriveLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_ManeuverDriveLb, SIGNAL(editingFinished()), SLOT(ManeuverDriveEdited()));
	}

	QLabel* lbPowerPlant;
	{
		int value = shipRec.PowerPlantVolume();

		lbPowerPlant = new QLabel(tr("Power Plant"), this);

		m_PowerPlant = new QSlider(Qt::Horizontal, this);
		m_PowerPlant->setRange(0, hullVol);
		m_PowerPlant->setSingleStep(1);
		m_PowerPlant->setPageStep(1);
		m_PowerPlant->setValue(value);
		m_PowerPlant->setToolTip(tr("<html>"
					"<b>Power Plant</b><br>"
					"Adjusts the volume of hull allocated to power plant. "
					"<br>"
					"Power plant converts fuel into energy and stores it in the capacitor. "
					"</html>"));
		connect(m_PowerPlant, SIGNAL(valueChanged(int)), SLOT(SetPowerPlant(int)));

		m_PowerPlantLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_PowerPlantLb, SIGNAL(editingFinished()), SLOT(PowerPlantEdited()));
	}

	QLabel* lbSensors = 0;
	if(shipRec.SensorModule() != CID_Invalid)
	{
		int value = shipRec.SensorVolume();

		lbSensors = new QLabel(tr("Sensors"), this);

		m_Sensors = new QSlider(Qt::Horizontal, this);
		m_Sensors->setRange(0, hullVol);
		m_Sensors->setSingleStep(1);
		m_Sensors->setPageStep(1);
		m_Sensors->setValue(value);
		m_Sensors->setToolTip(tr("<html>"
					"<b>Sensors</b><br>"
					"Adjusts the volume of hull allocated to sensors. "
					"<br>"
					"Sensors detect worlds, spacecraft, cities, vehicles and other objects in the environment of the spacecraft. "
					"</html>"));
		connect(m_Sensors, SIGNAL(valueChanged(int)), SLOT(SetSensors(int)));

		m_SensorsLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_SensorsLb, SIGNAL(editingFinished()), SLOT(SensorsEdited()));
	}
	else
	{
		m_Sensors = 0;
		m_SensorsLb = 0;
	}

	QLabel* lbShields = 0;
	if(shipRec.ShieldModule() != CID_Invalid)
	{
		int value = shipRec.ShieldVolume();

		lbShields = new QLabel(tr("Shields"), this);

		m_Shields = new QSlider(Qt::Horizontal, this);
		m_Shields->setRange(0, hullVol);
		m_Shields->setSingleStep(1);
		m_Shields->setPageStep(1);
		m_Shields->setValue(value);
		m_Shields->setToolTip(tr("<html>"
					"<b>Shields</b><br>"
					"Adjusts the volume of hull allocated to shields. "
					"<br>"
					"Shields create a protective field around the spacecraft. "
					"</html>"));
		connect(m_Shields, SIGNAL(valueChanged(int)), SLOT(SetShields(int)));

		m_ShieldsLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_ShieldsLb, SIGNAL(editingFinished()), SLOT(ShieldsEdited()));
	}
	else
	{
		m_Shields = 0;
		m_ShieldsLb = 0;
	}

	QHash<quint16, QLabel*> lbFTLDrives;
	{
		AuScPartSet ftlDrives;
		shipRec.FTLDrives(ftlDrives);
		if(!ftlDrives.empty())
		{
			for(QSet<quint16>::const_iterator pi = ftlDrives.cbegin(); pi != ftlDrives.cend(); pi++)
			{
				QString n = QString("%1. %2").arg(*pi).arg(g_Au.CommodityName(shipRec.PartModule(*pi)));
				QLabel* lb = new QLabel(n, this);

				int value = shipRec.FTLDriveVolume(*pi);

				AuSliderValue* sv = new AuSliderValue(Qt::Horizontal, *pi, this);
				sv->setRange(0, hullVol);
				sv->setSingleStep(1);
				sv->setPageStep(1);
				sv->setValue(value);
				sv->setToolTip(tr("<html>"
							"<b>FTL Drive</b><br>"
							"Adjusts the volume of hull allocated to this FTL drive. "
							"<br>"
							"FTL drives enable a spacecraft to travel Faster Than Light. "
							"</html>"));
				connect(sv, SIGNAL(ValueChanged(int, int)), SLOT(SetFTLDrive(int, int)));

				AuLineEditValue* svl = new AuLineEditValue(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), *pi, this);
				connect(svl, SIGNAL(EditingFinished(int)), SLOT(FTLDriveEdited(int)));

				lbFTLDrives.insert(*pi, lb);
				m_FTLDrives.insert(*pi, sv);
				m_FTLDriveLbs.insert(*pi, svl);
			}
		}
	}

	QHash<quint16, QLabel*> lbBays;
	{
		AuScPartSet bays;
		shipRec.Bays(bays);
		if(!bays.empty())
		{
			for(QSet<quint16>::const_iterator pi = bays.cbegin(); pi != bays.cend(); pi++)
			{
				QString n = QString("%1. %2").arg(*pi).arg(g_Au.CommodityName(shipRec.PartModule(*pi)));
				QLabel* lb = new QLabel(n, this);

				int value = shipRec.BayVolume(*pi);

				AuSliderValue* sv = new AuSliderValue(Qt::Horizontal, *pi, this);
				sv->setRange(0, hullVol);
				sv->setSingleStep(1);
				sv->setPageStep(1);
				sv->setValue(value);
				sv->setToolTip(tr("<html>"
							"<b>Bay</b><br>"
							"Adjusts the volume of hull allocated to this bay. "
							"<br>"
							"Bays provide equipment for large scale weapons, resource harvesting equipment, tractor beams, etc. "
							"Bay equipment is directionless and it is not blocked by the hull or other parts of the spacecraft. "
							"</html>"));
				connect(sv, SIGNAL(ValueChanged(int, int)), SLOT(SetBay(int, int)));

				AuLineEditValue* svl = new AuLineEditValue(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), *pi, this);
				connect(svl, SIGNAL(EditingFinished(int)), SLOT(BayEdited(int)));

				lbBays.insert(*pi, lb);
				m_Bays.insert(*pi, sv);
				m_BayLbs.insert(*pi, svl);
			}
		}
	}

	QScrollArea* scroll = new QScrollArea;
	{
		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(lbHullVol, row,0);
		lay->addWidget(lbHullVolTot, row,2);
		row++;
		lay->addWidget(lbSkeleton, row,0);
		lay->addWidget(m_Skeleton, row,1);
		lay->addWidget(m_SkeletonLb, row,2);
		row++;
		lay->addWidget(lbArmor, row,0);
		lay->addWidget(m_Armor, row,1);
		lay->addWidget(m_ArmorLb, row,2);
		row++;
		lay->addWidget(lbCapacitor, row,0);
		lay->addWidget(m_Capacitor, row,1);
		lay->addWidget(m_CapacitorLb, row,2);
		row++;
		lay->addWidget(lbCryo, row,0);
		lay->addWidget(m_Cryo, row,1);
		lay->addWidget(m_CryoLb, row,2);
		row++;
		lay->addWidget(lbFuel, row,0);
		lay->addWidget(m_Fuel, row,1);
		lay->addWidget(m_FuelLb, row,2);
		row++;
		lay->addWidget(lbHold, row,0);
		lay->addWidget(m_Hold, row,1);
		lay->addWidget(m_HoldLb, row,2);
		row++;
		lay->addWidget(lbLifeSupport, row,0);
		lay->addWidget(m_LifeSupport, row,1);
		lay->addWidget(m_LifeSupportLb, row,2);
		row++;
		lay->addWidget(lbManeuverDrive, row,0);
		lay->addWidget(m_ManeuverDrive, row,1);
		lay->addWidget(m_ManeuverDriveLb, row,2);
		row++;
		lay->addWidget(lbPowerPlant, row,0);
		lay->addWidget(m_PowerPlant, row,1);
		lay->addWidget(m_PowerPlantLb, row,2);
		row++;
		if(lbSensors)
		{
			lay->addWidget(lbSensors, row,0);
			lay->addWidget(m_Sensors, row,1);
			lay->addWidget(m_SensorsLb, row,2);
			row++;
		}
		if(lbShields)
		{
			lay->addWidget(lbShields, row,0);
			lay->addWidget(m_Shields, row,1);
			lay->addWidget(m_ShieldsLb, row,2);
			row++;
		}
		if(!lbFTLDrives.empty())
		{
			QHash<quint16, QLabel*>::const_iterator ni = lbFTLDrives.cbegin();
			QHash<quint16, AuSliderValue*>::const_iterator si = m_FTLDrives.cbegin();
			QHash<quint16, AuLineEditValue*>::const_iterator li = m_FTLDriveLbs.cbegin();
			while(ni != lbFTLDrives.cend() && si != m_FTLDrives.cend() && li != m_FTLDriveLbs.cend())
			{
				lay->addWidget(*ni, row,0);
				lay->addWidget(*si, row,1);
				lay->addWidget(*li, row,2);
				row++;

				ni++;
				si++;
				li++;
			}
		}
		if(!lbBays.empty())
		{
			QHash<quint16, QLabel*>::const_iterator ni = lbBays.cbegin();
			QHash<quint16, AuSliderValue*>::const_iterator si = m_Bays.cbegin();
			QHash<quint16, AuLineEditValue*>::const_iterator li = m_BayLbs.cbegin();
			while(ni != lbBays.cend() && si != m_Bays.cend() && li != m_BayLbs.cend())
			{
				lay->addWidget(*ni, row,0);
				lay->addWidget(*si, row,1);
				lay->addWidget(*li, row,2);
				row++;

				ni++;
				si++;
				li++;
			}
		}
		lay->setColumnStretch(1,1);
		QWidget* wf = new QWidget();
		wf->setLayout(lay);
		scroll->setWidget(wf);
		scroll->setWidgetResizable(true);
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbName, row,0);
	lay->addWidget(m_Name, row,1, 1,2);
	row++;
	lay->addWidget(lbDesc, row,0);
	lay->addWidget(m_Description, row,1, 1,2);
	row++;
	lay->addWidget(lbStruc, row,0);
	lay->addWidget(m_Structure, row,1, 1,2);
	row++;
	lay->addWidget(lbMinQ, row,0);
	lay->addWidget(m_SpinMinQ, row,1, 1,2);
	row++;
	lay->addWidget(lbHull, row,0);
	lay->addWidget(m_Hull, row,1, 1,2);
	row++;
	lay->addWidget(lbStream, row,0);
	lay->addWidget(m_Streamline, row,1, 1,2);
	row++;
	lay->addWidget(lbLandingOrient, row,0);
	lay->addWidget(m_LandingOrient, row,1, 1,2);
	row++;
	lay->addWidget(lbStationOrient, row,0);
	lay->addWidget(m_StationOrient, row,1, 1,2);
	row++;
	lay->addWidget(lbCnC, row,0);
	lay->addWidget(m_CnC, row,1, 1,2);
	row++;
	lay->addWidget(lbLifeSupportMod, row,0);
	lay->addWidget(m_LifeSupportMod, row,1, 1,2);
	row++;
	lay->addWidget(lbCapacitorMod, row,0);
	lay->addWidget(m_CapacitorMod, row,1, 1,2);
	row++;
	lay->addWidget(lbPowerPlantMod, row,0);
	lay->addWidget(m_PowerPlantMod, row,1, 1,2);
	row++;
	lay->addWidget(lbManeuverDriveMod, row,0);
	lay->addWidget(m_ManeuverDriveMod, row,1, 1,2);
	row++;
	lay->addWidget(scroll, row,0, 1,3);
	lay->setColumnStretch(1,1);
	lay->setSpacing(2);
	lay->setMargin(0);

	QWidget* controls = new QWidget(this);
	controls->setLayout(lay);

	m_Splitter = new QSplitter;
	m_Splitter->addWidget(controls);
	m_Splitter->addWidget(m_Analysis);
	m_Splitter->setChildrenCollapsible(false);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(m_Ok);
	blay->setSpacing(2);
	blay->setMargin(0);

	QVBoxLayout* dlay = new QVBoxLayout();
	dlay->addWidget(m_Splitter);
	dlay->addLayout(blay);

	dlay->setSpacing(2);
	dlay->setMargin(4);
	setAuDlgLayout(dlay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/ScDesignProps/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);

	EnableControls();
}

AuScDesignProps::~AuScDesignProps()
{
}

void AuScDesignProps::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		LimitVolumeSliders(0);
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuScDesignProps::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DesignSpacecraftProperties/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DesignSpacecraftProperties/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DesignSpacecraftProperties/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DesignSpacecraftProperties/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScDesignProps::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DesignSpacecraftProperties/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DesignSpacecraftProperties/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DesignSpacecraftProperties/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DesignSpacecraftProperties/SizeY",sz.height());
}

void AuScDesignProps::UpdateReport()
{
	QString rpt(m_DesignRec.ReportAnalysis());
	if(rpt.isEmpty())
	{
		rpt = tr("<html>"
				"<p>"
				"Design contains %1 parts. "
				"<br>"
				"Design reports no hull volume. "
				"</p>"

				"<p>"
				"Obstruction model data is needed to analyze a design and report its hull volume and other details. "
				"Be sure the obstruction model is up-to-date and the design contains at least one hull part. "
				"</p>"

				"<p>"
				"Most changes to the geometry invalidate the obstruction model. "
				"Use <i>Refresh Obstructions</i> to reconstruct the obstruction model after making changes. "
				"</p>"

				"</html>")
			.arg(m_DesignRec.Parts().empty() ? tr("no") : QString("%1").arg(m_DesignRec.Parts().size()));
	}
	m_Analysis->setHtml(rpt);
}

void AuScDesignProps::EnableControls()
{
	bool enableApply = true;
	int index = m_Structure->currentIndex();
	if(index < 0)
		enableApply = false;

	if(!m_Name->text().isEmpty() && !IsNameValid(m_Name->text().trimmed()))	// Empty name is ok
		enableApply = false;

	m_Ok->setEnabled(enableApply);

	bool enableStationOrient = (SPACECRAFT_STRUCTURE)m_Structure->itemData(index).toInt() == SC_Station;
	m_LandingOrient->setEnabled(!enableStationOrient);
	m_StationOrient->setEnabled(enableStationOrient);
}

void AuScDesignProps::ListModules(QComboBox* cb, SYSTEM system, COMMODITYID currentModuleCID)
{
	for(DbCommodityArray::const_iterator ci = g_Au.m_CommodityArray.cbegin(); ci != g_Au.m_CommodityArray.cend(); ci++)
		if(DbCommodity::IsScModule(system, (*ci).m_id))
			cb->addItem(QIcon((*ci).BitmapName()), (*ci).m_name, (*ci).m_id);

	int curIndex = cb->findData(currentModuleCID);
	if(curIndex >= 0)
		cb->setCurrentIndex(curIndex);
}

void AuScDesignProps::SetArmor(int value)
{
	m_DesignRec.SetArmorVolume(value);
	m_ArmorLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Armor);
}

void AuScDesignProps::SetBay(int value, int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_Bays.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_BayLbs.find(partId);
	if(bi == m_Bays.cend() || li == m_BayLbs.cend())
		return;

	m_DesignRec.SetBayVolume(partId, value);
	(*li)->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(*bi);
}

void AuScDesignProps::SetCapacitor(int value)
{
	m_DesignRec.SetCapacitorVolume(value);
	m_CapacitorLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Capacitor);
}

void AuScDesignProps::SetCryo(int value)
{
	m_DesignRec.SetCryoBerthVolume(value);
	m_CryoLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Cryo);
}

void AuScDesignProps::SetFTLDrive(int value, int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_FTLDrives.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_FTLDriveLbs.find(partId);
	if(bi == m_FTLDrives.cend() || li == m_FTLDriveLbs.cend())
		return;

	m_DesignRec.SetFTLDriveVolume(partId, value);
	(*li)->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(*bi);
}

void AuScDesignProps::SetFuel(int value)
{
	m_DesignRec.SetFuelVolume(value);
	m_FuelLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Fuel);
}

void AuScDesignProps::SetHold(int value)
{
	m_DesignRec.SetHoldVolume(value);
	m_HoldLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Hold);
}

void AuScDesignProps::SetSkeleton(int value)
{
	m_DesignRec.SetSkeletonVolume(value);
	m_SkeletonLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Skeleton);
}

void AuScDesignProps::SetLifeSupport(int value)
{
	m_DesignRec.SetLifeSupportVolume(value);
	m_LifeSupportLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_LifeSupport);
}

void AuScDesignProps::SetManeuverDrive(int value)
{
	m_DesignRec.SetManeuverDriveVolume(value);
	m_ManeuverDriveLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_ManeuverDrive);
}

void AuScDesignProps::SetPowerPlant(int value)
{
	m_DesignRec.SetPowerPlantVolume(value);
	m_PowerPlantLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_PowerPlant);
}

void AuScDesignProps::SetSensors(int value)
{
	m_DesignRec.SetSensorVolume(value);
	m_SensorsLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Sensors);
}

void AuScDesignProps::SetShields(int value)
{
	m_DesignRec.SetShieldVolume(value);
	m_ShieldsLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Shields);
}

void AuScDesignProps::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ScDesignProps/Splitter", m_Splitter->saveState());
}

void AuScDesignProps::StreamlineChanged(int index)
{
	m_DesignRec.SetStreamline((STREAMLINE)m_Streamline->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::LandingOrientationChanged(int index)
{
	m_DesignRec.SetLandingOrientation((LANDING_ORIENT)m_LandingOrient->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::StationOrientationChanged(int index)
{
	m_DesignRec.SetStationOrientation((AuScDesign::STATION_ORIENT)m_StationOrient->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::StructureChanged(int index)
{
	m_DesignRec.SetStructure((SPACECRAFT_STRUCTURE)m_Structure->itemData(index).toInt());
	UpdateReport();
	EnableControls();
}

void AuScDesignProps::SetMinQ(int value)
{
	m_DesignRec.SetMinQ(value);
	UpdateReport();
}

void AuScDesignProps::CapacitorModuleChanged(int index)
{
	m_DesignRec.SetCapacitorModule((COMMODITYID)m_CapacitorMod->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::CnCModuleChanged(int index)
{
	m_DesignRec.SetCommandAndControlModule((COMMODITYID)m_CnC->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::HullModuleChanged(int index)
{
	m_DesignRec.SetHullModule((COMMODITYID)m_Hull->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::LifeSupportModuleChanged(int index)
{
	m_DesignRec.SetLifeSupportModule((COMMODITYID)m_LifeSupportMod->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::NameChanged(const QString& text)
{
	m_DesignRec.SetName(text.trimmed());
	UpdateReport();
	EnableControls();
}

void AuScDesignProps::DescriptionChanged()
{
	QString text;
	if(!m_Description->toPlainText().isEmpty())
		text = m_Description->toHtml();
	m_DesignRec.SetDescription(text);
	UpdateReport();
}

void AuScDesignProps::ManeuverDriveModuleChanged(int index)
{
	m_DesignRec.SetManeuverDriveModule((COMMODITYID)m_ManeuverDriveMod->itemData(index).toInt());
	UpdateReport();
}

void AuScDesignProps::PowerPlantModuleChanged(int index)
{
	m_DesignRec.SetPowerPlantModule((COMMODITYID)m_PowerPlantMod->itemData(index).toInt());
	UpdateReport();
}

int AuScDesignProps::SumVolumeSliders() const
{
	int cursum = m_Armor->value();
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

void AuScDesignProps::LimitVolumeSliders(QSlider* control)
{
	// Get the sum of the volume sliders.
	quint32 cursum = SumVolumeSliders();
	//qDebug() << "cursum" << cursum << "hullVol" << m_DesignRec.HullVolume();
	if(cursum > m_DesignRec.HullVolume())
	{
		// We must make adjustments so the sum does not exceed the hull volume.
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
		int remove = cursum - m_DesignRec.HullVolume();
		if(m_Skeleton != control && m_Skeleton->value() >= remove)
		{
			// We can take it all from skeleton.
			m_Skeleton->setValue(m_Skeleton->value() - remove);
		}
		else if(remove > 250)
		{
			// Removing enough to take a ratio from each control. The else leg is way slow when adjusting millions.
			// Get the total removable cubic meters so we can calculate each slider's share of the amount to be removed.
			int totalRemovable = 0;
			if(m_Armor != control)
				totalRemovable += m_Armor->value();
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
				if(*bi != control)
					totalRemovable += (*bi)->value();
			if(m_Capacitor != control)
				totalRemovable += m_Capacitor->value();
			if(m_Cryo != control)
				totalRemovable += m_Cryo->value();
			for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
				if(*fi != control)
					totalRemovable += (*fi)->value();
			if(m_Fuel != control)
				totalRemovable += m_Fuel->value();
			if(m_Hold != control)
				totalRemovable += m_Hold->value();
			if(m_LifeSupport != control)
				totalRemovable += m_LifeSupport->value();
			if(m_ManeuverDrive != control)
				totalRemovable += m_ManeuverDrive->value();
			if(m_PowerPlant != control)
				totalRemovable += m_PowerPlant->value();
			if(m_Sensors && m_Sensors != control)
				totalRemovable += m_Sensors->value();
			if(m_Shields && m_Shields != control)
				totalRemovable += m_Shields->value();
			if(m_Skeleton != control)
				totalRemovable += m_Skeleton->value();

			// Remove each control's share of the total that can be removed.
			int removed = 0;
			if(removed < remove && m_Armor != control && m_Armor->value())
			{
				int v = m_Armor->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Armor->setValue(v - r);
				removed += r;
			}
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); removed < remove && bi != m_Bays.cend(); bi++)
			{
				if(*bi == control)
					continue;

				int v = (*bi)->value();
				if(!v)
					continue;

				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				(*bi)->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Capacitor != control && m_Capacitor->value())
			{
				int v = m_Capacitor->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Capacitor->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Cryo != control && m_Cryo->value())
			{
				int v = m_Cryo->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Cryo->setValue(v - r);
				removed += r;
			}
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_FTLDrives.cbegin();
					removed < remove && bi != m_FTLDrives.cend(); bi++)
			{
				if(*bi == control)
					continue;

				int v = (*bi)->value();
				if(!v)
					continue;

				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				(*bi)->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Fuel != control && m_Fuel->value())
			{
				int v = m_Fuel->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Fuel->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Hold != control && m_Hold->value())
			{
				int v = m_Hold->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Hold->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_LifeSupport != control && m_LifeSupport->value())
			{
				int v = m_LifeSupport->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_LifeSupport->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_ManeuverDrive != control && m_ManeuverDrive->value())
			{
				int v = m_ManeuverDrive->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_ManeuverDrive->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_PowerPlant != control && m_PowerPlant->value())
			{
				int v = m_PowerPlant->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_PowerPlant->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Sensors && m_Sensors != control && m_Sensors->value())
			{
				int v = m_Sensors->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Sensors->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Shields && m_Shields != control && m_Shields->value())
			{
				int v = m_Shields->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Shields->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Skeleton != control && m_Skeleton->value())
			{
				int v = m_Skeleton->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Skeleton->setValue(v - r);
				removed += r;
			}

			if(removed < remove)
				qWarning() << "error: AuScDesignProps::LimitVolumeSliders removed" << removed << "remove" << remove;
		}
		else
		{
			// Removing a small amount.
			// Adjust sliders by taking one off the highest slider that is not the control slider, until we are in limits.
			while(cursum > m_DesignRec.HullVolume())
			{
				// Take one off the highest slider, except the 'control' slider.
				QSlider* highest = 0;

				if(m_Armor != control && (!highest || m_Armor->value() > highest->value()))
					highest = m_Armor;

				for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
					if(*bi != control && (!highest || (*bi)->value() > highest->value()))
						highest = *bi;

				if(m_Capacitor != control && (!highest || m_Capacitor->value() > highest->value()))
					highest = m_Capacitor;

				if(m_Cryo != control && (!highest || m_Cryo->value() > highest->value()))
					highest = m_Cryo;

				for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
					if(*fi != control && (!highest || (*fi)->value() > highest->value()))
						highest = *fi;

				if(m_Fuel != control && (!highest || m_Fuel->value() > highest->value()))
					highest = m_Fuel;

				if(m_Hold != control && (!highest || m_Hold->value() > highest->value()))
					highest = m_Hold;

				if(m_LifeSupport != control && (!highest || m_LifeSupport->value() > highest->value()))
					highest = m_LifeSupport;

				if(m_ManeuverDrive != control && (!highest || m_ManeuverDrive->value() > highest->value()))
					highest = m_ManeuverDrive;

				if(m_PowerPlant != control && (!highest || m_PowerPlant->value() > highest->value()))
					highest = m_PowerPlant;

				if(m_Sensors && m_Sensors != control && (!highest || m_Sensors->value() > highest->value()))
					highest = m_Sensors;

				if(m_Shields && m_Shields != control && (!highest || m_Shields->value() > highest->value()))
					highest = m_Shields;

				if(m_Skeleton != control && (!highest || m_Skeleton->value() > highest->value()))
					highest = m_Skeleton;

				if(!highest)
					break;	// wtf?

				highest->setValue(highest->value()-1);
				cursum--;
			}
		}

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
		m_DesignRec.SetArmorVolume(m_Armor->value());
		for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
			m_DesignRec.SetBayVolume(bi.key(), (*bi)->value());
		m_DesignRec.SetCapacitorVolume(m_Capacitor->value());
		m_DesignRec.SetCryoBerthVolume(m_Cryo->value());
		for(QHash<quint16, AuSliderValue*>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
			m_DesignRec.SetFTLDriveVolume(fi.key(), (*fi)->value());
		m_DesignRec.SetFuelVolume(m_Fuel->value());
		m_DesignRec.SetHoldVolume(m_Hold->value());
		m_DesignRec.SetLifeSupportVolume(m_LifeSupport->value());
		m_DesignRec.SetManeuverDriveVolume(m_ManeuverDrive->value());
		m_DesignRec.SetPowerPlantVolume(m_PowerPlant->value());
		if(m_Sensors)
			m_DesignRec.SetSensorVolume(m_Sensors->value());
		if(m_Shields)
			m_DesignRec.SetShieldVolume(m_Shields->value());
		m_DesignRec.SetSkeletonVolume(m_Skeleton->value());

		// Update text values.
		m_ArmorLb->setText(QString("%L1m%2").arg(m_DesignRec.ArmorVolume()).arg(g_Au.CubedSymbol()));
		for(QHash<quint16, AuLineEditValue*>::iterator li = m_BayLbs.begin(); li != m_BayLbs.end(); li++)
			(*li)->setText(QString("%L1m%2").arg(m_DesignRec.BayVolume(li.key())).arg(g_Au.CubedSymbol()));
		m_CapacitorLb->setText(QString("%L1m%2").arg(m_DesignRec.CapacitorVolume()).arg(g_Au.CubedSymbol()));
		m_CryoLb->setText(QString("%L1m%2").arg(m_DesignRec.CryoBerthVolume()).arg(g_Au.CubedSymbol()));
		for(QHash<quint16, AuLineEditValue*>::iterator li = m_FTLDriveLbs.begin(); li != m_FTLDriveLbs.end(); li++)
			(*li)->setText(QString("%L1m%2").arg(m_DesignRec.FTLDriveVolume(li.key())).arg(g_Au.CubedSymbol()));
		m_FuelLb->setText(QString("%L1m%2").arg(m_DesignRec.FuelVolume()).arg(g_Au.CubedSymbol()));
		m_HoldLb->setText(QString("%L1m%2").arg(m_DesignRec.HoldVolume()).arg(g_Au.CubedSymbol()));
		m_LifeSupportLb->setText(QString("%L1m%2").arg(m_DesignRec.LifeSupportVolume()).arg(g_Au.CubedSymbol()));
		m_ManeuverDriveLb->setText(QString("%L1m%2").arg(m_DesignRec.ManeuverDriveVolume()).arg(g_Au.CubedSymbol()));
		m_PowerPlantLb->setText(QString("%L1m%2").arg(m_DesignRec.PowerPlantVolume()).arg(g_Au.CubedSymbol()));
		if(m_SensorsLb)
			m_SensorsLb->setText(QString("%L1m%2").arg(m_DesignRec.SensorVolume()).arg(g_Au.CubedSymbol()));
		if(m_ShieldsLb)
			m_ShieldsLb->setText(QString("%L1m%2").arg(m_DesignRec.ShieldVolume()).arg(g_Au.CubedSymbol()));
		m_SkeletonLb->setText(QString("%L1m%2").arg(m_DesignRec.SkeletonVolume()).arg(g_Au.CubedSymbol()));
	}
	else if(cursum < m_DesignRec.HullVolume())
	{
		// Adjust skeleton to use all unallocated hull volume.
		int fuel = m_Skeleton->value() + (m_DesignRec.HullVolume() - cursum);
		m_Skeleton->blockSignals(true);
		m_Skeleton->setValue(fuel);
		m_Skeleton->blockSignals(false);
		m_DesignRec.SetSkeletonVolume(m_Skeleton->value());
		m_SkeletonLb->setText(QString("%L1m%2").arg(m_DesignRec.SkeletonVolume()).arg(g_Au.CubedSymbol()));
	}

	UpdateReport();
}

quint32 AuScDesignProps::VolumeValue(const QString& s) const
{
	QString s1(s);
	s1.replace("+", "");
	s1.replace("-", "");
	s1.replace(".", "");
	s1.replace(",", "");
	s1.replace("m", "");
	s1.replace(g_Au.CubedSymbol(), "");
	quint32 vol = s1.toUInt();
	if(vol > m_DesignRec.HullVolume())
		vol = m_DesignRec.HullVolume();
	return vol;
}

void AuScDesignProps::ArmorEdited()
{
	quint32 value = VolumeValue(m_ArmorLb->text());
	m_Armor->setValue(value);
}

void AuScDesignProps::BayEdited(int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_Bays.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_BayLbs.find(partId);
	if(bi == m_Bays.cend() || li == m_BayLbs.cend())
		return;

	quint32 value = VolumeValue((*li)->text());
	(*bi)->setValue(value);
}

void AuScDesignProps::CapacitorEdited()
{
	quint32 value = VolumeValue(m_CapacitorLb->text());
	m_Capacitor->setValue(value);
}

void AuScDesignProps::CryoEdited()
{
	quint32 value = VolumeValue(m_CryoLb->text());
	m_Cryo->setValue(value);
}

void AuScDesignProps::FTLDriveEdited(int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_FTLDrives.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_FTLDriveLbs.find(partId);
	if(bi == m_FTLDrives.cend() || li == m_FTLDriveLbs.cend())
		return;

	quint32 value = VolumeValue((*li)->text());
	(*bi)->setValue(value);
}

void AuScDesignProps::FuelEdited()
{
	quint32 value = VolumeValue(m_FuelLb->text());
	m_Fuel->setValue(value);
}

void AuScDesignProps::SkeletonEdited()
{
	quint32 value = VolumeValue(m_SkeletonLb->text());
	m_Skeleton->setValue(value);
}

void AuScDesignProps::HoldEdited()
{
	quint32 value = VolumeValue(m_HoldLb->text());
	m_Hold->setValue(value);
}

void AuScDesignProps::LifeSupportEdited()
{
	quint32 value = VolumeValue(m_LifeSupportLb->text());
	m_LifeSupport->setValue(value);
}

void AuScDesignProps::ManeuverDriveEdited()
{
	quint32 value = VolumeValue(m_ManeuverDriveLb->text());
	m_ManeuverDrive->setValue(value);
}

void AuScDesignProps::PowerPlantEdited()
{
	quint32 value = VolumeValue(m_PowerPlantLb->text());
	m_PowerPlant->setValue(value);
}

void AuScDesignProps::SensorsEdited()
{
	quint32 value = VolumeValue(m_SensorsLb->text());
	m_Sensors->setValue(value);
}

void AuScDesignProps::ShieldsEdited()
{
	quint32 value = VolumeValue(m_ShieldsLb->text());
	m_Shields->setValue(value);
}

