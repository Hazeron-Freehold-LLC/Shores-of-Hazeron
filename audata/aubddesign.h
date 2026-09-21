//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubddesign.h,v 1.62 2022/05/04 23:30:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdDesign_H
#define AuBdDesign_H

#include "audesign.h"
#include "aubdmodules.h"
#include "dbindustry.h"

class AuBdReport;
class AuDoorStateHash;
class AuGLEffects;
class AuLiftHash;
class AuMatrix4x4;
class AuScParts;
class AuScPartSet;
class AuScReportGameTableHash;
class AuScReportLiftHash;
class AuScReportServicePanelHash;
class AuScReportStatusDisplayHash;
class AuScRoute;
class AuSegmentArray;
class AuZoneResource;
class DeProcessComponentArray;
class QProgressDialog;
class QSqlDatabase;

class AUDATA_DLL AuBdDesign : public AuDesign
{
friend class AuBdDesignHash;
private:
	// Design Properties
	quint32								m_HullVolume;			// Calculated from parts and stored for speed. Takes along time.
	quint8								m_IndustryId;			// INDUSTRYID enum intended for this design. Not a restriction.
	AuBdModules							m_Modules;
	AuScParts*							m_Parts;
	quint8								m_Structure;			// BUILDING_STRUCTURE enum

	// Allocated from m_HullVolume. These must all add up to equal m_HullVolume.
	quint32								m_Arena;				// In cubic meters.
	quint32								m_Armor;				// In cubic meters.
	quint32								m_Auditorium;			// In cubic meters.
	QHash<quint16, quint32>				m_Bays;					// Key is part number with bay module, value is cubic meters.
	quint32								m_Capacitor;			// In cubic meters.
	quint32								m_Hold;					// In cubic meters.
	quint32								m_HomesLarge;			// In cubic meters.
	quint32								m_HomesMedium;			// In cubic meters.
	quint32								m_HomesSmall;			// In cubic meters.
	quint32								m_LifeSupport;			// In cubic meters.
	quint32								m_Lounge;				// In cubic meters.
	quint32								m_Offices;				// In cubic meters.
	quint32								m_PowerPlant;			// In cubic meters.
	quint32								m_Sensors;				// In cubic meters.
	quint32								m_Shields;				// In cubic meters.
	quint32								m_Shops;				// In cubic meters.
	quint32								m_Store;				// In cubic meters.
	quint32								m_Skeleton;				// In cubic meters.

public:	// Constructors
	AuBdDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect);
	AuBdDesign();
	virtual ~AuBdDesign();
	AuBdDesign(const AuBdDesign &c);

public:	// Operators
	virtual AuBdDesign& operator=(const AuBdDesign &c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	// Returns square meters.
	static quint32 AreaFirstField();
	static quint32 AreaAddField();

	inline quint32 ArenaVolume() const
	{
		return m_Arena;
	}

	inline double ArenaVolumeCuFt() const
	{
		return m_Arena * CubicFeetPerCubicMeter();
	}

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

	inline quint32 AuditoriumVolume() const
	{
		return m_Auditorium;
	}

	inline double AuditoriumVolumeCuFt() const
	{
		return m_Auditorium * CubicFeetPerCubicMeter();
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

	// Returns a number from 0 to 1. 0=no influence, 1=full influence.
	float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const;

	float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
			bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
			const AuZoneResource& res) const;

	// Returns my site locations that are used to verify placement within a build zone.
	void BuildZoneCheckLocs(AuPointArray& locs) const;

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
			bool hullDoorsUsable, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	virtual unsigned int CompressedSizeLimit() const;

	// 'fenceStyle' is an AuScFence::STYLE
	bool CreateFence(const AuSegmentArray& fenceSegs, int fenceStyle, quint32 designId, const QString& designName,
			const QString& designDesc, const AuId& archId, const QString& archName);

	inline bool CreateFence(const AuSegmentArray& fenceSegs, int fenceStyle)
	{
		return CreateFence(fenceSegs, fenceStyle, 0, QString(), QString(), AuId(), QString());
	}

	// Returns the description as a single line.
	virtual QString DescriptionOneLine() const;

	// Copies only the fields the server wants from a design record loaded from a file is sent to the server.
	virtual void DesignLoaded(const AuDesign& design);

	double EquipmentMass() const;

	quint32 FieldArea() const;

	virtual bool FinalizeAssembly(QStringList& msgs);

	virtual bool FinalizeDesign(QStringList& msgs, QProgressDialog& progress);

	virtual void FinalizePreview();

	// Searches from 'fromLoc' to find a ladder segment.
	// Uses IsPathBlocked test to insure fromLoc can get to each ladder.
	bool FindLadderToGrab(const AuPoint& fromLoc, AuMatrix4x4& mLadder, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops) const;
	// In this version, we find a ladder that has at least one end at 'ladderPathLoc'.
	bool FindLadderToGrabAt(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder,
			const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	// Returns weight of fuel in lbs per cubic foot.
	static inline double FuelDensity()
	{
		// http://wiki.answers.com/Q/How_much_does_a_gallon_of_liquid_hydrogen_weigh
		// "1 liter of liquid hydrogen weighs only 0.07 kg"
		// 1 cubic foot = 28.3168466 liters
		return .07 * PoundsPerKilogram() * 28.3168466;
	}

	double GroundHeightAboveCG() const;

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

	// Returns the height of 'loc' above the first solid floor below it, starting with the room void that contains it, if any.
	// 'floorN' is a return value, set to the surface normal of the floor.
	// 'inGravityWell' is a return value, set true if 'loc' is inside a room void with artificial gravity.
	// 'gravityN' is the environmental gravity up vector.
	double HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, AuPoint& floorN, bool& inGravityWell,
			const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	// Returns the height of the next solid ceiling above 'loc', starting with the room void that contains it, if any.
	// 'gravityN' is the environmental gravity up vector.
	double HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops) const;

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

	inline quint32 HomeLargeVolume() const
	{
		return m_HomesLarge;
	}

	inline double HomeLargeVolumeCuFt() const
	{
		return m_HomesLarge * CubicFeetPerCubicMeter();
	}

	inline quint32 HomeMediumVolume() const
	{
		return m_HomesMedium;
	}

	inline double HomeMediumVolumeCuFt() const
	{
		return m_HomesMedium * CubicFeetPerCubicMeter();
	}

	inline quint32 HomeSmallVolume() const
	{
		return m_HomesSmall;
	}

	inline double HomeSmallVolumeCuFt() const
	{
		return m_HomesSmall * CubicFeetPerCubicMeter();
	}

	// Returns density in pounds per cubic foot (lbs / ft^3) of empty hull.
	// 'armorLevel' is in the range 0 to 10.
	static inline double HullDensity(double armorLevel)
	{
		// Density is increased by armor.
		return DensityOfMetalHull() * HullDensityFactor(armorLevel);
	}

	// Density factor is increased by armor.
	// Density factor is not affected by hull material, like metal vs magmium, because that would cause undesirable side effects
	// such as a station might become a space ship because of the resulting increase to acceleration.
	// 'armorLevel' is in the range 0 to 10.
	// Returns a density factor that is the actual density / nominal density.
	// The range of return values is [1 - 12.1].
	static double HullDensityFactor(double armorLevel);

	double HullMass() const;

	virtual COMMODITYID HullModule() const
	{
		return m_Modules.HullModule();
	}

	// This is the box radius of the largest possible hull.
	static double HullSizeLimitBase(BUILDING_STRUCTURE structure);
	static double HullSizeLimitDepth(BUILDING_STRUCTURE structure);
	static double HullSizeLimitHeight(BUILDING_STRUCTURE structure);

	static double SiteSizeLimitBase(BUILDING_STRUCTURE structure);
	static double SiteSizeLimitDepth(BUILDING_STRUCTURE structure);
	static double SiteSizeLimitHeight(BUILDING_STRUCTURE structure);

	// Only valid after CalcHullVolume has been called.
	inline quint32 HullVolume() const
	{
		return m_HullVolume;
	}

	inline double HullVolumeCuFt() const
	{
		return m_HullVolume * CubicFeetPerCubicMeter();
	}

	// Returns true if the 2D location of 'loc' falls within any of my site meshes.
	bool InBuildingSite(const AuPoint& loc) const;

	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	bool InBuildingSite(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const;

	// Returns id of build zone, if the 3D location of 'loc' falls ABOVE any of my build zone meshes.
	quint16 InBuildZone(const AuPoint& loc) const;

	inline INDUSTRYID IndustryId() const
	{
		return (INDUSTRYID)m_IndustryId;
	}

	bool IsInsideHull(const AuPoint& loc, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	bool IsInsideObstructionModel(const AuPoint& loc, const AuDoorStateHash& doorStates) const;

	// Returns true if 'fromLoc' is on a ladder path to 'ladderPathLoc', which is a route location known to be the end of a ladder
	// segment.
	bool IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, const AuDoorStateHash& doorStates) const;

	// 'doorStates' is the current state of doors, not the states of the doors in the design.
	bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
			const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	inline COMMODITYID LifeSupportModule() const
	{
		return m_Modules.LifeSupportModule();
	}

	// Returns cubic feet of life support needed.
	double LifeSupportNeeded(quint8 lifeSupportQ) const;

	// Returns life support power use rate in units per real world second.
	double LifeSupportPowerRate() const;

	// Returns the quality of life support system needed to support the number of non-officer berths in the design.
	// Life support quality is the limiting factor to non-officer berth count.
	static quint8 LifeSupportQualityRequiredByBerths(int nonOfficerBerths);

	inline quint32 LifeSupportVolume() const
	{
		return m_LifeSupport;
	}

	inline double LifeSupportVolumeCuFt() const
	{
		return m_LifeSupport * CubicFeetPerCubicMeter();
	}

	virtual void ListComponents(DeProcessComponentArray& pc) const;

	// Gets a list of the environment voids in the parts list.
	void ListEnvironmentVoids(AuScParts& envVoids) const;

	inline quint32 LoungeVolume() const
	{
		return m_Lounge;
	}

	void Load(quint32 idDesign);

	inline double LoungeVolumeCuFt() const
	{
		return m_Lounge * CubicFeetPerCubicMeter();
	}

	// Returns my mass in pounds.
	double Mass() const;

	QString MaterialName() const;

	static double MaxVolume();

	virtual quint64 MemoryUsage() const;

	inline quint32 OfficeVolume() const
	{
		return m_Offices;
	}

	inline double OfficeVolumeCuFt() const
	{
		return m_Offices * CubicFeetPerCubicMeter();
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
	bool PathLocInsideDoor(quint16 doorId, AuPoint& loc, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	// Power units produced per cubic foot of fuel burned.
	static inline double PowerPerFuel()
	{
		return 10.;
	}

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

	quint16 RandomBuildZoneLocation(AuPoint& zoneLoc) const;

	bool RandomRoomLocation(quint16 roomId, AuPoint& voidLoc, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	// Locates a random location inside a room named 'roomName'. Returns false if none is found.
	// 'roomName' is compared against the format returned by ListRoomNames, which is 'room name N' where N is the room id.
	// It is also compared case insensitively against leading fragments, so 'bridge' would match 'Bridge 12'.
	bool RandomRoomLocation(const QString& roomName, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops) const;

	quint16 RandomEnvironmentVoidLocation(AuPoint& voidLoc, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	// 'personnelAccess' is a bitwise combination of PERSONNEL_ACCESS bits, to specify who the room void location is for.
	quint16 RandomRoomVoidLocation(AuPoint& voidLoc, quint8 personnelAccess, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops) const;

	// Returns a location on a site mesh that is not blocked by any obstructions.
	quint16 RandomSiteLocation(AuPoint& siteLoc, const AuDoorStateHash& doorStates) const;

	virtual void RefreshObstructionsCache();
	virtual void RefreshObstructionsCache(QProgressDialog& progress);

	virtual bool RefreshObstructionsCacheIfNeeded();

	virtual QString ReportAnalysis() const;

	// Builds the data in 'report' based on this design as equipped.
	void ReportDesign(AuBdReport& report) const;

	// Returns the id of a room with 'roomName'.
	// CAVEAT: More than one room can share the same name.
	quint16 RoomId(const QString& roomName) const;

	quint16 RoomVoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	quint16 RoomVoidContainingLoc(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops) const;

	// Live because we are getting states of things from a live spacecraft, presumably.
	quint16 EnvironmentVoidContainingLocLive(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops) const;

	// Live because we are getting states of things from a live spacecraft, presumably.
	quint16 RoomVoidContainingLocLive(const AuPoint& loc, const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

	// Live because we are getting states of things from a live spacecraft, presumably.
	quint16 RoomVoidContainingLocLive(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops) const;

	// Determines a route from 'fromLoc' to 'toLoc', either or both of which might not be aboard.
	// 'canUnlockDoors' is true if the locked state of doors can be ignored.
	// Returns true if a route is found.
	bool Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, bool canUnlockDoors,
			const AuDoorStateHash& doorStates, const AuLiftHash& liftStops) const;

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

	void SetArenaVolume(quint32 hullVol);

	void SetArmorVolume(quint32 armor);

	void SetAuditoriumVolume(quint32 hullVol);

	// 'which' is a part id.
	void SetBayModule(quint16 which, COMMODITYID cid);

	// 'which' is a part id.
	void SetBayVolume(quint16 which, quint32 hullVol);

	inline void SetCapacitorModule(COMMODITYID cid)
	{
		m_Modules.SetCapacitorModule(cid);
	}

	void SetCapacitorVolume(quint32 hullVol);

	void SetHoldVolume(quint32 hullVol);

	void SetHomeLargeVolume(quint32 hullVol);

	void SetHomeMediumVolume(quint32 hullVol);

	void SetHomeSmallVolume(quint32 hullVol);

	inline void SetHullModule(COMMODITYID cid)
	{
		m_Modules.SetHullModule(cid);
	}

	void SetIndustryId(INDUSTRYID iid);

	inline void SetLifeSupportModule(COMMODITYID cid)
	{
		m_Modules.SetLifeSupportModule(cid);
	}

	void SetLifeSupportVolume(quint32 hullVol);

	void SetLoungeVolume(quint32 hullVol);

	void SetOfficeVolume(quint32 hullVol);

	void SetModules(const AuBdModules& modules);

	inline void SetPowerPlantModule(COMMODITYID cid)
	{
		m_Modules.SetPowerPlantModule(cid);
	}

	void SetPowerPlantVolume(quint32 hullVol);

	virtual void SetProperties(const AuDesign& design);

	void SetSensorVolume(quint32 hullVol);

	void SetShieldVolume(quint32 hullVol);

	void SetShopVolume(quint32 hullVol);

	void SetSkeletonVolume(quint32 vol);

	void SetStoreVolume(quint32 hullVol);

	void SetStructure(BUILDING_STRUCTURE structure);

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

	inline quint32 ShopVolume() const
	{
		return m_Shops;
	}

	inline double ShopVolumeCuFt() const
	{
		return m_Shops * CubicFeetPerCubicMeter();
	}

	// Check sight segment from 'eyeLoc' to 'sightLoc'.
	// Returns true if the sight segment hits the design. 'sightPart' is the specific part that is hit.
	// If I am hit, 'sightLoc' is adjusted closer to 'eyeLoc', to the point that was hit, and 'sightSurfaceNormal' is set.
	bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart, const AuDoorStateHash& doorStates,
			const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	// Check sight segment from 'eyeLoc' to 'sightLoc'.
	// Returns true if the sight segment hits a build zone. 'sightPart' is the specific build zone part that is hit.
	// If I am hit, 'sightLoc' is adjusted closer to 'eyeLoc', to the point that was hit.
	bool SightBuildZone(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart) const;

	inline quint32 SkeletonVolume() const
	{
		return m_Skeleton;
	}

	inline double SkeletonVolumeCuFt() const
	{
		return m_Skeleton * CubicFeetPerCubicMeter();
	}

	inline quint32 StoreVolume() const
	{
		return m_Store;
	}

	inline double StoreVolumeCuFt() const
	{
		return m_Store * CubicFeetPerCubicMeter();
	}

	inline BUILDING_STRUCTURE Structure() const
	{
		return (BUILDING_STRUCTURE)m_Structure;
	}

	virtual QString StructureName() const
	{
		return BuildingStructureName(Structure());
	}

	// Calculate the maximum range of the weapon based on the number of units fired.
	static inline double TractorBeamRange(double weaponFiredCuFt, quint8 bayQ)
	{
		return HarvesterBayRange(weaponFiredCuFt, bayQ) / 2;
	}

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
		return BLUEPRINT_Building;
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
			const AuLiftHash& liftStops, quint16 exceptVoidId) const;

	// Returns cubic meters.
	static quint32 VolumeFirstArena();
	static quint32 VolumeAddArena();

	// Returns cubic meters.
	static quint32 VolumeFirstAuditorium();
	static quint32 VolumeAddAuditorium();

	// Returns cubic meters.
	static quint32 VolumeFirstGenerator();
	static quint32 VolumeAddGenerator();

	// Returns cubic meters.
	static quint32 VolumeFirstHomeLarge();
	static quint32 VolumeAddHomeLarge();

	// Returns cubic meters.
	static quint32 VolumeFirstHomeMedium();
	static quint32 VolumeAddHomeMedium();

	// Returns cubic meters.
	static quint32 VolumeFirstHomeSmall();
	static quint32 VolumeAddHomeSmall();

	// Returns cubic meters.
	static quint32 VolumeFirstLounge();
	static quint32 VolumeAddLounge();

	// Returns cubic meters.
	static quint32 VolumeFirstOffice();
	static quint32 VolumeAddOffice();

	// Returns cubic meters.
	static quint32 VolumeFirstShop();
	static quint32 VolumeAddShop();

	// Returns cubic meters.
	static quint32 VolumeFirstStore();
	static quint32 VolumeAddStore();

private:
	// Calculates the value of m_HullVolume.
	void CalcHullVolume();

	// Updates m_Bays to match bays created by existing parts.
	void SyncBays();
};

inline QDataStream& operator<<(QDataStream& out, const AuBdDesign& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBdDesign& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuBdDesign)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Key is design id, value is the design.
class AUDATA_DLL AuBdDesignHash : public unordered_map<quint32, AuBdDesign, AuDesignIdHash>
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

	void Merge(const AuBdDesignHash& mdh);

	quint32 SoloInitNextId() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuBdDesignHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBdDesignHash& o)
{
	o << in;
	return in;
}

#endif // AuBdDesign_H
