//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdreport.h,v 1.52 2022/02/19 18:18:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdReport_H
#define AuBdReport_H

#include "auscreporthelpers.h"
#include "dbindustry.h"
#include "deconstructionprocess.h"

class AuBdDesign;
class DbIndustry;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuBdReport
{
friend class AuBdDesign;
	// Design
	AuId								m_ArchitectId;			// The architect of the design.
	QString								m_ArchitectName;
	QString								m_DesignDesc;
	quint32								m_DesignId;
	QString								m_DesignName;
	AuPartArchitectHash					m_PartArchitects;		// Architects of parts that differ from the architect of the design.

	// Structure
	quint8								m_Structure;			// BUILDING_STRUCTURE enum
	quint8								m_IndustryId;			// INDUSTRYID enum of intended industry. Not a restriction.

	// Volumes, in cubic meters.
	AuScReportSysVol					m_Hull;					// Overall hull volume.
	AuPoint								m_HullMax;
	AuPoint								m_HullMin;
	AuPoint								m_SiteMax;
	AuPoint								m_SiteMin;
	quint32								m_ArenaVolume;
	quint32								m_ArmorVolume;
	quint32								m_AuditoriumVolume;
	AuScReportSysVolHash				m_Bays;
	AuScReportSysVol					m_Capacitor;
	quint32								m_FieldArea;			// In square meters.
	quint32								m_HoldVolume;
	quint32								m_HomeLargeVolume;
	quint32								m_HomeMediumVolume;
	quint32								m_HomeSmallVolume;
	AuScReportSysVol					m_LifeSupport;
	quint32								m_LoungeVolume;
	quint32								m_OfficeVolume;
	AuScReportSysVol					m_PowerPlant;
	AuScReportSysVol					m_Sensor;
	AuScReportSysVol					m_Shield;
	quint32								m_ShopVolume;
	quint32								m_StoreVolume;
	quint32								m_SkeletonVolume;		// Framework components, walls, floors, ceilings, etc.
	AuScReportTurretHash				m_Turrets;

	// Quality
	quint8								m_Quality;				// Overall quality.
	quint16								m_CnCModule;			// COMMODITYID of module.
	quint8								m_CnCQ;					// Due to CnC module.
	DeConstructionProcess				m_ConProc;				// Only valid when finalized as a building, not an assembly.
																// Material is CID_Lumber. Must change to hull material for use.

	// Stations and Equipment
	AuScPartSet							m_CitizenPosts;
	AuScReportStationHash				m_CommandStations;
	AuScReportStationHash				m_CrewStations;
	AuScReportGameTableHash				m_GameTables;
	AuScPartSet							m_GuardPosts;
	AuScReportLiftHash					m_Lifts;				// Key is lift id, value is the stops.
	AuScPartSet							m_LivestockPosts;
	AuScReportStatusDisplayHash			m_StatusDisplays;
	AuScReportTransporterStationHash	m_Transporters;
	AuScPartSet							m_VehicleLandingSystems;
	AuScPartSet							m_VehicleLaunchers;

	// Beds/berths/bunks.
	quint16								m_Beds;

	// Parking
	quint16								m_ParkGround;
	quint16								m_ParkRocket;
	quint16								m_ParkSpace;
	quint16								m_ParkSpacecraft;
	quint16								m_ParkWater;

	// Mass
	double								m_MassKg;				// Overall mass
	double								m_MassEquipmentKg;
	double								m_MassHullKg;

	// Analysis
	quint32								m_ArenaCapacity;		// In whole arenas.
	quint32								m_AuditoriumCapacity;	// In whole auditoriums.
	double								m_CapacitorMax;			// In power units.
	quint32								m_FaceCount;
	quint32								m_FieldCapacity;		// In whole fields.
	double								m_FuelBurnRate;			// Cubic feet of fuel per second, which is also unit of h2 per sec.
	bool								m_HasBar;				// True if any room has HOLD_Bar hold access.
	bool								m_HasEnvironmentVoid;	// True if the design has any environment voids.
	bool								m_HasPantry;			// True if any room has HOLD_Pantry hold access.
	quint32								m_HoldCapacity;			// In commodity units.
	quint32								m_HomeLargeCapacity;	// In whole homes.
	quint32								m_HomeMediumCapacity;	// In whole homes.
	quint32								m_HomeSmallCapacity;	// In whole homes.
	double								m_IdlePowerWithShields;	// Power units consumed per second with shields up.
	quint32								m_LoungeCapacity;		// In whole lounges.
	quint32								m_OfficeCapacity;		// In whole jobs.
	QString								m_PartsList;			// Only on finalized assemblies.
	double								m_SensorRange;			// In feet
	quint32								m_ShopCapacity;			// In whole shops.
	quint32								m_StoreCapacity;		// In whole stores.
	quint32								m_TriangleCount;

public:	// Constructors
	AuBdReport();
	virtual ~AuBdReport();
	AuBdReport(const AuBdReport& c);

public:	// Operators
	const AuBdReport& operator=(const AuBdReport& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	bool operator==(const AuBdReport& c) const;

	inline bool operator!=(const AuBdReport& c) const
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

	inline quint32 ArenaCapacity() const
	{
		return m_ArenaCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 ArenaVolume() const
	{
		return m_ArenaVolume;
	}

	double ArmorLevel() const;

	bool ArmorOkForFrigidLand() const;

	bool ArmorOkForFrigidSeaBottom() const;

	bool ArmorOkForGasGiantCore() const;

	bool ArmorOkForInfernoLand() const;

	// Not frigid or inferno.
	bool ArmorOkForSeaBottom() const;

	// Returns volume in cubic meters.
	inline quint32 ArmorVolume() const
	{
		return m_ArmorVolume;
	}

	inline quint32 AuditoriumCapacity() const
	{
		return m_AuditoriumCapacity;
	}

	inline quint32 AuditoriumVolume() const
	{
		return m_AuditoriumVolume;
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

	inline quint16 Beds() const
	{
		return m_Beds;
	}

	// True if the manufacturing process has been initialized.
	bool CanBeConstructed() const;

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

	// Returns jurisdiction radius if this building is used as a capitol. Not really valid if used as something else.
	double CapitolJurisdictionRadius() const;

	inline const AuScPartSet& CitizenPosts() const
	{
		return m_CitizenPosts;
	}

	quint16 ClosestRemoteDoor(bool forVehicle, const AuPoint& loc, const AuDoorStateHash& doorStates) const;

	// Returns the transporter console part id.
	quint16 ClosestTransporterInRoom(const AuBdDesign& scd, quint16 roomId, const AuPoint& loc) const;

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

	// Only valid when finalized as a building, not an assembly.
	// Note the construction material must be changed based on the hull module. This process will have CID_Lumber as the material.
	inline const DeConstructionProcess& ConstructionProcess() const
	{
		return m_ConProc;
	}

	// Returns true if this buildings could be occupied by workers or residents, depending on usage.
	bool CouldBeOccupied() const;

	inline quint16 CrewStationCount() const
	{
		return m_CrewStations.size() + m_Transporters.size();
	}

	QString CrewStationName(quint16 partId) const;

	inline const AuScReportStationHash& CrewStations() const
	{
		return m_CrewStations;
	}

	// Returns density in pounds per cubic foot (lbs / ft^3) of the hull and all of its equipment.
	// Density is increased by armor.
	float Density() const;

	// Returns a brief descriptive line of text.
	QString DescLine() const;

	// This is the design description stored on the design by the architect.
	inline const QString& DesignDescription() const
	{
		return m_DesignDesc;
	}

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline const QString& DesignName() const
	{
		return m_DesignName;
	}

	// Returns BUILDING_ENVIRONMENT bits OR'd together to indicate the environments in which this blueprint can be built.
	quint8 EnvironmentBits() const;

	// A "unit" is 256 cu ft.
	static float EquipmentHitPointsPerUnit();

	// Returns hit points for a given volume of equipment in cubic meters, adjusted for quality.
	static quint32 EquipmentVolumeHits(quint32 volumeCuM);

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		max = m_HullMax;
		min = m_HullMin;
	}

	inline quint32 FaceCount() const
	{
		return m_FaceCount;
	}

	// Returns area in square meters.
	inline quint32 FieldArea() const
	{
		return m_FieldArea;
	}

	inline quint32 FieldCapacity() const
	{
		return m_FieldCapacity;
	}

	inline double FuelBurnRate() const
	{
		return m_FuelBurnRate;
	}

	inline const AuScReportGameTableHash& GameTables() const
	{
		return m_GameTables;
	}

	quint32 GeneratorJobs() const;

	inline const AuScPartSet& GuardPosts() const
	{
		return m_GuardPosts;
	}

	float HarvesterBayRange(quint16 which, unsigned int unitsFired) const;

	inline bool HasEnvironmentVoid() const
	{
		return m_HasEnvironmentVoid;
	}

	inline bool HasField() const
	{
		return m_FieldCapacity;
	}

	inline bool HasArena() const
	{
		return m_ArenaCapacity;
	}

	inline bool HasAuditorium() const
	{
		return m_AuditoriumCapacity;
	}

	inline bool HasCapacitor() const
	{
		return m_Capacitor.Volume();
	}

	inline bool HasGenerator() const
	{
		return m_PowerPlant.Volume();
	}

	inline bool HasHold() const
	{
		return m_HoldVolume;
	}

	inline bool HasHomeLarge() const
	{
		return m_HomeLargeCapacity;
	}

	inline bool HasHomeMedium() const
	{
		return m_HomeMediumCapacity;
	}

	inline bool HasHomeSmall() const
	{
		return m_HomeSmallCapacity;
	}

	inline bool HasLounge() const
	{
		return m_LoungeCapacity;
	}

	inline bool HasLifeSupport() const
	{
		return m_LifeSupport.Volume();
	}

	inline bool HasOffice() const
	{
		return m_OfficeCapacity;
	}

	inline bool HasStoreSpace() const
	{
		return m_StoreCapacity;
	}

	inline bool HasSensor() const
	{
		return m_Sensor.Module() != CID_Invalid && m_Sensor.Volume();
	}

	inline bool HasShield() const
	{
		return m_Shield.Module() != CID_Invalid && m_Shield.Volume();
	}

	inline bool HasShop() const
	{
		return m_ShopCapacity;
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

	inline bool HasWeaponBay() const
	{
		return !m_Bays.empty();
	}

	// In commodity units.
	inline quint32 HoldCapacity() const
	{
		return m_HoldCapacity;
	}

	// Hold hits are a portion of HullHits.
	quint32 HoldHits() const;

	// Returns volume in cubic meters.
	inline quint32 HoldVolume() const
	{
		return m_HoldVolume;
	}

	// In homes for individual people.
	inline quint32 HomeLargeCapacity() const
	{
		return m_HomeLargeCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 HomeLargeVolume() const
	{
		return m_HomeLargeVolume;
	}

	// In homes for individual people.
	inline quint32 HomeMediumCapacity() const
	{
		return m_HomeMediumCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 HomeMediumVolume() const
	{
		return m_HomeMediumVolume;
	}

	// In homes for individual people.
	inline quint32 HomeSmallCapacity() const
	{
		return m_HomeSmallCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 HomeSmallVolume() const
	{
		return m_HomeSmallVolume;
	}

	// SYS_Hull only. Does not include hold or skeleton like HullHits does.
	quint32 HullSystemHits() const;

	// Hit points are increased by armor, hull module, and hull quality.
	// A "unit" is 256 cu ft.
	float HullHitPointsPerUnit() const;

	COMMODITYID HullMaterial() const;

	inline const AuPoint& HullMax() const
	{
		return m_HullMax;
	}

	inline const AuPoint& HullMin() const
	{
		return m_HullMin;
	}

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

	void HullVolumeDistribution(quint32& hullVolume, quint32& arena, quint32& armor, quint32& auditorium, quint32& capacitor,
			quint32& hold, quint32& homeLarge, quint32& homeMedium, quint32& homeSmall, quint32& office, quint32& lifeSupport,
			quint32& lounge, quint32& powerPlant, quint32& store, quint32& sensor, bool& hasSensor, quint32& shield,
			bool& hasShield, quint32& shop, QMap<quint16, AuScReportSysVol>& bays, quint32& skeleton) const;

	// Returns hit points for a given volume of hull in cubic meters, adjusted for quality and armor level.
	quint32 HullVolumeHits(quint32 volumeCuM) const;

	inline double IdlePowerWithShields() const
	{
		return m_IdlePowerWithShields;
	}

	// Fills 'industries' with the ids of all industries that can use this building.
	void Industries(QSet<INDUSTRYID>& industries) const;

	// Returns 0 if this building design can be used for a specific industry 'dbi'.
	// Returns non-zero failure code if this building design cannot be sued for a specific industry 'dbi'.
	// See .cpp file for list of failure codes.
	int IndustryCanUse(const DbIndustry& dbi) const;

	// Returns a string version of the result code returned by IndustryCanUse.
	static QString IndustryCanUseMessage(int resultCode);

	inline INDUSTRYID IndustryId() const
	{
		return (INDUSTRYID)m_IndustryId;
	}

	// Returns homes and jobs when this blueprint is used for a specific industry.
	// 'manufacturingJobs' are those that can actually run a manufacturing process to make something.
	// 'proJobs' are the professional jobs. For manufacturing buildings, this is 0. For other buildings, this is
	//   dependent on the needs of the building, e.g. auditoriums for churches, lounges for cantinas. No manufacturing building
	//   creates professional jobs also.
	// 'serviceJobs' are all other jobs created by the building, not including the manufacturing and professional jobs.
	// 'stores' are for sales to the public, set to store capacity if allowed at the industry.
	// 'lounges' are for relaxation by the public, set to lounge capacity if allowed at the industry.
	void IndustryInfo(const DbIndustry& dbi, quint32& homesLarge, quint32& homesMedium, quint32& homesSmall,
			quint32& manufacturingJobs, quint32& serviceJobs, quint32& stores, quint32& lounges, quint32& proJobs) const;

	// Returns true for all kinds of platform structures.
	inline bool IsPlatform() const
	{
		return Structure() == BD_AerialPlatform ||
			Structure() == BD_SeaPlatform ||
			Structure() == BD_GroundPlatform;
	}

	quint32 LifeSupportHits() const;

	COMMODITYID LifeSupportModule() const;

	bool LifeSupportOkForFrigidLand() const;

	bool LifeSupportOkForFrigidSeaBottom() const;

	bool LifeSupportOkForGasGiantCore() const;

	bool LifeSupportOkForInfernoLand() const;

	bool LifeSupportOkForSeaBottom() const;

	bool LifeSupportOkForVacuumLand() const;

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

	inline quint32 LoungeCapacity() const
	{
		return m_LoungeCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 LoungeVolume() const
	{
		return m_LoungeVolume;
	}

	inline float MassKg() const
	{
		return m_MassKg;
	}

	inline float MassLbs() const
	{
		return m_MassKg * PoundsPerKilogram();
	}

	static double MaxSensorRange();

	// Returns the id of the medical station in 'roomId', if any.
	// Returns 0 if none.
	quint16 MedicalStationInRoom(quint16 roomId) const;

	quint64 MemoryUsage() const;

	// Called after a system module has been installed to keep design report in sync.
	void ModuleInstalled(quint16 which, COMMODITYID moduleCID, quint8 moduleQ);

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
			PowerPlantHits() +			// SYS_PowerPlant
			SensorHits() +				// SYS_Sensor
			ShieldHits() +				// SYS_Shield
			SurgeryUnitCount() +		// SYS_SurgeryUnit
			TransporterHits() +			// SYS_Transporter
			Turrets().size() +			// SYS_Turret
			VehicleLandingHits() +		// SYS_VehicleLanding
			VehicleLauncherHits() +		// SYS_VehicleLauncher
			WeaponBayHits() +			// SYS_Weapon
			CapacitorHits();			// SYS_Capacitor
	}

	// In commodity units.
	inline quint32 OfficeCapacity() const
	{
		return m_OfficeCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 OfficeVolume() const
	{
		return m_OfficeVolume;
	}

	// Returns true if the design is valid and ready to be used as an assembly. The only real requirement is a name and a part.
	// 'msgs' is filled with warnings and errors reported during the check, with html coloring.
	bool OkForAssembly(QStringList& msgs, const AuBdDesign& design) const;

	// Returns true if the design is valid and can be manufactured as a game object.
	// 'msgs' is filled with warnings and errors reported during the check, with html coloring.
	// 'design' is needed for a true analysis. The ReportAnalysis function calls this with a null design, for warnings and errors.
	bool OkToConstruct(QStringList& msgs, const AuBdDesign* design) const;

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

	// Returns jurisdiction radius if this building is used as a police station. Not really valid if used as something else.
	double PoliceJurisdictionRadius() const;

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

	// Returns the full detailed design analysis report with html formatting.
	// 'designState' is the AuDesign::STATE of the design being reported.
	QString ReportAnalysis(quint8 designState) const;

	// Returns a minimal description for use on tool tips.
	QString ReportToolTip() const;

	// Returns a radius that encompasses all of my points, measured from my own origin.
	double RoughHullRadius() const;

	// Returns a radius that encompasses all of my site, measured from my own origin.
	double RoughSiteRadius() const;

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

	// 'volume' is in cubic meters.
	void SetArenaVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetArmorVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetAuditoriumVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetBayVolume(quint16 partId, quint32 volume);

	// Returns true if any of them change.
	bool SetBayVolumes(const QMap<quint16, quint32>& bays);

	// 'volume' is in cubic meters.
	void SetCapacitorVolume(quint32 volume);

	// Sets the hull module based on 'cid'. Changes the construction process.
	void SetConstructionMaterial(COMMODITYID cid);

	// 'volume' is in cubic meters.
	void SetHoldVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetLifeSupportVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetHomeLargeVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetHomeMediumVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetHomeSmallVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetLoungeVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetOfficeVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetPowerPlantVolume(quint32 volume);

	void SetStoreVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetSensorVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetShieldVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetShopVolume(quint32 volume);

	// 'volume' is in cubic meters.
	void SetSkeletonVolume(quint32 volume);

	// Sets the quality in all systems. Called when this report is storing information on a live spacecraft, that has just been
	// initialized.
	// 'quality' is expected to be no less than the current m_Quality, which is the max of all system qualities.
	void SetQuality(quint8 quality);

	// Returns the radius of the visible shield bubble in feet.
	double ShieldBubbleRadius() const;

	// Shield capacitor stores 10 units of power per shield unit.
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

	// In commodity units.
	inline quint32 ShopCapacity() const
	{
		return m_ShopCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 ShopVolume() const
	{
		return m_ShopVolume;
	}

	inline AuPoint SiteCenter() const
	{
		return (SiteMin() + SiteMax()) / 2;
	}

	// Returns my site locations that are used to verify placement within a build zone.
	// The points returned by this function are based solely on site extents, which is all I know of the site geometry.
	// It is preferable to use AuBdDesign::BuildZoneCheckLocs because they are more accurate than an extents box.
	void BuildZoneCheckLocs(AuPointArray& bzLocs) const;

	inline const AuPoint& SiteMax() const
	{
		return m_SiteMax;
	}

	inline const AuPoint& SiteMin() const
	{
		return m_SiteMin;
	}

	static unsigned int SizeLimit();

	static unsigned int SizeLimitPerc();

	// Skeleton hits are a portion of HullHits.
	quint32 SkeletonHits() const;

	// Returns fraction of volume that is skeleton. 0..1.
	float SkeletonLevel() const;

	bool SkeletonOkForFrigidLand() const;

	bool SkeletonOkForFrigidSeaBottom() const;

	bool SkeletonOkForGasGiantCore() const;

	bool SkeletonOkForInfernoLand() const;

	// Not frigid or inferno.
	bool SkeletonOkForSeaBottom() const;

	// Returns volume in cubic meters.
	inline quint32 SkeletonVolume() const
	{
		return m_SkeletonVolume;
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

	inline quint32 StoreCapacity() const
	{
		return m_StoreCapacity;
	}

	// Returns volume in cubic meters.
	inline quint32 StoreVolume() const
	{
		return m_StoreVolume;
	}

	inline BUILDING_STRUCTURE Structure() const
	{
		return (BUILDING_STRUCTURE)m_Structure;
	}

	// Hull hits are the entire structural hit points of the ship. Same as StructureHits.
	inline quint32 StructureHits() const
	{
		return HullHits();		// SYS_Hull + SYS_Fuel + SYS_Hold + skeleton
	}

	quint16 SurgeryUnitCount() const;

	QList<quint16> SurgeryUnitIds() const;

	// 'consoleId' is the part id of the medical console.
	COMMODITYID SurgeryUnitModule(quint16 consoleId) const;

	quint32 SystemHits(const AuSysId& sysId) const;

	// Retrieves a list of all system ids.
	// 'structural' is true to get structural systems, false to get nonstructural system.
	void SystemIds(QSet<AuSysId>& sysIds, bool structural) const;

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
		return NonStructureHits() + StructureHits();
	}

	float TractorBeamRange(quint16 which, unsigned int unitsFired) const;

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

	// Returns the id of the transporter station in 'roomId', if any.
	// Returns 0 if none.
	quint16 TransporterStationInRoom(quint16 roomId) const;

	inline quint32 TriangleCount() const
	{
		return m_TriangleCount;
	}

	// Key is turbo lift part id.
	inline const AuScReportLiftHash& TurboLifts() const
	{
		return m_Lifts;
	}

	// Returns the door id at the 'stopName' of the turbo lift 'liftId'.
	quint16 TurboLiftStopDoor(quint16 liftId, const QString& stopName) const;

	double TurretRange(quint16 which) const;

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

	float WeaponBayPower(quint16 which, float unitsFired) const;

	float WeaponBayRange(quint16 which) const;

	inline const AuScReportSysVolHash& WeaponBays() const
	{
		return m_Bays;
	}

	// Converts volume into equivalent number of equipment units in the old system.
	double WeaponBayUnits(quint16 partId) const;

protected:
	// Return number of whole fields.
	quint32 CalcFieldCapacity() const;

	// Return number of whole arenas.
	quint32 CalcArenaCapacity() const;

	// Return number of whole auditoriums.
	quint32 CalcAuditoriumCapacity() const;

	double CalcCapacitorMax() const;

	// Returns mass in pounds.
	double CalcEquipmentMass() const;

	// Returns power plant fuel burn rate in cubic feet per real world second.
	// AuBdDesign::PowerPerFuel returns the number of power units produced from each cubic foot of fuel that is burned.
	double CalcFuelBurnRate() const;

	// Return number of commodity units.
	quint32 CalcHoldCapacity() const;

	// Return number of whole homes.
	quint32 CalcHomeLargeCapacity() const;

	// Return number of whole homes.
	quint32 CalcHomeMediumCapacity() const;

	// Return number of whole homes.
	quint32 CalcHomeSmallCapacity() const;

	// Returns mass in pounds.
	double CalcHullMass() const;

	// Return number of whole lounges.
	quint32 CalcLoungeCapacity() const;

	// Return number of whole offices.
	quint32 CalcOfficeCapacity() const;

	double CalcPowerUsageIdling(bool shieldsUp) const;

	// Return number of whole stores.
	quint32 CalcStoreCapacity() const;

	// Ranges of sensors in feet.
	double CalcSensorRange() const;

	// Return number of whole jobs.
	quint32 CalcShopCapacity() const;

	// Hull hits are the entire structural hit points of the building due to hull volume. Includes HoldHits and skeleton.
	quint32 HullHits() const;

	// Nominal demand, not adjusted for environment.
	int LifeSupportNominalDemand() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuBdReport& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBdReport& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuBdReport)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuBdReport_H
