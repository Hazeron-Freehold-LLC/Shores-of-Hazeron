//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdesign.h,v 1.162 2022/05/04 23:30:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDesign_H
#define AuScDesign_H

#include "audesign.h"
#include "auscmodules.h"

class AuDoorStateHash;
class AuGLEffects;
class AuLandingGearStateHash;
class AuLiftHash;
class AuMatrix4x4;
class AuScPartSet;
class AuScReport;
class AuScReportGameTableHash;
class AuScReportLiftHash;
class AuScReportServicePanelHash;
class AuScRoute;
class DeProcessComponentArray;
class QProgressDialog;
class QSqlDatabase;

class AUDATA_DLL AuScDesign : public AuDesign
{
friend class AuScDesignHash;
public:
	enum STATION_ORIENT
	{
		SORI_Random,
		SORI_Upright,
		SORI_UpsideDown,
		SORI_NoseDown,
		SORI_TailDown,
		SORI_PortDown,
		SORI_StarboardDown,
		SORI_Count
	};

private:
	// Design Properties
	quint32								m_HullVolume;			// Calculated from parts and stored for speed. Takes along time.
	quint8								m_LandingOrient;		// LANDING_ORIENT enum landing orientation
	AuScModules							m_Modules;
	AuScParts*							m_Parts;
	quint8								m_StationOrient;		// STATION_ORIENT enum recommended orbit orientation for stations
	quint8								m_Streamline;			// STREAMLINE enum
	quint8								m_Structure;			// SPACECRAFT_STRUCTURE enum
	quint8								m_MinQuality;			// Minimum Quality at which the design must be manufactured.

	// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
	quint32								m_Armor;				// In cubic meters.
	QHash<quint16, quint32>				m_Bays;					// Key is part number with bay module, value is cubic meters.
	quint32								m_Capacitor;			// In cubic meters.
	quint32								m_CryoBerths;			// In cubic meters. 1 cu m = 1 berth.
	QHash<quint16, quint32>				m_FTLDrives;			// Key is part number with FTL module, value is cubic meters.
	quint32								m_Fuel;					// In cubic meters.
	quint32								m_Hold;					// In cubic meters.
	quint32								m_LifeSupport;			// In cubic meters.
	quint32								m_ManeuverDrive;		// In cubic meters.
	quint32								m_PowerPlant;			// In cubic meters.
	quint32								m_Sensors;				// In cubic meters.
	quint32								m_Shields;				// In cubic meters.
	quint32								m_Skeleton;				// In cubic meters.

public:	// Constructors
	AuScDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect);
	AuScDesign();
	virtual ~AuScDesign();
	AuScDesign(const AuScDesign &c);

public:	// Operators
	virtual AuScDesign& operator=(const AuScDesign &c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	// Returns armor level in the range 0 to 10.
	inline float ArmorLevel() const
	{
		return ArmorLevel(m_Armor, m_HullVolume);
	}

	static inline float ArmorLevel(quint32 armorVol, quint32 hullVol)
	{
		if(!hullVol || !armorVol)
			return 0;
		float armor = (float)armorVol / (float)hullVol;		// Armor from 0 to 1.
		if(armor >= .5f)
			return 10.f;										// Max armor level is 10.
		return armor * 20.f;									// Armor level in the range 0 to 10.
	}

	inline quint32 ArmorVolume() const
	{
		return m_Armor;
	}

	inline double ArmorVolumeCuFt() const
	{
		return m_Armor * CubicFeetPerCubicMeter();
	}

	// Returns total of all bays.
	quint32 BayVolume() const;

	double BayVolumeCuFt() const;

	// 'which' is a part id.
	quint32 BayVolume(quint16 which) const;

	double BayVolumeCuFt(quint16 which) const;

	// Fills 'partIds' with ids of fire control stations that can have a weapon module associated with them.
	void Bays(AuScPartSet& partIds) const;

	// Cannons determines the number of heavy weapon rounds that can be fired simultaneously from a ballistic weapon bay.
	static inline quint32 BayWeaponCannons(double weaponCuFt)
	{
		if(!weaponCuFt)
			return 0;
		double weaponUnits = weaponCuFt / 256.;
		return ((quint32)round(weaponUnits/10.)) + 1;
	}

	// Ejector racks determines the number of bombs that can be dropped simultaneously from a weapon deployment bay.
	static inline quint32 BayWeaponEjectorRacks(double weaponCuFt)
	{
		if(!weaponCuFt)
			return 0;
		double weaponUnits = weaponCuFt / 256.;
		return ((quint32)round(weaponUnits/100.)) + 1;
	}

	// Projectors determines the number of energy weapons that can be fired simultaneously from an energy weapon bay.
	static inline quint32 BayWeaponProjectors(quint32 weaponCuFt)
	{
		if(!weaponCuFt)
			return 0;
		double weaponUnits = weaponCuFt / 256.;
		return ((quint32)round(weaponUnits/25.)) + 1;
	}

	// Launchers determines the number of missiles or rockets that can be launched simultaneously from a weapon launcher bay.
	static inline quint32 BayWeaponLaunchers(quint32 weaponCuFt)
	{
		if(!weaponCuFt)
			return 0;
		double weaponUnits = weaponCuFt / 256.;
		return ((quint32)round(weaponUnits/90.)) + 1;
	}

	//  Each unit fires 50 units of energy at QualityBaseline quality.
	static inline double BayWeaponPower(double weaponFiredCuFt, quint8 bayQ)
	{
		double weaponUnits = weaponFiredCuFt / 256.;
		return weaponUnits * 10 * QualityBoostF(bayQ);
	}

	// Calculate the maximum range of the weapon based on the number of units fired.
	static inline double BayWeaponRange(quint8 bayQ)
	{
		return AttackRange() * QualityBoostF(bayQ);
	}

	static inline double CannonBayRange(quint8 bayQ)
	{
		return 6000*FeetPerMeter() * QualityBoostF(bayQ);
	}

	double CapacitorMax(quint8 capacitorQ) const;

	inline COMMODITYID CapacitorModule() const
	{
		return m_Modules.CapacitorModule();
	}

	inline quint32 CapacitorVolume() const
	{
		return m_Capacitor;
	}

	inline double CapacitorVolumeCuFt() const
	{
		return m_Capacitor * CubicFeetPerCubicMeter();
	}

	virtual AuDesign* Clone() const;

	virtual void ClearParts();

	bool ClosestEntrance(AuPoint& entranceLoc, quint16& doorId, quint16& ladderId, const AuPoint& loc, bool useGroundLevelDoors,
			bool hullDoorsUsable, const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates,
			const AuLiftHash& liftStops) const;

	// Returns the number of command and control stations. This is command and crew stations.
	int CommandAndControlStations() const;

	inline COMMODITYID CommandAndControlModule() const
	{
		return m_Modules.CommandAndControlModule();
	}

	virtual unsigned int CompressedSizeLimit() const;

	virtual void ContrailEmitters(QMap<AuPoint, float>& contrails) const;

	inline quint32 CryoBerthVolume() const
	{
		return m_CryoBerths;
	}

	inline double CryoBerthVolumeCuFt() const
	{
		return m_CryoBerths * CubicFeetPerCubicMeter();
	}

	// Returns the description as a single line.
	// Format example: Streamlined Starship 32m^3 (using cubed symbol instead of ^3).
	virtual QString DescriptionOneLine() const;

	// Copies only the fields the server wants from a design record loaded from a file is sent to the server.
	virtual void DesignLoaded(const AuDesign& design);

	double EquipmentMass() const;

	virtual bool FinalizeAssembly(QStringList& msgs);

	virtual bool FinalizeDesign(QStringList& msgs, QProgressDialog& progress);

	virtual void FinalizePreview();

	// Searches from 'fromLoc' to find a ladder segment.
	// Uses IsPathBlocked test to insure fromLoc can get to each ladder.
	bool FindLadderToGrab(const AuPoint& fromLoc, AuMatrix4x4& mLadder, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;
	// In this version, we find a ladder that has at least one end at 'ladderPathLoc'.
	bool FindLadderToGrabAt(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Fills 'partIds' with ids of engineer stations that can have an FTL drive module associated with them.
	void FTLDrives(AuScPartSet& partIds) const;

	// Returns total of all FTL drives.
	quint32 FTLDriveVolume() const;

	double FTLDriveVolumeCuFt() const;

	// 'which' is a part id.
	quint32 FTLDriveVolume(quint16 which) const;

	double FTLDriveVolumeCuFt(quint16 which) const;

	// The formula for the volume of a sphere gives us the number of FTL drive units required.
	//  V = 4/3 * PI * r^3
	// where
	//  V = Volume, which is the volume of the ship hull in our calculation.
	//  r = Number of FTL drive units required.
	// reduced
	//  r = cuberoot(3/4 * V * PI)
	// Returns the number of cubic feet of FTL drive units needed to engage.
	double FTLDriveUnitsToEngage() const;

	// Returns power plant fuel burn rate in cubic feet per real world second.
	// AuScDesign::PowerPerFuel returns the number of power units produced from each cubic foot of fuel that is burned.
	double FuelBurnRate() const;

	// Returns weight of fuel in lbs per cubic foot.
	static inline double FuelDensity()
	{
		// http://wiki.answers.com/Q/How_much_does_a_gallon_of_liquid_hydrogen_weigh
		// "1 liter of liquid hydrogen weighs only 0.07 kg"
		// 1 cubic foot = 28.3168466 liters
		return .07 * PoundsPerKilogram() * 28.3168466;
	}

	double FuelMass() const;

	// Return number of commodity units.
	quint32 FuelCapacity() const;

	inline quint32 FuelVolume() const
	{
		return m_Fuel;
	}

	inline double FuelVolumeCuFt() const
	{
		return m_Fuel * CubicFeetPerCubicMeter();
	}

	double GroundHeightAboveCG(const AuLandingGearStateHash& landingGearStates) const;

	// Returns the number of commodity units that may be harvested from each energy unit that hits the target.
	static int HarvestUnitsPerEnergyUnit()
	{
		return 10;
	}

	// Calculate the maximum range of the weapon based on the number of units fired.
	static inline double HarvesterBayRange(double weaponFiredCuFt, quint8 bayQ)
	{
		// Weapon power drops off linearly to zero at the maximum range, determined as follows.
		//  U = units fired. Each unit fires 10 units of energy.
		//  E = energy fired
		//  E = U * 10
		//  R = range at which linear dropoff reaches 0
		//  R = sqrt(E) * 1000
		// Examples:   1 weapon bay units emit     10 units of power that will drop off to 0 units at   316 meters.
		//             2 weapon bay units emit     20 units of power that will drop off to 0 units at   447 meters.
		//             4 weapon bay units emit     40 units of power that will drop off to 0 units at   633 meters.
		//            10 weapon bay units emit    100 units of power that will drop off to 0 units at  1000 meters.
		//           100 weapon bay units emit   1000 units of power that will drop off to 0 units at  3162 meters.
		//          1000 weapon bay units emit  10000 units of power that will drop off to 0 units at 10000 meters.
		//         10000 weapon bay units emit 100000 units of power that will drop off to 0 units at 31623 meters.
		double weaponUnits = weaponFiredCuFt / 256.;
		return sqrt(weaponUnits*10) * 100 * FeetPerMeter() * QualityBoostF(bayQ);
	}

	// 'which' is a part id.
	bool HasBayModule(quint16 which) const;

	// 'which' is a part id.
	bool HasFTLDriveModule(quint16 which) const;

	// Returns the height of 'loc' above the first solid floor below it, starting with the room void that contains it, if any.
	// 'floorN' is a return value, set to the surface normal of the floor.
	// 'inGravityWell' is a return value, set true if 'loc' is inside a room void with artificial gravity.
	// 'gravityN' is the environmental gravity up vector.
	double HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, AuPoint& floorN, bool& inGravityWell,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Returns the height of the next solid ceiling above 'loc', starting with the room void that contains it, if any.
	// 'gravityN' is the environmental gravity up vector.
	double HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Return number of commodity units.
	quint32 HoldCapacity() const;

	inline quint32 HoldVolume() const
	{
		return m_Hold;
	}

	inline double HoldVolumeCuFt() const
	{
		return m_Hold * CubicFeetPerCubicMeter();
	}

	// Returns density in pounds per cubic foot (lbs / ft^3) of empty hull.
	// Density is increased by armor and streamlining.
	// 'armorLevel' is in the range 0 to 10.
	static inline double HullDensity(double armorLevel, STREAMLINE streamline)
	{
		// Density is increased by armor and streamlining.
		return DensityOfMetalHull() * HullDensityFactor(armorLevel, streamline);
	}

	// Density factor is increased by armor and streamlining.
	// Density factor is not affected by hull material, like metal vs magmium, because that would cause undesirable side effects
	// such as a station might become a space ship because of the resulting increase to acceleration.
	// 'armorLevel' is in the range 0 to 10.
	// Returns a density factor that is the actual density / nominal density.
	// The range of return values is [1 - 12.1].
	static double HullDensityFactor(double armorLevel, STREAMLINE streamline);

	double HullMass() const;

	virtual COMMODITYID HullModule() const
	{
		return m_Modules.HullModule();
	}

	// This is the box radius of the largest possible hull.
	static double HullSizeLimit();

	// Only valid after CalcHullVolume has been called.
	inline quint32 HullVolume() const
	{
		return m_HullVolume;
	}

	inline double HullVolumeCuFt() const
	{
		return m_HullVolume * CubicFeetPerCubicMeter();
	}

	bool IsInsideHull(const AuPoint& loc, const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates,
			const AuLiftHash& liftStops) const;

	bool IsInsideObstructionModel(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	// Returns true if 'fromLoc' is on a ladder path to 'ladderPathLoc', which is a route location known to be the end of a ladder
	// segment.
	bool IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	// 'doorStates' is the current state of doors, not the states of the doors in the design.
	// 'landingGearStates' is the current state of landing gear, not the states of the landing gear in the design.
	bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	inline LANDING_ORIENT LandingOrientation() const
	{
		return (LANDING_ORIENT)m_LandingOrient;
	}

	inline QString LandingOrientationName() const
	{
		return ::LandingOrientationName(LandingOrientation());
	}

	inline COMMODITYID LifeSupportModule() const
	{
		return m_Modules.LifeSupportModule();
	}

	// Returns cubic feet of life support needed.
	double LifeSupportNeeded(quint8 lifeSupportQ) const;

	// Returns life support power use rate in units per real world second.
	double LifeSupportPowerRate() const;

	inline quint32 LifeSupportVolume() const
	{
		return m_LifeSupport;
	}

	inline double LifeSupportVolumeCuFt() const
	{
		return m_LifeSupport * CubicFeetPerCubicMeter();
	}

	virtual void ListComponents(DeProcessComponentArray& pc) const;

	void Load(quint32 idDesign);

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
	double ManeuverAcceleration(quint8 maneuverDriveQ) const;

	// Force exerted by one cubic meter of maneuver drives, in lbs.
	static double ManeuverDriveForce();

	inline COMMODITYID ManeuverDriveModule() const
	{
		return m_Modules.ManeuverDriveModule();
	}

	inline quint32 ManeuverDriveVolume() const
	{
		return m_ManeuverDrive;
	}

	inline double ManeuverDriveVolumeCuFt() const
	{
		return m_ManeuverDrive * CubicFeetPerCubicMeter();
	}

	// Returns maneuver drive fuel use rate in units per real world second.
	// Only rocket drives use fuel.
	double ManeuverFuelRate(COMMODITYID maneuverDriveModule) const;

	// Returns maneuver drive power use rate in units per real world second.
	double ManeuverPowerRate(COMMODITYID maneuverDriveModule) const;

	// Rocket drives do not use power.
	inline double ManeuverPowerRateHolding(COMMODITYID maneuverDriveModule) const
	{
		return ManeuverPowerRate(maneuverDriveModule) / 10.;	// Maneuver drive consumes 10% of the power when holding.
	}

	// Returns my mass in pounds.
	double Mass() const;

	// Returns the highest setting available for the FTL drive.
	// Returns -1 if power plant is inadequate to engage the FTL drive or if the FTL drive module is not recognized.
	int MaxCapacitorFTLSetting(COMMODITYID ftlDriveMod, quint8 ftlDriveQ, quint8 capacitorQ) const
	{
		return MaxCapacitorFTLSetting(ftlDriveMod, ftlDriveQ, FTLDriveUnitsToEngage(), CapacitorMax(capacitorQ)); 
	}

	static int MaxCapacitorFTLSetting(COMMODITYID ftlDriveMod, quint8 ftlDriveQ, double ftlDriveUnitsToEngageCuFt,
			double capacitorMax);

	static double MaxVolume();

	virtual quint64 MemoryUsage() const;

	inline quint8 MinQ() const
	{
		return m_MinQuality;
	}

	static qint8 OnDeck(double rht);

	// Returns part id if successful.
	// 'partsAdded' is filled with clones for caller creation of undo data.
	virtual quint16 PartAdd(const AuScPart& part, AuScParts& partsAdded);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartAddFaces(quint16 partId, const AuScFaceList& faces, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsDelete' is filled with deleted parts for caller creation of undo data.
	virtual bool PartDelete(quint16 partId, AuScParts& partsDeleted);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartDeleteState(quint16 partId, quint8 state, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartFillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane,
			AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartFillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3, const AuPoint& v4,
			AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartFillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
			AuScParts& partsChanged);

	// Adds part objects (vertices, edges, faces) of 'part' to 'partId'.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartObjectAdd(quint16 partId, const AuScPart& part, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	// 'partsDeleted' is filled with clones for caller creation of undo data.
	virtual bool PartObjectDelete(quint16 partId, const AuScSelectInfo& selInfo, AuScParts& partsChanged, AuScParts& partsDeleted);

	virtual const AuScParts& Parts() const;

	virtual AuScParts& Parts();

	// Returns true if any changes are made.
	// 'partsAdded' is filled with clones for caller creation of undo data.
	virtual bool PartsCopy(const AuScSelection& partIds, AuScParts& partsAdded);

	virtual bool PartsCopySymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane, AuScParts& partsAdded);

	virtual bool PartSetOmitted(quint16 partId, bool omit, AuScParts& partsChanged);

	virtual bool PartSetSealed(quint16 partId, bool seal, AuScParts& partsChanged);

	// Converts each selected part into a decal if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeDecal(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a door if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeDoor(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a head light if possible. Converted parts are deleted, moved to 'partsDeleted'.
	//virtual bool PartsMakeHeadlightLens(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a hull if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeHull(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a landing gear if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeLandingGear(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	virtual bool PartsMakeLightLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
			AuScParts& partsDeleted);

	// Converts each selected part into a mesh if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeMesh(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a nav light if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeNavLightLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
			AuScParts& partsDeleted);

	// Converts each selected part into a room if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeRoom(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a rotating beacon if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeRotatingBeaconLens(const AuScSelection& sel, const QColor& lightColor, const AuPoint& rotationAxis,
			AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a strobe if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeStrobeLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
			AuScParts& partsDeleted);

	// Converts one selected room and door jig combination into a lift. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeTurboLift(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a turret if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeTurret(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsAdded,
			AuScParts& partsDeleted);

	// Converts each selected part into a window if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeWindow(const AuScSelection& sel, float opacity, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartsTransform(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged, bool objectMode);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	// This is used in object mode to drag copy selected portions of one or more parts.
	virtual bool PartsTransformCopy(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged);

	virtual bool PartsTransformLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged);

	virtual bool PartsUpdateHullDoors(const AuScSelection& sel, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsUpdated' is filled with clones for caller creation of undo data.
	virtual bool PartUpdate(quint16 partId, const AuScPart& part, AuScParts& partUpdated);

	// Sets 'loc' to the path segment location that is inside the door at 'doorId'. The sense of inside only applies to hull doors.
	// For interior doors, a path segment location on either side of the door is found.
	// Returns true on success, false if there is no path segment associated with 'doorId' or if neither end of its path segment is
	// inside the hull.
	bool PathLocInsideDoor(quint16 doorId, AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Quantity of power needed to activate the FTL drive one time.
	double PowerNeededToEngageFTLDrive() const;

	static double PowerNeededToEngageFTLDrive(double ftlDriveUnitsToEngageCuFt);

	// Power units produced per cubic foot of fuel burned.
	static inline double PowerPerFuel()
	{
		return 10.;
	}

	// Power usage per real world second when holding
	// - Gravity drives on HOLD
	// - Life Support on
	// - Sensors on
	// - Shields on
	double PowerUsageHolding(bool shieldsUp, COMMODITYID maneuverDriveModule) const;

	// Power usage per real world second when idle
	// - Gravity drives on
	// - Life Support on
	// - Sensors on
	// - Shields on
	double PowerUsageIdling(bool shieldsUp, COMMODITYID maneuverDriveModule) const;

	inline COMMODITYID PowerPlantModule() const
	{
		return m_Modules.PowerPlantModule();
	}

	inline quint32 PowerPlantVolume() const
	{
		return m_PowerPlant;
	}

	inline double PowerPlantVolumeCuFt() const
	{
		return m_PowerPlant * CubicFeetPerCubicMeter();
	}

	bool RandomRoomLocation(quint16 roomId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Locates a random location inside a room named 'roomName'. Returns false if none is found.
	// 'roomName' is compared against the format returned by ListRoomNames, which is 'room name N' where N is the room id.
	// It is also compared case insensitively against leading fragments, so 'bridge' would match 'Bridge 12'.
	bool RandomRoomLocation(const QString& roomName, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// 'personnelAccess' is a bitwise combination of PERSONNEL_ACCESS bits, to specify who the room void location is for.
	quint16 RandomRoomVoidLocation(AuPoint& voidLoc, quint8 personnelAccess, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual void RefreshObstructionsCache();
	virtual void RefreshObstructionsCache(QProgressDialog& progress);

	virtual bool RefreshObstructionsCacheIfNeeded();

	virtual QString ReportAnalysis() const;

	// Builds the data in 'report' based on this design as equipped.
	void ReportDesign(AuScReport& report) const;

	// Returns the id of a room with 'roomName'.
	// CAVEAT: More than one room can share the same name.
	quint16 RoomId(const QString& roomName) const;

	// Live because we are getting states of things from a live spacecraft, presumably.
	quint16 RoomVoidContainingLocLive(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Live because we are getting states of things from a live spacecraft, presumably.
	quint16 RoomVoidContainingLocLive(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Determines a route from 'fromLoc' to 'toLoc', either or both of which might not be aboard.
	// 'canUnlockDoors' is true if the locked state of doors can be ignored.
	// Returns true if a route is found.
	bool Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, bool canUnlockDoors,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Ranges of sensors in feet.
	double SensorRange(quint8 sensorQ) const;

	// Returns sensor power use rate in units per real world second.
	double SensorPowerRate() const;

	inline quint32 SensorVolume() const
	{
		return m_Sensors;
	}

	inline double SensorVolumeCuFt() const
	{
		return m_Sensors * CubicFeetPerCubicMeter();
	}

	void SetArmorVolume(quint32 armor);

	// 'which' is a part id.
	void SetBayModule(quint16 which, COMMODITYID cid);

	// 'which' is a part id.
	void SetBayVolume(quint16 which, quint32 hullVol);

	inline void SetCapacitorModule(COMMODITYID cid)
	{
		m_Modules.SetCapacitorModule(cid);
	}

	void SetCapacitorVolume(quint32 hullVol);

	inline void SetCommandAndControlModule(COMMODITYID cid)
	{
		m_Modules.SetCommandAndControlModule(cid);
	}

	void SetCryoBerthVolume(quint32 hullVol);

	void SetFuelVolume(quint32 hullVol);

	// 'which' is a part id.
	void SetFTLDriveModule(quint16 which, COMMODITYID cid);

	// 'which' is a part id.
	void SetFTLDriveVolume(quint16 which, quint32 hullVol);

	void SetHoldVolume(quint32 hullVol);

	inline void SetHullModule(COMMODITYID cid)
	{
		m_Modules.SetHullModule(cid);
	}

	void SetLandingOrientation(LANDING_ORIENT ori);

	inline void SetLifeSupportModule(COMMODITYID cid)
	{
		m_Modules.SetLifeSupportModule(cid);
	}

	void SetLifeSupportVolume(quint32 hullVol);

	inline void SetManeuverDriveModule(COMMODITYID cid)
	{
		m_Modules.SetManeuverDriveModule(cid);
	}

	void SetManeuverDriveVolume(quint32 hullVol);

	// Sets the minimum quality at which this design is to be built.
	void SetMinQ(quint8 qmin);

	void SetModules(const AuScModules& modules);

	inline void SetPowerPlantModule(COMMODITYID cid)
	{
		m_Modules.SetPowerPlantModule(cid);
	}

	void SetPowerPlantVolume(quint32 hullVol);

	virtual void SetProperties(const AuDesign& design);

	void SetSensorVolume(quint32 hullVol);

	void SetShieldVolume(quint32 hullVol);

	void SetSkeletonVolume(quint32 vol);

	void SetStationOrientation(STATION_ORIENT ori);

	void SetStreamline(STREAMLINE streamline);

	void SetStructure(SPACECRAFT_STRUCTURE structure);

	// Fills 'textures' with the OpenGL texture ids expected by the Sc shaders.
	// Loads the textures if needed.
	// An OpenGL context must be current when this function is called.
	// Note the 'textures' array is larger than the m_TexturePalette. Palette textures are looked up/loaded to fill most of the
	// entries in the 'textures' array.
	virtual void ShaderTexturePalette(COMMODITYID hullModule, AuGLEffects& re, AuTexturePalette& textures) const;

	// Returns shield power use rate in units per real world second.
	double ShieldPowerRate() const;

	inline quint32 ShieldVolume() const
	{
		return m_Shields;
	}

	inline double ShieldVolumeCuFt() const
	{
		return m_Shields * CubicFeetPerCubicMeter();
	}

	// Check sight segment from 'eyeLoc' to 'sightLoc'.
	// Returns true if the sight segment hits the design. 'sightPart' is the specific part that is hit.
	// If I am hit, 'sightLoc' is adjusted closer to 'eyeLoc', to the point that was hit, and 'sightSurfaceNormal' is set.
	bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	inline quint32 SkeletonVolume() const
	{
		return m_Skeleton;
	}

	inline double SkeletonVolumeCuFt() const
	{
		return m_Skeleton * CubicFeetPerCubicMeter();
	}

	inline STATION_ORIENT StationOrientation() const
	{
		return (STATION_ORIENT)m_StationOrient;
	}

	inline QString StationOrientationName() const
	{
		return StationOrientationName(StationOrientation());
	}

	inline static QString StationOrientationName(STATION_ORIENT orient)
	{
		if(orient < 0 || orient > SORI_Count)
			orient = SORI_Count;
		QString s[SORI_Count+1] =
		{
			QObject::tr("Random"),								// SORI_Random
			QObject::tr("Upright"),								// SORI_Upright
			QObject::tr("Upside Down"),							// SORI_UpsideDown
			QObject::tr("Nose Down"),							// SORI_NoseDown
			QObject::tr("Tail Down"),							// SORI_TailDown
			QObject::tr("Port Side Down"),						// SORI_PortDown
			QObject::tr("Starboard Side Down"),					// SORI_StarboardDown
			QObject::tr("Invalid Station Orientation")			// SORI_Count
		};
		return s[orient];
	}

	inline STREAMLINE Streamline() const
	{
		return (STREAMLINE)m_Streamline;
	}

	inline QString StreamlineName() const
	{
		return ::StreamlineName(Streamline());
	}

	inline SPACECRAFT_STRUCTURE Structure() const
	{
		return (SPACECRAFT_STRUCTURE)m_Structure;
	}

	virtual QString StructureName() const
	{
		return SpacecraftStructureName(Structure());
	}

	// Calculate the maximum range of the weapon based on the number of units fired.
	static inline double TractorBeamRange(double weaponFiredCuFt, quint8 bayQ)
	{
		return HarvesterBayRange(weaponFiredCuFt, bayQ) / 2;
	}

	// Turn performance in radians/sec
	double TurnRate(quint8 maneuverDriveQ) const;

	void Insert(QSqlDatabase& db) const;
	void Update(QSqlDatabase& db) const;

	virtual void Rx(QDataStream& in);
	virtual void Tx(QDataStream& out) const;

	virtual BLUEPRINT_TYPE Type() const
	{
		return TypeStatic();
	}

	static BLUEPRINT_TYPE TypeStatic()
	{
		return BLUEPRINT_Spacecraft;
	}

	// Adds parts and their ids.
	virtual void UndoRedoAdd(const AuScParts& parts);

	// Deletes parts by id.
	virtual void UndoRedoDelete(const QList<quint16>& partIds);

	// Swaps parts 1:1 by id.
	virtual void UndoRedoSwap(AuScParts& swapParts);

	// Modifies values of this record as necessary to make it valid.
	// Returns true if the record was already valid and no changes are required.
	// Returns false if this record was invalid and changes were made to make it valid.
	virtual bool Validate();

	// Returns any kind of void. Does not have to be a room void.
	quint16 VoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, quint16 exceptVoidId) const;

private:
	// Calculates the value of m_HullVolume.
	void CalcHullVolume();

	// Updates m_Bays and m_FTLDrives to match bays and ftlDrives created by existing parts.
	void SyncBaysAndFTLDrives();
};

inline QDataStream& operator<<(QDataStream& out, const AuScDesign& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScDesign& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuScDesign)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Key is design id, value is the design.
class AUDATA_DLL AuScDesignHash : public unordered_map<quint32, AuScDesign, AuDesignIdHash>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint32 HighestId() const;

	void Load(quint32 idDesign);

	// Loads all designs by 'idArchitect'.
	void LoadArchitect(const AuId& idArchitect);

	// Loads all designs that will be stored in the solo init GS file.
	void LoadSoloInit();

	quint64 MemoryUsage() const;

	void Merge(const AuScDesignHash& mdh);

	quint32 SoloInitNextId() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScDesignHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScDesignHash& o)
{
	o << in;
	return in;
}

#endif // AuScDesign_H
