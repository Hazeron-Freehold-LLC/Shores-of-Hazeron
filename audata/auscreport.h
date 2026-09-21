//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscreport.h,v 1.112 2022/02/19 18:19:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScReport_H
#define AuScReport_H

#include "audata.h"
#include "auscreporthelpers.h"
#include "demanufacturingprocess.h"

class AuScParts;

class AUDATA_DLL AuScReport
{
friend class AuScDesign;
	// Design
	AuId								m_ArchitectId;			// The architect of the design.
	QString								m_ArchitectName;
	QString								m_DesignDesc;
	quint32								m_DesignId;
	QString								m_DesignName;
	AuPartArchitectHash					m_PartArchitects;		// Architects of parts that differ from the architect of the design.

	// Structure
	quint8								m_LandingOrient;		// Preferred landing orientation for ship designs. LANDING_ORIENT
	quint8								m_StationOrient;		// Preferred orbit orientation for station designs.
	quint8								m_Streamline;			// STREAMLINE enum
	quint8								m_Structure;			// SPACECRAFT_STRUCTURE enum

	// Volumes, in cubic meters.
	AuScReportSysVol					m_Hull;					// Overall hull volume.
	AuPoint								m_HullMax;
	AuPoint								m_HullMin;
	quint32								m_ArmorVolume;
	AuScReportSysVolHash				m_Bays;
	AuScReportSysVol					m_Capacitor;
	quint32								m_CryoBerthVolume;
	AuScReportSysVolHash				m_FTLDrives;
	quint32								m_FuelVolume;
	quint32								m_HoldVolume;
	AuScReportSysVol					m_LifeSupport;
	AuScReportSysVol					m_ManeuverDrive;
	AuScReportSysVol					m_PowerPlant;
	AuScReportSysVol					m_Sensor;
	AuScReportSysVol					m_Shield;
	quint32								m_SkeletonVolume;		// Structural components, walls, floors, ceilings, etc.
	AuScReportTurretHash				m_Turrets;

	// Quality
	quint8								m_Quality;				// Overall quality.
	quint16								m_CnCModule;			// COMMODITYID of module.
	quint8								m_CnCQ;					// Due to CnC module.
	DeManufacturingProcess				m_MfgProc;				// Only valid when finalized as a spacecraft, not an assembly.

	// Stations and Equipment
	AuScPartSet							m_CitizenPosts;
	AuScReportStationHash				m_CommandStations;
	AuScReportStationHash				m_CrewStations;
	AuScReportStationHash				m_DesignerStations;
	AuScReportGameTableHash				m_GameTables;
	AuScPartSet							m_GuardPosts;
	AuScReportLiftHash					m_Lifts;				// Key is lift id, value is the stops.
	AuScPartSet							m_LivestockPosts;
	AuScReportServicePanelHash			m_ServicePanels;
	AuScReportStatusDisplayHash			m_StatusDisplays;
	AuScReportTransporterStationHash	m_Transporters;
	AuScPartSet							m_VehicleLandingSystems;
	AuScPartSet							m_VehicleLaunchers;

	// Berths
	quint16								m_CaptBerths;
	quint16								m_CrewBerths;
	quint16								m_OfficerBerths;
	quint16								m_PassengerBerths;
	quint16								m_TroopBerths;

	// Parking
	quint16								m_ParkGround;
	quint16								m_ParkRocket;
	quint16								m_ParkSpace;
	quint16								m_ParkSpacecraft;
	quint16								m_ParkWater;

	// Mass
	double								m_MassKg;				// Overall mass
	double								m_MassEquipmentKg;
	double								m_MassFuelKg;
	double								m_MassHullKg;

	// Analysis
	double								m_Acceleration;			// in feet/sec/sec.
	double								m_AccelerationVert;		// in feet/sec/sec.
	double								m_CapacitorMax;			// In power units.
	quint32								m_FaceCount;
	double								m_FTLDriveUnitsToEngage;	// Cubic feet of drive units needed to engage FTL.
	double								m_FuelBurnRate;			// Cubic feet of fuel per second, which is also unit of h2 per sec.
	quint32								m_FuelCapacity;			// In commodity units which is considered cubic feet of hydrogen.
	bool								m_HasBar;				// True if any room has HOLD_Bar hold access.
	bool								m_HasPantry;			// True if any room has HOLD_Pantry hold access.
	quint32								m_HoldCapacity;			// In commodity units.
	double								m_HoldPowerNoShields;	// Power units consumed per second with shields down.
	double								m_HoldPowerWithShields;	// Power units consumed per second with shields up.
	double								m_IdlePowerNoShields;	// Power units consumed per second with shields down.
	double								m_IdlePowerWithShields;	// Power units consumed per second with shields up.
	double								m_LifeSupportNeeded;	// In cubic meters.
	double								m_ManeuverFuelRate;		// In cubic feet of fuel per second.
	QString								m_PartsList;			// Only on finalized assemblies.
	double								m_SensorRange;			// In feet
	quint32								m_TriangleCount;
	double								m_TurnRate;				// in radians per second

public:	// Constructors
	AuScReport();
	virtual ~AuScReport();
	AuScReport(const AuScReport& c);

public:	// Operators
	const AuScReport& operator=(const AuScReport& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	bool operator==(const AuScReport& c) const;

	inline bool operator!=(const AuScReport& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline const AuId& ArchitectId() const
	{
		return m_ArchitectId;
	}

	inline const QString& ArchitectName() const
	{
		return m_ArchitectName;
	}

	double ArmorLevel() const;

	// Returns volume in cubic meters.
	inline quint32 ArmorVolume() const
	{
		return m_ArmorVolume;
	}

	// Returns highest Q of all bays.
	quint8 BayQ() const;

	// Returns total of all bays.
	quint32 BayVolume() const;

	// Returns volume in cubic meters.
	inline quint32 BayVolume(quint16 partId) const
	{
		AuScReportSysVolHash::const_iterator bi = m_Bays.find(partId);
		if(bi == m_Bays.cend())
			return 0;
		return (*bi).Volume();
	}

	// True if the manufacturing process has been initialized.
	bool CanBeManufactured() const;

	// SYS_Capacitor
	quint32 CapacitorHits() const;

	inline double CapacitorMax() const
	{
		return m_CapacitorMax;
	}

	COMMODITYID CapacitorModule() const;

	inline quint8 CapacitorQ() const
	{
		return m_Capacitor.Quality();
	}

	// Converts volume into equivalent number of equipment units in the old system.
	double CapacitorUnits() const;

	// Returns volume in cubic meters.
	inline quint32 CapacitorVolume() const
	{
		return m_Capacitor.Volume();
	}

	inline quint16 CaptBerths() const
	{
		return m_CaptBerths;
	}

	inline const AuScPartSet& CitizenPosts() const
	{
		return m_CitizenPosts;
	}

	quint16 ClosestRemoteDoor(bool forVehicle, const AuPoint& loc, const AuDoorStateHash& doorStates) const;

	// Returns the transporter console part id.
	quint16 ClosestTransporterInRoom(const AuScDesign& scd, quint16 roomId, const AuPoint& loc) const;

	inline COMMODITYID CommandAndControlModule() const
	{
		return (COMMODITYID)m_CnCModule;
	}

	inline quint16 CommandStationCount() const
	{
		return m_CommandStations.size();
	}

	QString CommandStationName(quint16 partId) const;

	inline const AuScReportStationHash& CommandStations() const
	{
		return m_CommandStations;
	}

	static unsigned int CompressedSizeLimit();

	static unsigned int CompressedSizeLimitPerc();

	inline quint16 CrewBerths() const
	{
		return m_CrewBerths;
	}

	inline quint16 CrewStationCount() const
	{
		return m_CrewStations.size() + m_Transporters.size();
	}

	QString CrewStationName(quint16 partId) const;

	inline const AuScReportStationHash& CrewStations() const
	{
		return m_CrewStations;
	}

	// SYS_LifeSupport
	quint32 CryoBerthHits() const;

	// One cryo berth per four cubic meters of cryo berths.
	inline quint32 CryoBerths() const
	{
		return m_CryoBerthVolume/4;
	}

	// Returns volume in cubic meters.
	inline quint32 CryoBerthVolume() const
	{
		return m_CryoBerthVolume;
	}

	// Returns density in pounds per cubic foot (lbs / ft^3) of the hull and all of its equipment.
	// Density is increased by armor and streamlining.
	float Density() const;

	// Returns a brief descriptive line of text.
	QString DescLine() const;

	// This is the design description stored on the design by the architect.
	inline const QString& DesignDescription() const
	{
		return m_DesignDesc;
	}

	inline const AuScReportStationHash& DesignerStations() const
	{
		return m_DesignerStations;
	}

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline const QString& DesignName() const
	{
		return m_DesignName;
	}

	// Reduce system modules appropriately when repaired using parts of quality 'repairQ'.
	// 'systemsRepaired' contains the systems ids that received parts.
	// Returns true if any system module quality is changed.
	bool DowngradeSystemModules(quint8 repairQ, const QSet<AuSysId>& systemsRepaired);

	bool EnginesRequireEngineer() const;

	// A "unit" is 256 cu ft.
	static float EquipmentHitPointsPerUnit();

	// Returns hit points for a given volume of equipment in cubic meters, adjusted for quality.
	static quint32 EquipmentVolumeHits(quint32 volumeCuM);

	double EstimatedCost() const;

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		max = m_HullMax;
		min = m_HullMin;
	}

	inline quint32 FaceCount() const
	{
		return m_FaceCount;
	}

	// SYS_FTLDrive
	quint32 FTLDriveHits() const;

	quint32 FTLDriveHits(quint16 partId) const;

	double FTLDrivePowerRate(quint16 partId) const;

	inline const AuScReportSysVolHash& FTLDrives() const
	{
		return m_FTLDrives;
	}

	quint8 FTLDriveQ() const;

	quint8 FTLDriveQuality(quint16 partId) const;

	// Converts volume into equivalent number of equipment units in the old system.
	double FTLDriveUnits(quint16 partId) const;

	inline double FTLDriveUnitsToEngage() const
	{
		return m_FTLDriveUnitsToEngage / 256.;
	}

	inline double FTLDriveUnitsToEngageCuFt() const
	{
		return m_FTLDriveUnitsToEngage;
	}

	// Returns total of all FTL drives.
	quint32 FTLDriveVolume() const;

	// Returns volume in cubic meters.
	inline quint32 FTLDriveVolume(quint16 partId) const
	{
		AuScReportSysVolHash::const_iterator fi = m_FTLDrives.find(partId);
		if(fi == m_FTLDrives.cend())
			return 0;
		return (*fi).Volume();
	}

	// Returns part id of FTL drive with warp drive module installed. 0 if none.
	quint16 FTLWarpDrive() const;

	// Returns part id of FTL drive with wormhole drive module installed. 0 if none.
	quint16 FTLWormholeDrive() const;

	inline double FuelBurnRate() const
	{
		return m_FuelBurnRate;
	}

	// Return value is in commodity units which are also cubic feet of hydrogen.
	inline quint32 FuelCapacity() const
	{
		return m_FuelCapacity;
	}

	// Fuel hits are a portion of HullHits.
	// SYS_Fuel
	quint32 FuelHits() const;

	// Returns volume in cubic meters.
	inline quint32 FuelVolume() const
	{
		return m_FuelVolume;
	}

	inline const AuScReportGameTableHash& GameTables() const
	{
		return m_GameTables;
	}

	inline const AuScPartSet& GuardPosts() const
	{
		return m_GuardPosts;
	}

	float HarvesterBayRange(quint16 partId, unsigned int unitsFired) const;

	inline bool HasCapacitor() const
	{
		return m_Capacitor.Volume();
	}

	bool HasEngineerStation() const;

	inline bool HasFTLDrive() const
	{
		return !m_FTLDrives.empty();
	}

	inline bool HasFuel() const
	{
		return m_FuelVolume;
	}

	bool HasHelm() const;

	inline bool HasHold() const
	{
		return m_HoldVolume;
	}

	inline bool HasLifeSupport() const
	{
		return m_LifeSupport.Volume();
	}

	inline bool HasManeuverDrive() const
	{
		return m_ManeuverDrive.Volume();
	}

	inline bool HasPowerPlant() const
	{
		return m_PowerPlant.Volume();
	}

	inline bool HasSensor() const
	{
		return m_Sensor.Module() != CID_Invalid && m_Sensor.Volume();
	}

	inline bool HasShield() const
	{
		return m_Shield.Module() != CID_Invalid && m_Shield.Volume();
	}

	bool HasSurgeryUnit() const;

	inline bool HasTransporter() const
	{
		return !m_Transporters.empty();
	}

	inline bool HasTurret() const
	{
		return !m_Turrets.empty();
	}

	inline bool HasVehicleLauncher() const
	{
		return !m_VehicleLaunchers.empty();
	}

	inline bool HasVehicleLandingSystem() const
	{
		return !m_VehicleLandingSystems.empty();
	}

	inline bool HasWarpFTLDrive() const
	{
		for(AuScReportSysVolHash::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
			if(DbCommodity::IsWarpFTLDriveModule((*fi).Module()))
				return true;
		return false;
	}

	inline bool HasWeaponBay() const
	{
		return !m_Bays.empty();
	}

	inline bool HasWormholeFTLDrive() const
	{
		for(AuScReportSysVolHash::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
			if(DbCommodity::IsWormholeFTLDriveModule((*fi).Module()))
				return true;
		return false;
	}

	// In commodity units.
	inline quint32 HoldCapacity() const
	{
		return m_HoldCapacity;
	}

	// Hold hits are a portion of HullHits.
	// SYS_Hold
	quint32 HoldHits() const;

	// Returns volume in cubic meters.
	inline quint32 HoldVolume() const
	{
		return m_HoldVolume;
	}

	// SYS_Hull only. Does not include fuel, hold, or skeleton like HullHits does.
	quint32 HullSystemHits() const;

	// Hit points are increased by armor, streamlining, hull module, and hull quality.
	// A "unit" is 256 cu ft.
	float HullHitPointsPerUnit() const;

	COMMODITYID HullMaterial() const;

	COMMODITYID HullModule() const;

	inline quint8 HullQ() const
	{
		return m_Hull.Quality();
	}

	// In cubic meters.
	inline quint32 HullVolume() const
	{
		return m_Hull.Volume();
	}

	void HullVolumeDistribution(quint32& hullVolume, quint32& armor, quint32& capacitor, quint32& cryoBerths, quint32& fuel,
			quint32& hold, quint32& lifeSupport, quint32& maneuverDrive, quint32& powerPlant, quint32& sensor, bool& hasSensor,
			quint32& shield, bool& hasShield, QMap<quint16, AuScReportSysVol>& ftlDrives,
			QMap<quint16, AuScReportSysVol>& bays, quint32& skeleton) const;

	// Returns hit points for a given volume of hull in cubic meters, adjusted for quality, streamlining and armor level.
	quint32 HullVolumeHits(quint32 volumeCuM) const;

	inline double IdlePowerWithShields() const
	{
		return m_IdlePowerWithShields;
	}

	inline bool IsFirstClass() const	// berthing accomodations, false if none
	{
		if(!m_PassengerBerths)
			return false;				// Passenger berths needed.
		if(!m_HasBar)
			return false;				// Access to food and liquor needed for first class.
		return true;
	}

	inline bool IsSecondClass() const	// berthing accomodations, false if none
	{
		if(!m_PassengerBerths)
			return false;				// Passenger berths needed.
		if(!m_HasPantry && !m_HasBar)
			return false;				// Access to food but not liquor needed for second class.
		return true;
	}

	bool IsSpaceStation() const;

	inline LANDING_ORIENT LandingOrientation() const
	{
		return (LANDING_ORIENT)m_LandingOrient;
	}

	COMMODITYID LifeSupportModule() const;

	// Includes CryoBerthHits.
	quint32 LifeSupportHits() const;

	// Returns the amount of life support needed before the system is adequate, in cubic meters. 0 if it's ok.
	inline double LifeSupportNeeded() const
	{
		return m_LifeSupportNeeded;
	}

	// Returns life support power use rate in units per real world second.
	double LifeSupportPowerRate() const;

	inline quint8 LifeSupportQ() const
	{
		return m_LifeSupport.Quality();
	}

	// Converts volume into equivalent number of equipment units in the old system.
	double LifeSupportUnits() const;

	// Returns volume in cubic meters.
	inline quint32 LifeSupportVolume() const
	{
		return m_LifeSupport.Volume();
	}

	void ListComponents(DeProcessComponentArray& pc, const AuScParts& parts) const;

	inline const AuScPartSet& LivestockPosts() const
	{
		return m_LivestockPosts;
	}

	inline double ManeuverAcceleration() const
	{
		return m_Acceleration;
	}

	inline double ManeuverAccelerationVertical() const
	{
		return m_AccelerationVert;
	}

	// SYS_ManeuverDrive
	quint32 ManeuverDriveHits() const;

	COMMODITYID ManeuverDriveModule() const;

	inline quint8 ManeuverDriveQ() const
	{
		return m_ManeuverDrive.Quality();
	}

	// Converts volume into equivalent number of equipment units in the old system.
	double ManeuverDriveUnits() const;

	// Returns volume in cubic meters.
	inline quint32 ManeuverDriveVolume() const
	{
		return m_ManeuverDrive.Volume();
	}

	// Returns maneuver drive fuel use rate in units per real world second.
	// Only rocket drives use fuel and will return 0.
	double ManeuverFuelRate() const;

	// Returns maneuver drive power use rate in units per real world second.
	// Rocket drives do not use power and will return 0.
	double ManeuverPowerRate() const;

	// Rocket drives do not use power.
	inline double ManeuverPowerRateHolding() const
	{
		return ManeuverPowerRate() / 10.;	// Maneuver drive consumes 10% of the power when holding.
	}

	// Only valid when finalized as a spacecraft, not an assembly.
	inline const DeManufacturingProcess& ManufacturingProcess() const
	{
		return m_MfgProc;
	}

	inline float MassKg() const
	{
		return m_MassKg;
	}

	inline float MassLbs() const
	{
		return m_MassKg * PoundsPerKilogram();
	}

	quint8 MaxCapacitorFTLSetting(quint16 whichFTL) const;

	// Adjusted for streamlining.
	// Starts at 256 and goes up with increasing levels of streamlining.
	float MaxDrag() const;

	quint64 MemoryUsage() const;

	// Called after a system module has been installed to keep design report in sync.
	void ModuleInstalled(quint16 partId, COMMODITYID moduleCID, quint8 moduleQ);

	// Returns true if dimensions or structure (station) require this design to be built in space.
	bool MustBeBuiltInSpace() const;

	// Total hit points of non-structural systems at 100% health.
	inline unsigned int NonStructureHits() const
	{
		// NOTE: If you change the total hits sum calculation, be sure to
		//       update SelectSystem so it calculates the correct system
		//       distribution.
		//       Also update AuSpacecraftDamage::HitsRemaining.
		// Game tables don't contribute to hit points because we don't damage them.
		// These volumes are allocated from hull volume but they are not structural. This defines a volume of equipment that fills a
		// volume of the hull. The hit points of that equipment are calculated here.
		return
			(CommandStationCount() + CrewStationCount()) +		// SYS_CommandAndControl
			LifeSupportHits() +			// SYS_LifeSupport
			ManeuverDriveHits() +		// SYS_ManeuverDrive
			PowerPlantHits() +			// SYS_PowerPlant
			SensorHits() +				// SYS_Sensor
			ShieldHits() +				// SYS_Shield
			SurgeryUnitCount() +		// SYS_SurgeryUnit
			TransporterHits() +			// SYS_Transporter
			Turrets().size() +			// SYS_Turret
			VehicleLandingHits() +		// SYS_VehicleLanding
			VehicleLauncherHits() +		// SYS_VehicleLauncher
			WeaponBayHits() +			// SYS_Weapon
			FTLDriveHits() +			// SYS_FTLDrive
			CapacitorHits();			// SYS_Capacitor
	}

	inline quint16 OfficerBerths() const
	{
		return m_OfficerBerths;
	}

	// Returns true if the design is valid and ready to be used as an assembly. The only real requirement is a name and a part.
	// 'msgs' is filled with warnings and errors reported during the check, with html coloring.
	bool OkForAssembly(QStringList& msgs, const AuScDesign& design) const;

	// Returns true if the design is valid and can be manufactured as a game object.
	// 'msgs' is filled with warnings and errors reported during the check, with html coloring.
	// 'design' is needed for a true analysis. The ReportAnalysis function calls this with a null design, for warnings and errors.
	bool OkToManufacture(QStringList& msgs, const AuScDesign* design) const;

	inline quint16 ParkingSpotsGround() const
	{
		return m_ParkGround;
	}

	inline quint16 ParkingSpotsRocket() const
	{
		return m_ParkRocket;
	}

	// Literally space vehicle spot count, not including rockets.
	inline quint16 ParkingSpotsSpace() const
	{
		return m_ParkSpace;
	}

	// Space ships.
	inline quint16 ParkingSpotsSpacecraft() const
	{
		return m_ParkSpacecraft;
	}

	inline quint16 ParkingSpotsTotal() const
	{
		return m_ParkGround + m_ParkRocket + m_ParkSpace + m_ParkWater;	// should not include spacecraft + m_ParkSpacecraft;
	}

	inline quint16 ParkingSpotsWater() const
	{
		return m_ParkWater;
	}

	inline quint16 PassengerBerths() const
	{
		return m_PassengerBerths;
	}

	// Quantity of power needed to activate the FTL drive one time.
	// Returns -1 if the FTL has an unrecognized module or there is not FTL drive with that id.
	double PowerNeededToEngageFTLDrive(quint16 whichFTL, quint8 driveSetting) const;

	// Power units produced per cubic foot of fuel burned.
	static inline double PowerPerFuel()
	{
		return 10.;
	}

	quint32 PowerPlantHits() const;

	COMMODITYID PowerPlantModule() const;

	inline quint8 PowerPlantQ() const
	{
		return m_PowerPlant.Quality();
	}

	// Converts volume into equivalent number of equipment units in the old system.
	double PowerPlantUnits() const;

	// Returns volume in cubic meters.
	inline quint32 PowerPlantVolume() const
	{
		return m_PowerPlant.Volume();
	}

	// 'volume' is in cubic meters.
	void RefitArmorVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitBayVolume(quint16 partId, quint32 volume, quint8 minQ);

	// Returns true if any of them change.
	bool RefitBayVolumes(const QMap<quint16, quint32>& bays, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitCapacitorVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitCryoBerthVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitFTLDriveVolume(quint16 partId, quint32 volume, quint8 minQ);

	// Returns true if any of them change.
	bool RefitFTLDriveVolumes(const QMap<quint16, quint32>& ftlDrives, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitFuelVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitHoldVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitLifeSupportVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitManeuverDriveVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitPowerPlantVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitSensorVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitShieldVolume(quint32 volume, quint8 minQ);

	// 'volume' is in cubic meters.
	void RefitSkeletonVolume(quint32 volume, quint8 minQ);

	// Returns the full detailed design analysis report with html formatting.
	// 'designState' is the AuDesign::STATE of the design being reported.
	QString ReportAnalysis(quint8 designState) const;

	// Returns a minimal description for use on tool tips.
	QString ReportToolTip() const;

	// Returns a radius that encompasses all of my points, measured from my own origin.
	double RoughHullRadius() const;

	quint32 SensorHits() const;

	COMMODITYID SensorModule() const;

	// Returns sensor power use rate in units per real world second.
	double SensorPowerRate() const;

	// Returns sensor range in feet.
	inline double SensorRange() const
	{
		return m_SensorRange;
	}

	// Returns deep space sensor range in pc. This is the range that solar system emissions can be detected.
	double SensorRangeDeepSpace() const;

	inline quint8 SensorQ() const
	{
		return m_Sensor.Quality();
	}

	// Converts volume into equivalent number of equipment units in the old system.
	double SensorUnits() const;

	// Returns volume in cubic meters.
	inline quint32 SensorVolume() const
	{
		return m_Sensor.Volume();
	}

	inline quint16 ServicePanelCount() const
	{
		return m_ServicePanels.size();
	}

	QString ServicePanelName(quint16 partId) const;

	inline const AuScReportServicePanelHash& ServicePanels() const
	{
		return m_ServicePanels;
	}

	// 'volume' is in cubic meters.
	void SetArmorVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetBayVolume(quint16 partId, quint32 volume);

	// Returns true if any of them change.
	bool SetBayVolumes(const QMap<quint16, quint32>& bays);

	// 'volume' is in cubic meters.
	void SetCapacitorVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetCryoBerthVolume(quint32 volume);

	void SetFTLDriveQuality(quint16 partId, quint8 quality);

	// 'volume' is in cubic meters.
	void SetFTLDriveVolume(quint16 partId, quint32 volume);

	// Returns true if any of them change.
	bool SetFTLDriveVolumes(const QMap<quint16, quint32>& ftlDrives);

	// 'volume' is in cubic meters.
	void SetFuelVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetHoldVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetLifeSupportVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetManeuverDriveVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetPowerPlantVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetSensorVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetShieldVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetSkeletonVolume(quint32 volume);

	void SetSurgeryUnitQuality(quint16 partId, quint8 quality);

	// Returns true if any quality changes.
	bool SetSystemQuality(const AuSysId& sysId, quint8 quality);

	// Sets the quality in all systems. Called when this report is storing information on a live spacecraft, that has just been
	// initialized.
	// 'quality' is expected to be no less than the current m_Quality, which is the max of all system qualities.
	// 'patents' is used to limit systems that have a module installed to the available patent quality.
	void SetQuality(quint8 quality);

	void SetTransporterQuality(quint16 partId, quint8 quality);

	void SetTurretQuality(quint16 partId, quint8 quality);

	void SetWeaponBayQuality(quint16 partId, quint8 quality);

	double ShieldCapacitor() const;

	// Returns the number of energy units dissipated by an energy shield for every point of damage absorbed.
	// E.g. A Q0 shield uses 1/10 energy point per hit of damage deflected. In other words, every 10 units of damage deflected
	// dissipates 1 unit of shield energy.
	inline float ShieldDissipatePerDamagePoint() const
	{
		quint8 shieldQ = m_Shield.Quality();
		if(!shieldQ)
			return 1.f/10.f;

		const float tlBoost = QualityBoostF(shieldQ);
		return 1.f/(10.f * tlBoost);
	}

	quint32 ShieldHits() const;

	COMMODITYID ShieldModule() const;

	// Returns shield power use rate in units per real world second.
	double ShieldPowerRate() const;

	inline quint8 ShieldQ() const
	{
		return m_Shield.Quality();
	}

	// Converts volume into equivalent number of equipment units in the old system.
	double ShieldUnits() const;

	// Returns volume in cubic meters.
	inline quint32 ShieldVolume() const
	{
		return m_Shield.Volume();
	}

	static unsigned int SizeLimit();

	static unsigned int SizeLimitPerc();

	// Skeleton hits are a portion of HullHits.
	quint32 SkeletonHits() const;

	// Returns fraction of volume that is skeleton. 0..1.
	float SkeletonLevel() const;

	// Returns volume in cubic meters.
	inline quint32 SkeletonVolume() const
	{
		return m_SkeletonVolume;
	}

	inline quint8 StationOrient() const
	{
		return m_StationOrient;
	}

	inline quint16 StatusDisplayCount() const
	{
		return m_StatusDisplays.size();
	}

	QString StatusDisplayName(quint16 partId) const;

	inline const AuScReportStatusDisplayHash& StatusDisplays() const
	{
		return m_StatusDisplays;
	}

	inline STREAMLINE Streamline() const
	{
		return (STREAMLINE)m_Streamline;
	}

	inline SPACECRAFT_STRUCTURE Structure() const
	{
		return (SPACECRAFT_STRUCTURE)m_Structure;
	}

	// Structural hits account for all hit points of the hull volume.
	inline quint32 StructureHits() const
	{
		return HullHits();		// SYS_Hull + SYS_Fuel + SYS_Hold + skeleton
	}

	quint16 SurgeryUnitCount() const;

	QList<quint16> SurgeryUnitIds() const;

	// 'consoleId' is the part id of the medical console.
	COMMODITYID SurgeryUnitModule(quint16 consoleId) const;

	quint8 SurgeryUnitQuality(quint16 consoleId) const;

	// Since sysId which 0 is invalid, it returns sum of all subsystems for systems that can have multiples.
	quint32 SystemHits(const AuSysId& sysId) const;

	// Retrieves a list of all system ids.
	// 'structural' is true to get structural systems, false to get nonstructural system.
	void SystemIds(QSet<AuSysId>& sysIds, bool structural) const;

	quint32 SystemModuleCount(const AuSysId& sysId) const;

	QString SystemName(const AuSysId& sysId) const;

	QString SystemName(const AuSysId& sysId, COMMODITYID module) const;

	// Overall quality.
	inline quint8 Quality() const
	{
		return m_Quality;
	}

	// Total hit points at 100% health.
	inline quint32 TotalHits() const
	{
		// NOTE: If you change the total hits sum calculation, be sure to
		//       update SelectSystem so it calculates the correct system
		//       distribution.
		//       Also update AuSpacecraftDamage::HitsRemaining.
		// Game tables don't contribute to hit points because we don't damage them.
		return NonStructureHits() + StructureHits();
	}

	float TractorBeamRange(quint16 partId, unsigned int unitsFired) const;

	quint8 TransporterQuality(quint16 partId) const;

	// Hits due to transporter pads, not the console.
	quint32 TransporterHits() const;

	// Hits due to transporter pads, not the console.
	quint32 TransporterHits(quint16 partId) const;

	// 'partId' is a console or station.
	quint16 TransporterRoomId(quint16 partId) const;

	inline const AuScReportTransporterStationHash& Transporters() const
	{
		return m_Transporters;
	}

	inline quint32 TriangleCount() const
	{
		return m_TriangleCount;
	}

	inline quint16 TroopBerths() const
	{
		return m_TroopBerths;
	}

	// Key is turbo lift part id.
	inline const AuScReportLiftHash& TurboLifts() const
	{
		return m_Lifts;
	}

	// Returns the door id at the 'stopName' of the turbo lift 'liftId'.
	quint16 TurboLiftStopDoor(quint16 liftId, const QString& stopName) const;

	// in radians per second
	inline double TurnRate() const
	{
		return m_TurnRate;
	}

	quint8 TurretQuality(quint16 partId) const;

	double TurretRange(quint16 partId) const;

	inline const AuScReportTurretHash& Turrets() const
	{
		return m_Turrets;
	}

	// Updates stored values that are calculated based on system volumes.
	void UpdateCalculatedValues();

	// Returns the total number of 'spotType' parking spots.
	quint16 VehicleComplement(VEHICLE_SPOT_TYPE spotType) const;

	// Hits due to vehicle landing systems.
	// SYS_VehicleLanding
	quint32 VehicleLandingHits() const;

	inline const AuScPartSet& VehicleLandingSystems() const
	{
		return m_VehicleLandingSystems;
	}

	// Hits due to vehicle launchers.
	// SYS_VehicleLaunch
	quint32 VehicleLauncherHits() const;

	inline const AuScPartSet& VehicleLaunchers() const
	{
		return m_VehicleLaunchers;
	}

	quint32 WeaponBayHits() const;

	quint32 WeaponBayHits(quint16 partId) const;

	quint8 WeaponBayQuality(quint16 partId) const;

	float WeaponBayPower(quint16 partId, float unitsFired) const;

	float WeaponBayRange(quint16 partId) const;

	// Converts volume into equivalent number of equipment units in the old system.
	double WeaponBayUnits(quint16 partId) const;

	inline const AuScReportSysVolHash& WeaponBays() const
	{
		return m_Bays;
	}

protected:
	double CalcCapacitorMax() const;

	// Returns mass in pounds.
	double CalcEquipmentMass() const;

	// The formula for the volume of a sphere gives us the number of FTL drive units required.
	//  V = 4/3 * PI * r^3
	// where
	//  V = Volume, which is the volume of the ship hull in our calculation.
	//  r = Number of FTL drive units required.
	// reduced
	//  r = cuberoot(3/4 * V * PI)
	// Returns the number of cubic feet of FTL drive units needed to engage.
	double CalcFTLDriveUnitsToEngage() const;

	// Returns power plant fuel burn rate in cubic feet per real world second.
	// AuScDesign::PowerPerFuel returns the number of power units produced from each cubic foot of fuel that is burned.
	double CalcFuelBurnRate() const;

	// Return number of commodity units.
	quint32 CalcFuelCapacity() const;

	// Returns mass in pounds.
	double CalcFuelMass() const;

	// Return number of commodity units.
	quint32 CalcHoldCapacity() const;

	// Returns mass in pounds.
	double CalcHullMass() const;

	// Returns cubic feet of life support needed.
	double CalcLifeSupportNeeded() const;

	// Returns acceleration in ft/sec/sec.
	// 1000 ft/sec/sec is pretty good.
	// We'll loosely apply Newton's second law here, which is
	//  F = ma
	// where
	//  F = Force
	//  m = mass
	//  a = acceleration
	// In the case of our maneuver drives,
	//  F is the force exerted by the maneuver drives.
	//  m is one mass unit per cubic foot, modified by streamlining
	//    and armor.
	//  a is acceleration in ft/sec/sec.
	double CalcManeuverAcceleration() const;

	// Returns maneuver drive fuel use rate in units per real world second.
	// Only rocket drives use fuel.
	double CalcManeuverFuelRate() const;

	double CalcPowerUsageHolding(bool shieldsUp) const;

	double CalcPowerUsageIdling(bool shieldsUp) const;

	// Ranges of sensors in feet.
	double CalcSensorRange() const;

	double CalcTurnRate() const;

	// Hull hits are the entire structural hit points of the ship due to hull volume. Includes FuelHits, HoldHits and skeleton.
	quint32 HullHits() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScReport& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReport& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuScReport)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuScReport_H
