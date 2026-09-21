//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscparts.cpp,v 1.399 2022/02/19 18:20:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscparts.h"
//
#include "au3dsglsl.h"
#include "aubox.h"
#include "aufilever.h"
#include "auglobal.h"
#include "ausegment.h"
#include "auscdesign.h"
#include "auscreport.h"
#include "auscroute.h"
#include "austylesheet.h"
#include <QDataStream>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <QProgressDialog>
#include <QStringList>
#include <QVariant>

#define DESIGNER_MIME_PART_VER			0x01

#define DESIGNER_MIME_PART_OBJECT_VER	0x01

#define PART_SERIALIZE_VER				0x11

#define d_BuildZoneHeight				(2*FeetPerMeter())
#define d_GroupHull						"Hull"
#define d_GroupPost						"Post"
#define d_GroupSection					"Section"
#define d_HullPlatesPerUnit				192				// 1/16" thick unarmored hull takes 192 1'x1' slices from 1 cu ft of metal.
#define d_SearchTolerance				.025f					// Pixels search tolerance when finding things near the cursor.
#define d_ShowHullSilhouette			false
#define d_SpacecraftLight				(200*FeetPerMeter())	// Attenuation distance of exterior light.
//#define d_ObstructMargin				(1)						// in feet
#define d_ObstructMargin				(2)						// in feet
//#define d_ObstructMargin				(FeetPerMeter())		// in feet

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QReadWriteLock							s_FenceLock;									// Guards these default fences.
AuScFaceList							s_FencePosts[AuScFence::STYLE_Count+1];			// Faces of each style fence post
AuScFaceList							s_FencePostBarriers[AuScFence::STYLE_Count];	// Faces of post barriers
AuScFaceList							s_FenceSections[AuScFence::STYLE_Count+1];		// Faces of each style fence section
AuScFaceList							s_FenceSectionBarriers[AuScFence::STYLE_Count];	// Faces of section barriers 
AuScPathSeg								s_PathSeg;				// Static constructed path segment for the Null return value.

//static
const AuScPathSeg& AuScPathSeg::Null()
{
	return s_PathSeg;
}

quint64 AuScPathSeg::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_Cost);
	mem += sizeof(m_Ladder);
	mem += sizeof(m_LiftId);
	mem += sizeof(m_Seg);
	mem += StringMemoryUsage(m_Stop1);
	mem += StringMemoryUsage(m_Stop2);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScPathSegList::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		out << *ci;
}

void AuScPathSegList::operator<<(QDataStream& in)
{
	clear();
	quint16 c;
	in >> c;
	while(c--)
	{
		AuScPathSeg v;
		in >> v;
		append(v);
	}
}

bool AuScPathSegList::AddIfUniqueSegment(const AuScPathSeg& seg)
{
	if(size() >= 0xffff)
		return false;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).IsSameSegment(seg))
			return false;
	append(seg);
	return true;
}

bool AuScPathSegList::IsAtLoc(const AuPoint& loc) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).IsAt(loc))
			return true;
	return false;
}

quint64 AuScPathSegList::MemoryUsage() const
{
	quint64 mem = sizeof(AuScPathSegList);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += (*ci).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
AuScPart* AuScPart::Create(PART_TYPE type)
{
	// This generates a wonderful compiler warning if I forget to do this for a new part class.
	switch(type)
	{
	case Part_Invalid:
	case PART_Count:
		break;
	case Part_Mesh:
		return new AuScMesh;
	case Part_DoorJig:
		return new AuScDoorJig;
	case Part_WindowJig:
		return new AuScWindowJig;
	case Part_Hull:
		return new AuScHull;
	case Part_Room:
		return new AuScRoom;
	case Part_Door:
		return new AuScDoor;
	case Part_Window:
		return new AuScWindow;
	case Part_LandingGear:
		return new AuScLandingGear;
	case Part_Lift:
		return new AuScLift;
	case Part_StatusAlert:
		return new AuScStatusAlert;
	case Part_StatusLifeSupport:
		return new AuScStatusLifeSupport;
	case Part_StatusMission:
		return new AuScStatusMission;
	case Part_PanelCapacitor:
		return new AuScPanelCapacitor;
	case Part_PanelFTLDrive:
		return new AuScPanelFTLDrive;
	case Part_PanelLifeSupport:
		return new AuScPanelLifeSupport;
	case Part_PanelManeuverDrive:
		return new AuScPanelManeuverDrive;
	case Part_PanelPowerPlant:
		return new AuScPanelPowerPlant;
	case Part_PanelSensor:
		return new AuScPanelSensor;
	case Part_PanelShield:
		return new AuScPanelShield;
	case Part_PanelWeapon:
		return new AuScPanelWeapon;
	case Part_StationCommand:
		return new AuScStationCommand;
	case Part_StationDesigner:
		return new AuScStationDesigner;
	case Part_StationEngineer:
		return new AuScStationEngineer;
	case Part_StationFireControl:
		return new AuScStationFireControl;
	case Part_StationHelm:
		return new AuScStationHelm;
	case Part_StationMedic:
		return new AuScStationMedic;
	case Part_StationNavigator:
		return new AuScStationNavigator;
	case Part_StationPowerRelay:
		return new AuScStationPowerRelay;
	case Part_StationSensor:
		return new AuScStationSensor;
	case Part_StationShield:
		return new AuScStationShield;
	case Part_StationTransporter:
		return new AuScStationTransporter;
	case Part_ConsoleCommand:
		return new AuScConsoleCommand;
	case Part_ConsoleEngineer:
		return new AuScConsoleEngineer;
	case Part_ConsoleFireControl:
		return new AuScConsoleFireControl;
	case Part_ConsoleHelm:
		return new AuScConsoleHelm;
	case Part_ConsoleMedic:
		return new AuScConsoleMedic;
	case Part_ConsoleNavigator:
		return new AuScConsoleNavigator;
	case Part_ConsolePowerRelay:
		return new AuScConsolePowerRelay;
	case Part_ConsoleSensor:
		return new AuScConsoleSensor;
	case Part_ConsoleShield:
		return new AuScConsoleShield;
	case Part_ConsoleTransporter:
		return new AuScConsoleTransporter;
	case Part_TroopPost:
		return new AuScTroopPost;
	case Part_GroundVehicleParkingSpot:
		return new AuScGroundVehicleParkingSpot;
	case Part_SpacecraftParkingSpot:
		return new AuScSpacecraftParkingSpot;
	case Part_SpaceVehicleLauncher:
		return new AuScSpaceVehicleLauncher;
	case Part_SpaceVehicleParkingSpot:
		return new AuScSpaceVehicleParkingSpot;
	case Part_BerthCapt:
		return new AuScBerthCapt;
	case Part_BerthCrew:
		return new AuScBerthCrew;
	case Part_BerthOfficer:
		return new AuScBerthOfficer;
	case Part_BerthPassenger:
		return new AuScBerthPassenger;
	case Part_BerthTroop:
		return new AuScBerthTroop;
	case Part_GameTable:
		return new AuScGameTable;
	case Part_ContrailEmitter:
		return new AuScContrailEmitter;
	case Part_HeadlightLens_Deprecated:
		return new AuScHeadlightLens;
	case Part_NavLightLens:
		return new AuScNavLightLens;
	case Part_RotatingBeaconLens:
		return new AuScRotatingBeaconLens;
	case Part_StrobeLens:
		return new AuScStrobeLens;
	case Part_Turret:
		return new AuScTurret;
	case Part_PodBay:
		return new AuScPodBay;
	case Part_Sketch:
		return new AuScSketch;
	case Part_LaunchTubeJig:
		return new AuScLaunchTubeJig;
	case Part_CuttingJig:
		return new AuScCuttingJig;
	case Part_SpaceVehicleLandingSystem:
		return new AuScSpaceVehicleLandingSystem;
	case Part_LightBulb:
		return new AuScLightBulb;
	case Part_StatusWorldMap:
		return new AuScStatusWorldMap;
	case Part_Decal:
		return new AuScDecal;
	case Part_EmpireFlag:
		return new AuScEmpireFlag;
	case Part_EmpireName:
		return new AuScEmpireName;
	case Part_ShipName:
		return new AuScShipName;
	case Part_BerthName:
		return new AuScBerthName;
	case Part_CaptName:
		return new AuScCaptName;
	case Part_DecalJig:
		return new AuScDecalJig;
	case Part_EnvironmentVoid:
		return new AuScEnvironmentVoid;
	case Part_HullVoid:
		return new AuScHullVoid;
	case Part_Barrier:
		return new AuScBarrier;
	case Part_RoomVoid:
		return new AuScRoomVoid;
	case Part_PathWalk:
		return new AuScPathWalk;
	case Part_PathLadder:
		return new AuScPathLadder;
	case Part_WaterVehicleParkingSpot:
		return new AuScWaterVehicleParkingSpot;
	case Part_RocketParkingSpot:
		return new AuScRocketParkingSpot;
	case Part_CitizenPost:
		return new AuScCitizenPost;
	case Part_Fence:
		return new AuScFence;
	case Part_LivestockPost:
		return new AuScLivestockPost;
	case Part_Shrub:
		return new AuScShrub;
	case Part_Site:
		return new AuScSite;
	case Part_Tree:
		return new AuScTree;
	case Part_LightLens:
		return new AuScLightLens;
	case Part_Camera:
		return new AuScCamera;
	case Part_BuildZone:
		return new AuScBuildZone;
	}
	qWarning() << "error: AuScPart::Create type" << type << "invalid";
	return 0;
}

// Returns the system associated with a part type that can have a module installed.
//static
SYSTEM AuScPart::ModuleSystem(AuScPart::PART_TYPE partType)
{
	switch((PART_TYPE)partType)	// cast gives me a warning if I forget to add a case here.
	{
	case Part_StationEngineer:
	case Part_ConsoleEngineer:
		return SYS_FTLDrive;
	case Part_StationFireControl:
	case Part_ConsoleFireControl:
		return SYS_Weapon;
	case Part_StationMedic:
	case Part_ConsoleMedic:
		return SYS_SurgeryUnit;
	case Part_StationSensor:
	case Part_ConsoleSensor:
		return SYS_Sensor;
	case Part_StationShield:
	case Part_ConsoleShield:
		return SYS_Shield;
	case Part_StationTransporter:
	case Part_ConsoleTransporter:
		return SYS_Transporter;
	case Part_Turret:
		return SYS_Turret;

	case Part_Invalid:
	case PART_Count:
	case Part_Barrier:
	case Part_BerthCapt:
	case Part_BerthCrew:
	case Part_BerthName:
	case Part_BerthOfficer:
	case Part_BerthPassenger:
	case Part_BerthTroop:
	case Part_BuildZone:
	case Part_Camera:
	case Part_CaptName:
	case Part_CitizenPost:
	case Part_ConsoleCommand:
	case Part_ConsoleHelm:
	case Part_ConsoleNavigator:
	case Part_ConsolePowerRelay:
	case Part_ContrailEmitter:
	case Part_CuttingJig:
	case Part_Decal:
	case Part_DecalJig:
	case Part_Door:
	case Part_DoorJig:
	case Part_EmpireFlag:
	case Part_EmpireName:
	case Part_EnvironmentVoid:
	case Part_Fence:
	case Part_GameTable:
	case Part_GroundVehicleParkingSpot:
	case Part_HeadlightLens_Deprecated:
	case Part_Hull:
	case Part_HullVoid:
	case Part_LandingGear:
	case Part_LaunchTubeJig:
	case Part_Lift:
	case Part_LightBulb:
	case Part_LivestockPost:
	case Part_Mesh:
	case Part_NavLightLens:
	case Part_PanelCapacitor:
	case Part_PanelFTLDrive:
	case Part_PanelLifeSupport:
	case Part_PanelManeuverDrive:
	case Part_PanelPowerPlant:
	case Part_PanelSensor:
	case Part_PanelShield:
	case Part_PanelWeapon:
	case Part_PathLadder:
	case Part_PathWalk:
	case Part_PodBay:
	case Part_RocketParkingSpot:
	case Part_Room:
	case Part_RoomVoid:
	case Part_RotatingBeaconLens:
	case Part_ShipName:
	case Part_Shrub:
	case Part_Site:
	case Part_Sketch:
	case Part_SpacecraftParkingSpot:
	case Part_SpaceVehicleLandingSystem:
	case Part_SpaceVehicleLauncher:
	case Part_SpaceVehicleParkingSpot:
	case Part_StationCommand:
	case Part_StationDesigner:
	case Part_StationHelm:
	case Part_StationNavigator:
	case Part_StationPowerRelay:
	case Part_StatusAlert:
	case Part_StatusLifeSupport:
	case Part_StatusMission:
	case Part_StatusWorldMap:
	case Part_LightLens:
	case Part_StrobeLens:
	case Part_Tree:
	case Part_TroopPost:
	case Part_WaterVehicleParkingSpot:
	case Part_Window:
	case Part_WindowJig:
		break;
	};
	return SYS_Count;
}

AuScPart::AuScPart()
{
	m_Bits = 0;
	m_StreamVersion = 0;
}

AuScPart::~AuScPart()
{
}

AuScPart::AuScPart(const AuScPart& c)
{
	m_ArchId = c.m_ArchId;
	m_ArchName = c.m_ArchName;
	m_Bits = c.m_Bits;
	m_Group = c.m_Group;
	m_StreamVersion = c.m_StreamVersion;
}

void AuScPart::operator>>(QDataStream& out) const
{
	out << (quint8)((m_Bits & 0x0f) | 0xf0);	// Top nibble used to be a version. At 0xf0 the following version byte was added.
	out << (quint8)PART_SERIALIZE_VER;
	out << m_ArchId;
	out << m_ArchName;
	out << m_Group;
}

quint8 AuScPart::ReadVersion() const
{
	return m_StreamVersion;
	//return m_Bits >> 4;
}

void AuScPart::operator<<(QDataStream& in)
{
	in >> m_Bits;	// Top nibble contains version.
	if((m_Bits >> 4) == 0x0f)
	{
		// Version hit 15 (0x0f) so a version byte was added.
		in >> m_StreamVersion;
	}
	else
	{
		if((m_Bits >> 4) == 0x07)
		{
			// Due to a bug, version 5 is read back as a 7 sometimes.
			// We never actually wrote a version 7 so we can safely change it back to a 5.
			m_Bits = 0x50 | (m_Bits & 0x0f);
		}
		m_StreamVersion = m_Bits >> 4;	// Get the stream version from the bits byte.
	}
	m_Bits &= 0x0f;		// Turn off the top nibble.

	if(ReadVersion() >= 0x03)
	{
		in >> m_ArchId;
		in >> m_ArchName;

		// Human architect keeps creeping in. I think it is due to the clipboard.
		if(m_ArchName == "Human")
		{
			m_ArchId = AuId();
			m_ArchName = QString();
		}
	}

	if(ReadVersion() >= 0x10)
	{
		in >> m_Group;
	}
}

bool AuScPart::operator==(const AuScPart& c) const
{
	if(m_ArchId != c.m_ArchId)
		return false;
	if(m_ArchName != c.m_ArchName)
		return false;
	if(m_Bits != c.m_Bits)
		return false;
	if(m_Group != c.m_Group)
		return false;
	return true;
}

QByteArray AuScPart::Pack() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)Type();
	out << *this;
	return ba;
}

//static
AuScPart* AuScPart::Unpack(const QByteArray& ba)
{
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 type;
	in >> type;
	if(!type)
	{
		qWarning() << "error: AuScPart::Unpack read type 0";
		return 0;
	}
	AuScPart* s = Create((PART_TYPE)type);	// Spits out a warning if type is invalid.
	if(!type)
		return 0;
	in >> *s;
	return s;
}

void AuScPart::Transform(const AuMatrix4x4& t)
{
	Q_UNUSED(t);
}

void AuScPart::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	Q_UNUSED(t);
	Q_UNUSED(selInfo);
}

void AuScPart::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	Q_UNUSED(t);
	Q_UNUSED(selInfo);
}

void AuScPart::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	Q_UNUSED(opaqueTris);
	Q_UNUSED(transparentTris);
	Q_UNUSED(themeColors);
}

void AuScPart::RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count], quint8 state) const
{
	Q_UNUSED(opaqueTris);
	Q_UNUSED(transparentTris);
	Q_UNUSED(themeColors);
	Q_UNUSED(state);
}

bool AuScPart::CompareAssociatedDoorState(quint8 doorState) const
{
	// Consider doors that are locked or jammed.
	quint8 state = AssociatedPartState();
	if(state == AuScDoor::Closed)
		return doorState != AuScDoor::Open && doorState != AuScDoor::StuckOpen && doorState != AuScDoor::LockedOpen;
	else if(state == AuScDoor::Open)
		return doorState == AuScDoor::Open || doorState == AuScDoor::StuckOpen || doorState == AuScDoor::LockedOpen;
	else
		return state == doorState;
}

bool AuScPart::CompareAssociatedLandingGearState(quint8 landingGearState) const
{
	// Consider landing gear that is stuck.
	quint8 state = AssociatedPartState();
	if(state == AuScLandingGear::Extended)
		return landingGearState == AuScLandingGear::Extended || landingGearState == AuScLandingGear::StuckExtended;
	else if(state == AuScLandingGear::Retracted)
		return landingGearState != AuScLandingGear::Extended && landingGearState != AuScLandingGear::StuckExtended;
	else
		return state == landingGearState;
}

QString AuScPart::FullName(const AuScParts& parts) const
{
	if(!AssociatedPartId())
		return PartName();

	AuScParts::const_iterator api = parts.find(AssociatedPartId());
	if(api == parts.cend())
		return PartName();

	return QString("%1 %2, %3")
		.arg((*api)->PartName())
		.arg(api.key())
		.arg(PartName());
}

quint32 AuScPart::CountFaces() const
{
	AuScFaceList faces;
	Faces(faces);
	return faces.size();
}

quint32 AuScPart::CountTriangles() const
{
	QColor themeColors[AuScFace::THEME_Count] =
	{
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
	};
	AuScTriRenderList opaqueTris;
	AuScTriRenderList transparentTris;

	AuScFaceList faces;
	Faces(faces);
	faces.RenderTriangles(opaqueTris, transparentTris, themeColors);
	return opaqueTris.size() + transparentTris.size();
}

quint64 AuScPart::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_ArchId);
	mem += StringMemoryUsage(m_ArchName);
	mem += sizeof(m_Bits);
	mem += StringMemoryUsage(m_Group);
	mem += sizeof(m_StreamVersion);
	return mem;
}

//static
const char* AuScPart::GroupNameHull()
{
	return d_GroupHull;
}

//static
const char* AuScPart::GroupNameFencePost()
{
	return d_GroupPost;
}

//static
const char* AuScPart::GroupNameFenceSection()
{
	return d_GroupSection;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScParts::AuScParts()
{
	m_PartId = 0;
}

AuScParts::AuScParts(const AuScParts& c) : QHash<quint16, AuScPart*>(c)
{
	m_PartId = c.m_PartId;
	for(const_iterator si = c.cbegin(); si != c.cend(); si++)
		insert(si.key(), (*si)->Clone());
}

const AuScParts& AuScParts::operator=(const AuScParts& c)
{
	m_PartId = c.m_PartId;
	Clear();
	for(const_iterator si = c.cbegin(); si != c.cend(); si++)
		insert(si.key(), (*si)->Clone());
	return *this;
}

void AuScParts::operator>>(QDataStream& out) const
{
	quint16 c = size();

	out << c;

	for(const_iterator si = cbegin(); si != cend(); si++)
	{
		out << si.key();
		out << (quint8)(*si)->Type();
		out << *(*si);
	}
}

void AuScParts::operator<<(QDataStream& in)
{
	Clear();

	quint16 c;

	in >> c;

	quint8 readVersion = 0;
	quint16 id;
	quint8 type;
	AuScPart* s;
	while(c--)
	{
		in >> id;
		in >> type;
		s = AuScPart::Create((AuScPart::PART_TYPE)type);
		if(s)
		{
			//qDebug() << "AuScParts::operator<<" << id << type << s->TypeName();
			in >> *s;
			insert(id, s);
			if(!readVersion)
				readVersion = s->ReadVersion();
		}
		else
			qWarning() << "error: AuScParts::operator<< invalid part type" << type << "id" << id << "c" << c;
	}

	// Set next part id based on current highest part id.
	m_PartId = HighestPartId() + 1;

	// Must finish the transition to group names. Find meshes and change their "AuScMesh:partId" group name into a
	// group name. 'partId' is the room they were associated with, for lighting. That is what a group does now.
	for(iterator pi = begin(); pi != end(); pi++)
		(*pi)->PostReadCleanup(*this);
}

void AuScParts::Clear()
{
	for(iterator pi = begin(); pi != end(); pi++)
		delete *pi;
	clear();
}

void AuScParts::Transform(const AuMatrix4x4& t)
{
	for(iterator pi = begin(); pi != end(); pi++)
		(*pi)->Transform(t);
}

quint16 AuScParts::HighestPartId() const
{
	quint16 highId = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if(pi.key() > highId)
			highId = pi.key();
	return highId;
}

quint16 AuScParts::AvailablePartId()
{
	// 64k parts is plenty for our needs.
	if(size() >= 0xfffe)
		return 0;

	// Find an unused part id, starting with the next one after where we left off last time.
	// The count tells us a part number is available in there somewhere.
	// We don't just restart at the beginning every time because of undo/redo and other issues
	// After we wrap around at the end, we could end up hunting for a hole; it's there.
	bool nextInUse;
	do
	{
		// Bump to the next part id.
		m_PartId++;						// This could roll over to 0.
		if(!m_PartId || m_PartId == 0xffff)
			m_PartId = 1;				// Skip 0 and 0xffff, which was used a lot by the old designs as an invalid id.
		nextInUse = contains(m_PartId);	// Check to see if the next id is in use.
	}
	while(nextInUse);
	return m_PartId;
}

quint16 AuScParts::AddPart(const AuScPart& s, AuScParts& partsAdded)
{
	if(!s.IsValid())
		return 0;

	quint16 id = AvailablePartId();
	if(!id)
		return 0;

	AuScPart* sc = s.Clone();
	if(!sc)
		return 0;

	// Store added part, for undo.
	partsAdded.insert(id, sc->Clone());

	insert(id, sc);
	return id;
}

quint16 AuScParts::AddPart(const AuScPart& s)
{
	if(!s.IsValid())
		return 0;

	quint16 id = AvailablePartId();
	if(!id)
		return 0;

	AuScPart* sc = s.Clone();
	if(!sc)
		return 0;

	insert(id, sc);
	return id;
}

bool AuScParts::DeletePart(quint16 partId, AuScParts& partsDeleted)
{
	iterator hi = find(partId);
	if(hi == end())
		return false;

	if((*hi)->IsLocked() || (*hi)->IsHidden())
		return false;

	// Store deleted part prior to erase, for undo.
	partsDeleted.insert(partId, *hi);

	erase(hi);
	return true;
}

bool AuScParts::AddPartObjects(quint16 partId, const AuScPart& s, AuScParts& partsChanged)
{
	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	if((*hi)->IsLocked() || (*hi)->IsHidden())
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(partId, (*hi)->Clone());

	// Copy faces from 's' to 'hi'. We don't really copy edges or vertices, just whole faces.
	AuScFaceList faces;
	s.Faces(faces);
	(*hi)->AddFaces(faces);
	return true;
}

bool AuScParts::DeletePartObjects(quint16 partId, const AuScSelectInfo& selInfo, AuScParts& partsChanged, AuScParts& partsDeleted)
{
	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	if((*hi)->IsLocked() || (*hi)->IsHidden())
		return false;

	// Grab clone prior to change, for undo.
	AuScPart* c = (*hi)->Clone();

	(*hi)->DeleteFaces(selInfo.m_Faces);
	if(!(*hi)->IsValid())
	{
		partsDeleted.insert(partId, c);
		delete *hi;
		erase(hi);
	}
	else
		partsChanged.insert(partId, c);
	return true;
}

bool AuScParts::UpdatePart(quint16 partId, const AuScPart& s, AuScParts& partsUpdated)
{
	iterator hi = find(partId);
	if(hi == end())
		return false;

	if((*hi)->IsLocked() || (*hi)->IsHidden())
		return false;

	// Store clone of part prior to update, for undo.
	partsUpdated.insert(partId, (*hi)->Clone());

	(*hi)->Update(s);
	return true;
}

quint16 AuScParts::FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, bool findVertices, bool findEdges,
		bool findFaces) const
{
	if(empty())
		return 0;

	quint16 partId;

	// Check faces first because they have the least things to find.
	if(findFaces)
	{
		partId = FindFaceAtPixel(mvp, pixel);
		if(partId)
			return partId;	// Found a face.
	}

	// No faces were found in the search radius. Search vertices.
	if(findVertices)
	{
		partId = FindVertexAtPixel(mvp, pixel);
		if(partId)
			return partId;	// Found a vertex.
	}

	// No vertices were found in the search radius. Search edge lines.
	if(findEdges)
	{
		partId = FindEdgeAtPixel(mvp, pixel);
		if(partId)
			return partId;	// Found an edge.
	}

	return 0;
}

quint16 AuScParts::FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds,
		bool findVertices, bool findEdges, bool findFaces) const
{
	if(empty())
		return 0;

	quint16 partId;

	// Check faces first because they have the least things to find.
	if(findFaces)
	{
		partId = FindFaceAtPixel(mvp, pixel, partIds);
		if(partId)
			return partId;	// Found a face.
	}

	// No faces were found in the search radius. Search vertices.
	if(findVertices)
	{
		partId = FindVertexAtPixel(mvp, pixel, partIds);
		if(partId)
			return partId;	// Found a vertex.
	}

	// No vertices were found in the search radius. Search edge lines.
	if(findEdges)
	{
		partId = FindEdgeAtPixel(mvp, pixel, partIds);
		if(partId)
			return partId;	// Found an edge.
	}

	return 0;
}

void AuScParts::FindPartsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices,
		bool findEdges, bool findFaces) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei)->InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces))
			partIds.insert(ei.key(), AuScSelectInfo());
}

quint16 AuScParts::FindEdgeAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	if(empty())
		return 0;

	quint16 closestId = 0;
	double closestDist = d_SearchTolerance;	// We won't find any shape that is not closer than this initial value.
	if(snapLoc)
	{
		AuPoint eloc;
		for(const_iterator ei = cbegin(); ei != cend(); ei++)
		{
			double dist = (*ei)->EdgeDistance(mvp, pixel, &eloc);
			if(dist >= 0 && dist < closestDist)
			{
				closestId = ei.key();
				closestDist = dist;
				*snapLoc = eloc;
			}
		}
	}
	else
	{
		for(const_iterator ei = cbegin(); ei != cend(); ei++)
		{
			double dist = (*ei)->EdgeDistance(mvp, pixel, 0);
			if(dist >= 0 && dist < closestDist)
			{
				closestId = ei.key();
				closestDist = dist;
			}
		}
	}

	return closestId;
}

quint16 AuScParts::FindEdgeAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds,
		AuPoint* snapLoc) const
{
	if(empty())
		return 0;

	quint16 closestId = 0;
	double closestDist = d_SearchTolerance;	// We won't find any shape that is not closer than this initial value.
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
	{
		if(!partIds.contains(ei.key()))
			continue;
		double dist = (*ei)->EdgeDistance(mvp, pixel, snapLoc);
		if(dist >= 0 && dist < closestDist)
		{
			closestId = ei.key();
			closestDist = dist;
		}
	}

	return closestId;
}

quint16 AuScParts::FindFaceAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	if(empty())
		return 0;

	quint32 findex = 0;
	quint16 closestId = 0;
	double closestDist = d_SearchTolerance;	// We won't find any part that is not closer than this initial distance.
	AuPoint eloc;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
	{
		double dist = (*ei)->FaceDistance(mvp, pixel, findex, eloc);
		if(dist >= 0 && dist < closestDist)
		{
			closestId = ei.key();
			closestDist = dist;
			if(snapLoc)
				*snapLoc = eloc;
		}
	}

	return closestId;
}

quint16 AuScParts::FindFaceAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds) const
{
	if(empty())
		return 0;

	quint32 findex = 0;
	quint16 closestId = 0;
	double closestDist = d_SearchTolerance;	// We won't find any part that is not closer than this initial distance.
	AuPoint eloc;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
	{
		if(!partIds.contains(ei.key()))
			continue;

		double dist = (*ei)->FaceDistance(mvp, pixel, findex, eloc);
		if(dist >= 0 && dist < closestDist)
		{
			closestId = ei.key();
			closestDist = dist;
		}
	}

	return closestId;
}

quint16 AuScParts::FindVertexAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	if(empty())
		return 0;

	quint32 vertexIndex = 0;
	quint16 closestId = 0;
	double closestDist = d_SearchTolerance;	// We won't find any part that is not closer than this initial distance.
	if(snapLoc)
	{
		AuPoint eloc;
		for(const_iterator ei = cbegin(); ei != cend(); ei++)
		{
			double dist = (*ei)->VertexDistance(mvp, pixel, vertexIndex, &eloc);
			if(dist >= 0 && dist < closestDist)
			{
				closestId = ei.key();
				closestDist = dist;
				*snapLoc = eloc;
			}
		}
	}
	else
	{
		for(const_iterator ei = cbegin(); ei != cend(); ei++)
		{
			double dist = (*ei)->VertexDistance(mvp, pixel, vertexIndex, 0);
			if(dist >= 0 && dist < closestDist)
			{
				closestId = ei.key();
				closestDist = dist;
			}
		}
	}

	return closestId;
}

quint16 AuScParts::FindVertexAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds,
		AuPoint* snapLoc) const
{
	if(empty())
		return 0;

	quint32 vertexIndex = 0;
	quint16 closestId = 0;
	double closestDist = d_SearchTolerance;	// We won't find any part that is not closer than this initial distance.
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
	{
		if(!partIds.contains(ei.key()))
			continue;
		double dist = (*ei)->VertexDistance(mvp, pixel, vertexIndex, snapLoc);
		if(dist >= 0 && dist < closestDist)
		{
			closestId = ei.key();
			closestDist = dist;
		}
	}

	return closestId;
}

void AuScParts::FindPartObjectsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices,
		bool findEdges, bool findFaces) const
{
	for(AuScSelection::iterator si = partIds.begin(); si != partIds.end(); si++)
	{
		const_iterator ei = find(si.key());
		if(ei != cend())
			(*ei)->InViewRectangle(mvp, pixelRect, *si, findVertices, findEdges, findFaces);
	}
}

void AuScParts::GroupNames(QStringList& groupNames) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if(!groupNames.contains((*ei)->GroupName()))
			groupNames.append((*ei)->GroupName());
}

void AuScParts::Ids(AuScSelection& partIds) const
{
	AuScSelectInfo selInfo;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		partIds.insert(ei.key(), selInfo);
}

void AuScParts::IdsNotOmitted(AuScSelection& partIds) const
{
	AuScSelectInfo selInfo;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if(!(*ei)->IsOmitted())
			partIds.insert(ei.key(), selInfo);
}

// Creates a custom MIME data object of the parts in 'partIds'. Coordinates are relative to 'designPlane' by
// translating the parts through the inverse of the designPlane.
QMimeData* AuScParts::CreateMIME(const AuId& architectId, bool objectMode, const AuScSelection& partIds,
		const AuMatrix4x4& designPlane) const
{
	if(objectMode)
	{
		// List the part objects that will be copied.
		AuScParts copyParts;
		for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
		{
			const_iterator ei = constFind(si.key());
			if(ei != cend())
			{
				quint16 id = copyParts.AvailablePartId();
				if(id)
				{
					AuScPart* p = (*ei)->CloneParts(*si);
					p->ClearArchitectForFileSave(architectId);
					p->Transform(designPlane.Inverse());
					copyParts.insert(id, p);
				}
			}
		}
		if(copyParts.empty())
			return 0;

		// Store the parts.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DESIGNER_MIME_PART_OBJECT_TYPE;
		out << (quint8)DESIGNER_MIME_PART_OBJECT_VER;
		out << copyParts;

		QMimeData* mimeData = new QMimeData;
		mimeData->setData("application/x-soh-mime;value=\"DesignerParts\"", ba);
		return mimeData;
	}
	else
	{
		// List the parts that will be copied.
		AuScParts copyParts;
		for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
		{
			const_iterator ei = constFind(si.key());
			if(ei != cend())
			{
				//quint16 id = copyParts.AvailablePartId();
				//if(id)
				{
					AuScPart* p = (*ei)->Clone();
					p->Transform(designPlane.Inverse());
					//copyParts.insert(id, p);
					copyParts.insert(si.key(), p);
				}
			}
		}
		if(copyParts.empty())
			return 0;

		// Store the parts.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DESIGNER_MIME_PART_TYPE;
		out << (quint8)DESIGNER_MIME_PART_VER;
		out << copyParts;

		QMimeData* mimeData = new QMimeData;
		mimeData->setData("application/x-soh-mime;value=\"DesignerParts\"", ba);
		return mimeData;
	}
}

// Returns a list of part that are read from 'mimeData'.
//static
void AuScParts::ReadMIME(const AuId& architectId, const QString& architectName, bool objectMode, const QMimeData& mimeData,
		AuScParts& parts)
{
	if(objectMode)
	{
		if(!mimeData.hasFormat("application/x-soh-mime;value=\"DesignerParts\""))
			return;

		QByteArray ba = mimeData.data("application/x-soh-mime;value=\"DesignerParts\"");
		if(ba.isEmpty())
			return;

		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);

		quint8 fileType, fileVer;
		in >> fileType;
		in >> fileVer;

		if(fileType != DESIGNER_MIME_PART_OBJECT_TYPE || fileVer != DESIGNER_MIME_PART_OBJECT_VER)
			return;

		in >> parts;
	}
	else
	{
		if(!mimeData.hasFormat("application/x-soh-mime;value=\"DesignerParts\""))
			return;

		QByteArray ba = mimeData.data("application/x-soh-mime;value=\"DesignerParts\"");
		if(ba.isEmpty())
			return;

		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);

		quint8 fileType, fileVer;
		in >> fileType;
		in >> fileVer;

		if(fileType != DESIGNER_MIME_PART_TYPE || fileVer != DESIGNER_MIME_PART_VER)
			return;

		in >> parts;
	}
	parts.SetArchitectAfterFileLoad(architectId, architectName);
}

//static
bool AuScParts::CheckMIME(bool objectMode, const QMimeData& mimeData)
{
	if(objectMode)
	{
		if(!mimeData.hasFormat("application/x-soh-mime;value=\"DesignerParts\""))
			return false;

		QByteArray ba = mimeData.data("application/x-soh-mime;value=\"DesignerParts\"");
		if(ba.isEmpty())
			return false;

		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);

		quint8 fileType, fileVer;
		in >> fileType;
		in >> fileVer;

		if(fileType != DESIGNER_MIME_PART_OBJECT_TYPE || fileVer != DESIGNER_MIME_PART_OBJECT_VER)
			return false;

		quint16 partCount;
		in >> partCount;
		if(!partCount)
			return false;

		// The MIME data appears to contain at least one part. We'll assume it has some objects (vertices, edges, faces).
		return true;
	}
	else
	{
		if(!mimeData.hasFormat("application/x-soh-mime;value=\"DesignerParts\""))
			return false;

		QByteArray ba = mimeData.data("application/x-soh-mime;value=\"DesignerParts\"");
		if(ba.isEmpty())
			return false;

		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);

		quint8 fileType, fileVer;
		in >> fileType;
		in >> fileVer;

		if(fileType != DESIGNER_MIME_PART_TYPE || fileVer != DESIGNER_MIME_PART_VER)
			return false;

		quint16 partCount;
		in >> partCount;
		if(!partCount)
			return false;

		// The MIME data appears to contain at least one part.
		return true;
	}
}

QString AuScParts::SelectedDesc(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return QString("p%1").arg(partId);

	return (*pi)->SelectedDesc(*this);
}

QString AuScParts::StopName(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return "";

	return (*pi)->StopName();
}

QString AuScParts::TurboLiftAtStop(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return "";

	return (*pi)->AtStop();
}

void AuScParts::Stops(quint16 partId, QStringList& stops) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return;

	stops = (*pi)->StopNames();
}

// Adds parts and their ids.
void AuScParts::UndoRedoAdd(const AuScParts& parts)
{
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		if(!contains(pi.key()))
			insert(pi.key(), (*pi)->Clone());
		else
			qWarning() << "error: AuScParts::UndoRedoAdd cannot restore part" << (*pi)->SelectedDesc(*this)
				<< "id" << pi.key() << "is in use.";
}

// Deletes parts by id.
void AuScParts::UndoRedoDelete(const QList<quint16>& partIds)
{
	for(QList<quint16>::const_iterator pi = partIds.cbegin(); pi != partIds.cend(); pi++)
	{
		iterator hi = find(*pi);
		if(hi != end())
		{
			delete *hi;
			erase(hi);
		}
		else
			qWarning() << "error: AuScParts::UndoRedoDelete cannot delete part id" << *pi;
	}
}

// Swaps parts 1:1 by id.
void AuScParts::UndoRedoSwap(AuScParts& swapParts)
{
	for(AuScParts::iterator si = swapParts.begin(); si != swapParts.end(); si++)
	{
		iterator pi = find(si.key());
		if(pi != end())
		{
			// Swap the parts.
			AuScPart* part = *pi;
			*pi = *si;
			*si = part;
		}
		else
			qWarning() << "error: AuScParts::UndoRedoSwap cannot swap part" << (*si)->SelectedDesc(*this)
				<< "id" << si.key() << "does not correspond to any part.";
	}
}

bool AuScParts::SelectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
		bool findEdges, bool findFaces) const
{
	if(empty())
		return false;
	if(sel.empty())
		return false;

	// Vertices
	quint16 closestVertexPartId = 0;
	double closestVertexDist = d_SearchTolerance;
	AuPoint closestVertexLoc;
	if(findVertices)
	{
		AuPoint vertLoc;
		for(AuScSelection::const_iterator si = sel.begin(); si != sel.end(); si++)
		{
			const_iterator ei = find(si.key());
			if(ei == cend())
				continue;

			quint32 vertexIndex;
			double dist = (*ei)->VertexDistance(mvp, pixel, vertexIndex, &vertLoc);
			if(dist >= 0 && dist < closestVertexDist)
			{
				closestVertexPartId = ei.key();
				closestVertexDist = dist;
				closestVertexLoc = vertLoc;
			}
		}
	}

	// Faces
	quint16 closestFacePartId = 0;
	double closestFaceDist = d_SearchTolerance;
	AuPoint closestFaceLoc;
	if(findFaces)
	{
		AuPoint faceLoc;
		for(AuScSelection::const_iterator si = sel.begin(); si != sel.end(); si++)
		{
			const_iterator ei = find(si.key());
			if(ei == cend())
				continue;

			quint32 vertexIndex;
			double dist = (*ei)->FaceDistance(mvp, pixel, vertexIndex, faceLoc);
			if(dist >= 0 && dist < closestFaceDist)
			{
				closestFacePartId = ei.key();
				closestFaceDist = dist;
				closestFaceLoc = faceLoc;
			}
		}
	}

	// Edges. Only look for them if we did not find a vertex or face grip.
	quint16 closestEdgePartId = 0;
	double closestEdgeDist = d_SearchTolerance;
	AuSegment closestEdgeSeg;
	if(findEdges && !closestVertexPartId && !closestFacePartId)
	{
		AuSegment edgeSeg;
		for(AuScSelection::const_iterator si = sel.begin(); si != sel.end(); si++)
		{
			const_iterator ei = find(si.key());
			if(ei == cend())
				continue;

			double dist = (*ei)->EdgeDistance(mvp, pixel, edgeSeg);
			if(dist >= 0 && dist < closestEdgeDist)
			{
				closestEdgePartId = ei.key();
				closestEdgeDist = dist;
				closestEdgeSeg = edgeSeg;
			}
		}
	}

	// We will select the closest vertex, edge or face.
	bool anyChange = false;
	if(closestVertexDist < closestEdgeDist && closestVertexDist < closestFaceDist)
	{
		// Select the closest vertex.
		// We know the location of the closest vertex. Many vertices can overlap there. We must select them all.
		const_iterator ei = find(closestVertexPartId);
		if(ei != cend())
		{
			AuScSelection::iterator si = sel.find(closestVertexPartId);
			if(si != sel.end() && (*ei)->SelectVerticesAt(closestVertexLoc, *si))
				anyChange = true;
		}
	}
	else if(closestEdgeDist < closestFaceDist)
	{
		// Select the closest edge.
		// We know the end points of the closest edge. Many edges can overlap there. We must select them all.
		const_iterator ei = find(closestEdgePartId);
		if(ei != cend())
		{
			AuScSelection::iterator si = sel.find(closestEdgePartId);
			if(si != sel.end() && (*ei)->SelectEdgesAt(closestEdgeSeg, *si))
				anyChange = true;
		}
	}
	else if(closestFaceDist < d_SearchTolerance)
	{
		// Select the closest face. There should be only one.
		const_iterator ei = find(closestFacePartId);
		if(ei != cend())
		{
			AuScSelection::iterator si = sel.find(closestFacePartId);
			if(si != sel.end() && (*ei)->SelectFaceAt(closestFaceLoc, *si))
				anyChange = true;
		}
	}

	return anyChange;
}

bool AuScParts::SelectContiguous(double angleTol, AuScSelection& sel) const
{
	if(empty())
		return false;
	if(sel.empty())
		return false;

	bool anySel = false;
	for(AuScSelection::iterator si = sel.begin(); si != sel.end(); si++)
	{
		const_iterator ei = find(si.key());
		if(ei == cend())
			continue;
		if((*ei)->SelectContiguous(angleTol, *si))
			anySel = true;
	}
	return anySel;
}

bool AuScParts::SelectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const
{
	if(empty())
		return false;
	if(sel.empty())
		return false;

	AuScSelection::iterator si = sel.find(partId);
	if(si == sel.end())
		return false;	// partId is not selected.

	const_iterator pi = find(partId);
	if(pi == cend())
		return false;	// partId does not exist? D:

	AuScPoint vertLoc = (*pi)->VertexLoc(partVertex);

	if(!(*pi)->SelectVerticesAtTextureCoord(QPointF(vertLoc.m_Tx, vertLoc.m_Ty), *si))
		return false;

	return true;
}

bool AuScParts::DeselectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
		bool findEdges, bool findFaces) const
{
	if(empty())
		return false;
	if(sel.empty())
		return false;

	// Vertices
	quint16 closestVertexPartId = 0;
	double closestVertexDist = d_SearchTolerance;
	AuPoint closestVertexLoc;
	if(findVertices)
	{
		AuPoint vertLoc;
		for(AuScSelection::const_iterator si = sel.begin(); si != sel.end(); si++)
		{
			const_iterator ei = find(si.key());
			if(ei == cend())
				continue;

			quint32 vertexIndex;
			double dist = (*ei)->VertexDistance(mvp, pixel, vertexIndex, &vertLoc);
			if(dist >= 0 && dist < closestVertexDist)
			{
				closestVertexPartId = ei.key();
				closestVertexDist = dist;
				closestVertexLoc = vertLoc;
			}
		}
	}

	// Faces
	quint16 closestFacePartId = 0;
	double closestFaceDist = d_SearchTolerance;
	AuPoint closestFaceLoc;
	if(findFaces)
	{
		AuPoint faceLoc;
		for(AuScSelection::const_iterator si = sel.begin(); si != sel.end(); si++)
		{
			const_iterator ei = find(si.key());
			if(ei == cend())
				continue;

			quint32 vertexIndex;
			double dist = (*ei)->FaceDistance(mvp, pixel, vertexIndex, faceLoc);
			if(dist >= 0 && dist < closestFaceDist)
			{
				closestFacePartId = ei.key();
				closestFaceDist = dist;
				closestFaceLoc = faceLoc;
			}
		}
	}

	// Edges. Only look for them if we did not find a vertex or face grip.
	quint16 closestEdgePartId = 0;
	double closestEdgeDist = d_SearchTolerance;
	AuSegment closestEdgeSeg;
	if(findEdges && !closestVertexPartId && !closestFacePartId)
	{
		AuSegment edgeSeg;
		for(AuScSelection::const_iterator si = sel.begin(); si != sel.end(); si++)
		{
			const_iterator ei = find(si.key());
			if(ei == cend())
				continue;

			double dist = (*ei)->EdgeDistance(mvp, pixel, edgeSeg);
			if(dist >= 0 && dist < closestEdgeDist)
			{
				closestEdgePartId = ei.key();
				closestEdgeDist = dist;
				closestEdgeSeg = edgeSeg;
			}
		}
	}

	// We will select the closest vertex, edge or face.
	bool anyChange = false;
	if(closestVertexDist < closestEdgeDist && closestVertexDist < closestFaceDist)
	{
		// Deselect the closest vertex.
		// We know the location of the closest vertex. Many vertices can overlap there. We must select them all.
		const_iterator ei = find(closestVertexPartId);
		if(ei != cend())
		{
			AuScSelection::iterator si = sel.find(closestVertexPartId);
			if(si != sel.end() && (*ei)->DeselectVerticesAt(closestVertexLoc, *si))
				anyChange = true;
		}
	}
	else if(closestEdgeDist < closestFaceDist)
	{
		// Deselect the closest edge.
		// We know the end points of the closest edge. Many edges can overlap there. We must select them all.
		const_iterator ei = find(closestEdgePartId);
		if(ei != cend())
		{
			AuScSelection::iterator si = sel.find(closestEdgePartId);
			if(si != sel.end() && (*ei)->DeselectEdgesAt(closestEdgeSeg, *si))
				anyChange = true;
		}
	}
	else if(closestFaceDist < d_SearchTolerance)
	{
		// Deselect the closest face. There should be only one.
		const_iterator ei = find(closestFacePartId);
		if(ei != cend())
		{
			AuScSelection::iterator si = sel.find(closestFacePartId);
			if(si != sel.end() && (*ei)->DeselectFacesAt(closestFaceLoc, *si))
				anyChange = true;
		}
	}

	return anyChange;
}

bool AuScParts::DeselectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const
{
	if(empty())
		return false;
	if(sel.empty())
		return false;

	AuScSelection::iterator si = sel.find(partId);
	if(si == sel.end())
		return false;	// partId is not selected.

	const_iterator pi = find(partId);
	if(pi == cend())
		return false;	// partId does not exist? D:

	AuScPoint vertLoc = (*pi)->VertexLoc(partVertex);

	if(!(*pi)->DeselectVerticesAtTextureCoord(QPointF(vertLoc.m_Tx, vertLoc.m_Ty), *si))
		return false;

	return true;
}

bool AuScParts::JoinParts(quint16 targetPart, const AuScSelection& sel, AuScParts& partsChanged, AuScParts& partsDeleted)
{
	if(sel.size() < 2)
		return false;

	iterator ti = find(targetPart);
	if(ti == end())
		return false;

	if((*ti)->IsLocked() || (*ti)->IsHidden())
		return false;

	partsChanged.insert(targetPart, (*ti)->Clone());

	// Join the parts of the selection to the target part.
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Skip the target part.
		if(si.key() == targetPart)
			continue;

		// Find the part we are joining into the target.
		iterator ji = find(si.key());
		if(ji == end())
			continue;

		if((*ji)->IsLocked() || (*ji)->IsHidden())
			continue;

		// Some parts cannot be joined to other parts.
		if(!(*ji)->CanJoinFaces(*ti))
			continue;

		// Add its faces to the target.
		AuScFaceList faces;
		(*ji)->Faces(faces);
		(*ti)->AddFaces(faces);

		// Move the absorbed part to the deleted undo list.
		partsDeleted.insert(ji.key(), *ji);
		erase(ji);
	}

	return true;
}

bool AuScParts::JigCutParts(const AuId& archId, const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged,
		AuScParts& partsDeleted)
{
	// Locate the parts involved.
	quint16 cut1id = 0;
	AuScMesh* cut1 = 0;
	quint16 cut2id = 0;
	AuScMesh* cut2 = 0;
	AuScMesh* jig = 0;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator fi = find(si.key());
		if(fi == end())
			continue;

		// A turbo lift provides both a jig and a room mesh.
		if((*fi)->IsJig())
			jig = (AuScMesh*)(*fi);

		// Cannot cut parts sealed by another architect.
		if((*fi)->IsSealed() && (*fi)->ArchId() != archId && !(*fi)->ArchId().IsNull())
			continue;

		if((*fi)->IsHullMesh() || (*fi)->IsRoomMesh() || (*fi)->IsTurboLiftMesh())
		{
			if(!cut1)
			{
				cut1id = fi.key();
				cut1 = (AuScMesh*)(*fi);
			}
			else
			{
				cut2id = fi.key();
				cut2 = (AuScMesh*)(*fi);
			}
		}
	}

	// We cannot continue unless we find the jig and at least one hull or room part.
	if(cut1 == cut2)
		return false;	// wtf
	if( ! (jig && (cut1 || cut2)))
		return false;	// We need a jig and at least one part to cut.

	// Test locked/visibility of parts to be cut.
	if(jig->IsHidden())
		return false;
	if(cut1 && (cut1->IsLocked() || cut1->IsHidden()))
		return false;
	if(cut2 && (cut2->IsLocked() || cut2->IsHidden()))
		return false;

	if(jig->IsLaunchTubeJig())
	{
		// Launch tube jig requires two parts to cut.
		if(!cut1 || !cut2)
			return false;

		// Launch tube jig is picky, wants to cut only a room and a hull.
		if( ! ((cut1->IsRoomMesh() && cut2->IsHullMesh()) ||
				(cut1->IsHullMesh() && cut2->IsRoomMesh())))
			return false;
	}

	AuScFaceList cut1Faces;
	if(cut1)
		cut1->JigCutFaces(cut1Faces);

	AuScFaceList cut2Faces;
	if(cut2)
		cut2->JigCutFaces(cut2Faces);

	AuScFaceList jigFaces;
	jig->JigFaces(jigFaces);

	AuScFaceList lining;				// Faces on the sides of the plug that remains; they are facing inward.
	jigFaces.JigCut(cut1Faces, cut2Faces, lining);

	// Create parts from the result.
	int whoGetsTheLining = 0;	// 0=Nobody, 1=cut1, 2=cut2
	if(jig->IsDoorJig())
	{
		AuScFaceList doorFaces;
		cut1Faces.Subtract(jigFaces, doorFaces);
		cut2Faces.Subtract(jigFaces, doorFaces);
		doorFaces.JoinFaces();
		if(cut1 && cut2)
		{
			// Door jig builds the door using faces inside the jig and the inner lining.
			doorFaces.AddFaces(lining);

			// Create the closed state of the door mesh.
			if(!doorFaces.empty())
			{
				AuScDoor door;
				door.AddFaces(doorFaces);
				door.AddState(AuScDoor::Open);
				AddPart(door, partsAdded);
			}

			// Decide who gets the lining.
			// We prefer to give it to the hull because it could be rendered without the interior. We don't want to see gaps.
			if(cut1->IsHullMesh())
				whoGetsTheLining = 1;
			else
				whoGetsTheLining = 2;
		}
		else if(cut1)
		{
			AuScPart* cutout = cut1->CloneParts(AuScSelectInfo());	// Create a clone without any faces.
			cutout->AddFaces(doorFaces);
			AddPart(*cutout, partsAdded);
			delete cutout;

			// Decide who gets the lining.
			whoGetsTheLining = 1;
		}
		else if(cut2)
		{
			AuScPart* cutout = cut2->CloneParts(AuScSelectInfo());	// Create a clone without any faces.
			cutout->AddFaces(doorFaces);
			AddPart(*cutout, partsAdded);
			delete cutout;

			// Decide who gets the lining.
			whoGetsTheLining = 2;
		}
	}
	else if(jig->IsWindowJig())
	{
		// Window jig builds window glass faces from fragments inside the jig.
		AuScFaceList window1;
		cut1Faces.Subtract(jigFaces, window1);
		window1.JoinFaces();

		AuScFaceList window2;
		cut2Faces.Subtract(jigFaces, window2);
		window2.JoinFaces();

		// Create the window glass meshes as two separate parts so the person can delete unwanted panes.
		if(cut1 && cut2 && !window1.empty() && !window2.empty())
		{
			float opacity = .25f;		// Two panes were created so they can be transparent.
			if((cut1->IsRoomMesh() && cut2->IsHullMesh()) ||
					(cut1->IsHullMesh() && cut2->IsRoomMesh()))
			{
				// Going from hull to room or room to hull, same same.
				// Make one thin double-sided window at the hull side only.
				AuScWindow window;
				if(cut1->IsHullMesh())
				{
					// Window 1 is the hull side.
					window1.SmoothVertices();
					window.AddFaces(window1);

					window1.ReverseTriangleFacing();
					window1.ReverseVertexNormals();
					window.AddFaces(window1);
				}
				else
				{
					// Window 2 is the hull side.
					window2.SmoothVertices();
					window.AddFaces(window2);

					window2.ReverseTriangleFacing();
					window2.ReverseVertexNormals();
					window.AddFaces(window2);
				}
				window.SetOpacity(opacity);		// Sets opacity of all faces.
				AddPart(window, partsAdded);
			}
			else
			{
				// This could be room to room or hull to hull. In either case, we only need one-sided window panes.
				if(!window1.empty())
				{
					window1.SmoothVertices();
					AuScWindow window;
					window.AddFaces(window1);
					window.SetOpacity(opacity);		// Sets opacity of all faces.
					AddPart(window, partsAdded);
				}
				if(!window2.empty())
				{
					window2.SmoothVertices();
					AuScWindow window;
					window.AddFaces(window2);
					window.SetOpacity(opacity);		// Sets opacity of all faces.
					AddPart(window, partsAdded);
				}
			}

			// Decide who gets the lining.
			// We prefer to give it to the hull because it could be rendered without the interior. We don't want to see gaps.
			if(cut1->IsHullMesh())
				whoGetsTheLining = 1;
			else
				whoGetsTheLining = 2;
		}
		else
		{
			float opacity = 1.f;		// When there is only one pane, it is opaque.
			if(!window1.empty())
			{
				window1.SmoothVertices();
				AuScWindow window;
				window.AddFaces(window1);
				window.SetOpacity(opacity);		// Sets opacity of all faces.
				AddPart(window, partsAdded);
			}
			if(!window2.empty())
			{
				window2.SmoothVertices();
				AuScWindow window;
				window.AddFaces(window2);
				window.SetOpacity(opacity);		// Sets opacity of all faces.
				AddPart(window, partsAdded);
			}

			// Decide who gets the lining.
			// We prefer to give it to the hull because it could be rendered without the interior. We don't want to see gaps.
			if(cut1 && cut2 && cut1->IsHullMesh())
				whoGetsTheLining = 1;
			else
				whoGetsTheLining = 2;
		}
	}
	else if(jig->IsLaunchTubeJig())
	{
		// Launch tube jig builds the door using hull faces inside the jig.
		AuScFaceList doorFaces;
		if(cut1->IsHullMesh())
		{
			// Subtract room faces first so we are left with only the faces from the hull for the door.
			cut2Faces.Subtract(jigFaces, doorFaces);
			doorFaces.Clear();
			cut1Faces.Subtract(jigFaces, doorFaces);

			// Decide who gets the lining.
			// We prefer to give it to the hull because it could be rendered without the interior. We don't want to see gaps.
			whoGetsTheLining = 1;
		}
		else
		{
			// Subtract room faces first so we are left with only the faces from the hull for the door.
			cut1Faces.Subtract(jigFaces, doorFaces);
			doorFaces.Clear();
			cut2Faces.Subtract(jigFaces, doorFaces);

			// Decide who gets the lining.
			whoGetsTheLining = 2;
		}
		doorFaces.JoinFaces();

		// Create the closed state of the door mesh.
		if(!doorFaces.empty())
		{
			// Build a two sided door.
			AuScDoor door;
			door.AddFaces(doorFaces);

			// Reverse the door faces and add them again for the other side.
			doorFaces.ReverseTriangleFacing();
			doorFaces.ReverseVertexNormals();
			door.AddFaces(doorFaces);
			door.AddState(AuScDoor::Open);
			AddPart(door, partsAdded);
		}
	}
	else if(cut1 && cut2 && (cut1->IsTurboLiftMesh() || cut2->IsTurboLiftMesh()))
	{
		// Turbo lift is cutting a door.
		AuScFaceList doorFaces;
		cut1Faces.Subtract(jigFaces, doorFaces);
		cut2Faces.Subtract(jigFaces, doorFaces);
		doorFaces.JoinFaces();

		// Door jig builds the door using faces inside the jig and the inner lining.
		doorFaces.AddFaces(lining);

		// Create the closed state of the door mesh.
		if(!doorFaces.empty())
		{
			AuScDoor door;
			door.AddFaces(doorFaces);
			if(cut1->IsTurboLiftMesh())
				door.SetStopName(cut1->AtStop());	// Name the door after the current turbo lift stop.
			else
				door.SetStopName(cut2->AtStop());	// Name the door after the current turbo lift stop.
			door.AddState(AuScDoor::Open);
			AddPart(door, partsAdded);
		}

		// Decide who gets the lining.
		// We prefer to give it to the room because it could be rendered without the turbo lift. We don't want to see gaps.
		// The turbo lift is always rendered at a stop, so it doesn't need the lining.
		if(cut1->IsTurboLiftMesh())
			whoGetsTheLining = 2;
		else
			whoGetsTheLining = 1;
	}
	else if(jig->IsDecalJig())
	{
		// Decal jig creates a decal without actually cutting the original surface.
		AuScFaceList cutoutFaces;
		cut1Faces.Subtract(jigFaces, cutoutFaces);
		cutoutFaces.JoinFaces();
		if(cut1)
		{
			AuScDecal cutout;
			cutout.AddFaces(cutoutFaces);
			AddPart(cutout, partsAdded);
		}
		return true;
	}
	else
	{
		AuScFaceList cutoutFaces;
		cut1Faces.Subtract(jigFaces, cutoutFaces);
		cut2Faces.Subtract(jigFaces, cutoutFaces);
		cutoutFaces.JoinFaces();
		if(cut1 && cut2)
		{
			// Door jig builds the door using faces inside the jig and the inner lining.
			cutoutFaces.AddFaces(lining);

			// Create the closed state of the door mesh.
			if(!cutoutFaces.empty())
			{
				AuScPart* cutout = cut1->CloneParts(AuScSelectInfo());	// Create a clone without any faces.
				cutout->AddFaces(cutoutFaces);
				AddPart(*cutout, partsAdded);
				delete cutout;
			}

			// Decide who gets the lining.
			// We prefer to give it to the hull because it could be rendered without the interior. We don't want to see gaps.
			// We prefer to give it to the room when the other is a turbo lift because the room could be rendered without the 
			// turbo lift. We don't want to see gaps.
			if(cut1->IsHullMesh())
				whoGetsTheLining = 1;
			else if(cut1->IsTurboLiftMesh())
				whoGetsTheLining = 2;
			else
				whoGetsTheLining = 1;
		}
		else if(cut1)
		{
			AuScPart* cutout = cut1->CloneParts(AuScSelectInfo());	// Create a clone without any faces.
			cutout->AddFaces(cutoutFaces);
			AddPart(*cutout, partsAdded);
			delete cutout;

			// Decide who gets the lining.
			whoGetsTheLining = 1;
		}
		else if(cut2)
		{
			AuScPart* cutout = cut2->CloneParts(AuScSelectInfo());	// Create a clone without any faces.
			cutout->AddFaces(cutoutFaces);
			AddPart(*cutout, partsAdded);
			delete cutout;

			// Decide who gets the lining.
			whoGetsTheLining = 2;
		}
	}

	// Clean up the cut faces now that the doors and windows have been cut out.
	cut1Faces.JoinFaces();
	cut2Faces.JoinFaces();

	// Line the hole.
	if(!lining.empty())
	{
		// Reverse triangle facing and flatten the normals for the hole lining.
		lining.ReverseTriangleFacing();
		lining.FlattenVertexNormals();

		if(whoGetsTheLining == 1)
			cut1Faces.AddFaces(lining);
		else if(whoGetsTheLining == 2)
			cut2Faces.AddFaces(lining);
	}

	// Change the cut parts.
	if(cut1Faces.empty())
	{
		// Cut part was entirely absorbed?
		DeletePart(cut1id, partsDeleted);
		cut1 = 0;
	}
	else if(cut1)
	{
		partsChanged.insert(cut1id, cut1->Clone());
		cut1->SetJigCutFaces(cut1Faces);
	}

	if(cut2Faces.empty())
	{
		// Cut part was entirely absorbed?
		DeletePart(cut2id, partsDeleted);
		cut2 = 0;
	}
	else if(cut2)
	{
		partsChanged.insert(cut2id, cut2->Clone());
		cut2->SetJigCutFaces(cut2Faces);
	}

	return true;
}

bool AuScParts::SplitParts(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged)
{
	AuScPart* splitMesh = 0;

	// Split the parts of the selection into the splitMesh.
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the face we are splitting.
		iterator fi = find(si.key());
		if(fi == end())
			continue;

		if((*fi)->IsLocked() || (*fi)->IsHidden())
			continue;

		// Add its selected faces to the splitMesh.
		AuScFaceList faces;
		(*fi)->Faces(faces, *si);
		if(!faces.empty())
		{
			if(!splitMesh)
			{
				splitMesh = (*fi)->Clone();
				splitMesh->SetFaces(faces);
			}
			else
				splitMesh->AddFaces(faces);

			// Make a copy of the face for undo.
			partsChanged.insert(fi.key(), (*fi)->Clone());

			// Remove joined faces from the selected part.
			(*fi)->DeleteFaces((*si).m_Faces);
		}
	}

	if(!splitMesh)
		return false;

	AddPart(*splitMesh, partsAdded);

	delete splitMesh;

	return true;
}

bool AuScParts::FractureFaces(const AuScSelection& sel, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	bool anyChange = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		if(!(*pi)->CanFracture())
			continue;

		AuScPart* c = (*pi)->Clone();

		if((*pi)->FractureFaces(*si))
		{
			partsChanged.insert(pi.key(), c);
			anyChange = true;
		}
		else
			delete c;
	}
	return anyChange;
}

bool AuScParts::FuseFaces(const AuScSelection& sel, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	bool anyChange = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		if(!(*pi)->CanFuse())
			continue;

		AuScPart* c = (*pi)->Clone();

		if((*pi)->FuseFaces(*si))
		{
			partsChanged.insert(pi.key(), c);
			anyChange = true;
		}
		else
			delete c;
	}
	return anyChange;
}

void AuScParts::ReverseFaces()
{
	for(iterator pi = begin(); pi != end(); pi++)
		(*pi)->ReverseFaces();
}

bool AuScParts::ReverseFaces(const AuScSelection& sel, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	// Reverse draw direction of the parts of the selection.
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->ReverseFaces(*si);
	}
	return true;
}

bool AuScParts::DivideFaces(const AuScSelection& sel, quint8 divisions, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;
	if(!divisions)
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->DivideFaces(*si, divisions);
	}
	return true;
}

bool AuScParts::PolishFaces(const AuScSelection& sel, float shine, float reflectivity, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->PolishFaces(*si, shine, reflectivity);
	}
	return true;
}

bool AuScParts::EffectIntensityVertices(const AuScSelection& sel, float intensity, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectIntensityVertices(*si, intensity);
	}
	return true;
}

bool AuScParts::GlowVertices(const AuScSelection& sel, const QColor& glow, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->GlowVertices(*si, glow);
	}
	return true;
}

bool AuScParts::PaintFaces(const AuScSelection& sel, const QColor& paint, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	// Prevent colors that are too dark.
	QColor col(paint);
	qreal h,s,v;
	col.getHsvF(&h, &s, &v);
	if(v < .05f)
	{
		col.setHsvF(h, s, .05f);
		col.setAlpha(paint.alpha());
	}

	// Reverse draw direction of the parts of the selection.
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->PaintFaces(*si, col);
	}
	return true;
}

bool AuScParts::PaintFaces(const AuScSelection& sel, AuScFace::COLOR_THEME theme, quint8 opacity, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	// Reverse draw direction of the parts of the selection.
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->PaintFaces(*si, theme, opacity);
	}
	return true;
}

bool AuScParts::ReverseVertexNormals()
{
	for(iterator pi = begin(); pi != end(); pi++)
		(*pi)->ReverseVertexNormals();
	return true;
}

void AuScParts::FixPartAssociations(const QHash<quint16, quint16>& nuPartIds)
{
	for(QHash<quint16, quint16>::const_iterator ni = nuPartIds.cbegin(); ni != nuPartIds.cend(); ni++)
	{
		// Find the nu copied part.
		iterator pi = find(*ni);
		if(pi == end())
			continue;	// wut?

		// Check for associations.
		quint16 aid = (*pi)->AssociatedPartId();
		if(!aid)
			continue;	// Nu part is not associated with any other part.

		// Check to see if the associated part is among the nu parts.
		QHash<quint16, quint16>::const_iterator ai = nuPartIds.find(aid);
		if(ai == nuPartIds.cend())
			continue;	// Associated part is not among the pasted parts.

		// The nu part is associated with another one of the nu parts.
		// Change the associated part id to the nu one, without changing the associated state.
		(*pi)->SetAssociatedPart(*ai, (*pi)->AssociatedPartState());
	}
}

void AuScParts::FlattenVertices()
{
	for(iterator pi = begin(); pi != end(); pi++)
		(*pi)->FlattenVertices();
}

bool AuScParts::FlattenVertices(const AuScSelection& sel, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->FlattenVertices(*si);
	}
	return true;
}

bool AuScParts::SmoothVertices(const AuScSelection& sel, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SmoothVertices(*si);
	}
	return true;
}

bool AuScParts::FillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	if(boundary.size() < 3)
		return false;

	// Fill the face on the first part in the selection.
	iterator pi = find(sel.begin().key());
	if(pi == end())
		return false;

	if((*pi)->IsLocked() || (*pi)->IsHidden())
		return false;

	// Transform the boundary to the real XY plane so the math is easier.
	AuPointArray cbnd(boundary);
	cbnd.Transform(gridPlane.Inverse());

	// Prepare boundary.
	if(!cbnd.IsClockwise())
		cbnd.ReverseOrder();

	// Triangulate the boundary with edge lines at the boundary.
	AuPointArray tris;
	QVector<bool> edges;
	cbnd.TriangulateBoundary(tris, edges);

	AuScFaceList faces;
	if(tris.size() == 3)
	{
		// Create a triangle face.
		AuScPoint v0(tris[0], 0,0, AuPoint(0,0,1));
		AuScPoint v1(tris[1], 0,0, AuPoint(0,0,1));
		AuScPoint v2(tris[2], 0,0, AuPoint(0,0,1));
		AuScFaceTri ft(v0, v1, v2);

		// Transform the result to the grid plane.
		ft.Transform(gridPlane);
		faces.AddFace(ft);
	}
	else if(tris.size() == 6)
	{
		AuScPoint v0(tris[0], 0,0, AuPoint(0,0,1));
		AuScPoint v1(tris[1], 0,0, AuPoint(0,0,1));
		AuScPoint v2(tris[2], 0,0, AuPoint(0,0,1));
		AuScFaceTri ft1(v0, v1, v2);

		AuScPoint v3(tris[3], 0,0, AuPoint(0,0,1));
		AuScPoint v4(tris[4], 0,0, AuPoint(0,0,1));
		AuScPoint v5(tris[5], 0,0, AuPoint(0,0,1));
		AuScFaceTri ft2(v3, v4, v5);

		AuScFace* fq = ft1.Join(ft2);
		if(fq)
		{
			// The two triangles joined together into something, hopefully a quad.
			// Transform the result to the grid plane.
			fq->Transform(gridPlane);
			faces.AddFace(*fq);
			delete fq;
		}
	}

	if(faces.empty())
	{
		// Create a polygon face.
		AuScFacePolygon fp(tris, edges);

		// Transform the result to the grid plane.
		fp.Transform(gridPlane);
		faces.AddFace(fp);
	}

	if(faces.empty())
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->AddFaces(faces);
	return true;
}

bool AuScParts::FillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3, const AuPoint& v4,
		AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	// Fill the face on the first part in the selection.
	iterator pi = find(sel.begin().key());
	if(pi == end())
		return false;

	if((*pi)->IsLocked() || (*pi)->IsHidden())
		return false;

	// Calculate surface normal.
	AuPoint vx(v4 - v1);
	vx.Unit();
	AuPoint vy(v2 - v1);
	vy.Unit();
	AuPoint n = vx.Cross(vy);
	n.Unit();

	AuScFaceList faces;

	// Create a face.
	AuScPoint p1(v1, 0,0, n);
	AuScPoint p2(v2, 0,0, n);
	AuScPoint p3(v3, 0,0, n);
	AuScPoint p4(v4, 0,0, n);
	AuScFaceQuad f(p1, p2, p3, p4);
	faces.AddFace(f);

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->AddFaces(faces);
	return true;
}

bool AuScParts::FillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
		AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	// Fill the face on the first part in the selection.
	iterator pi = find(sel.begin().key());
	if(pi == end())
		return false;

	if((*pi)->IsLocked() || (*pi)->IsHidden())
		return false;

	// Calculate surface normal.
	AuPoint vx(v3 - v1);
	vx.Unit();
	AuPoint vy(v2 - v1);
	vy.Unit();
	AuPoint n = vx.Cross(vy);
	n.Unit();

	AuScFaceList faces;

	// Create a face.
	AuScPoint p1(v1, 0,0, n);
	AuScPoint p2(v2, 0,0, n);
	AuScPoint p3(v3, 0,0, n);
	AuScFaceTri f(p1, p2, p3);
	faces.AddFace(f);

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->AddFaces(faces);
	return true;
}

bool AuScParts::SetGroupName(quint16 partId, const QString& groupName, AuScParts& partsChanged)
{
	// An empty group name is valid.
	if(!groupName.isEmpty() && !IsNameValid(groupName))
		return false;

	iterator pi = find(partId);
	if(pi == end())
		return false;

	if((*pi)->GroupName() == groupName)
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetGroupName(groupName);
	return true;
}

bool AuScParts::SetHidden(quint16 partId, bool hide, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if((*pi)->IsHidden() == hide)
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetHidden(hide);
	return true;
}

bool AuScParts::SetLocked(quint16 partId, bool lock, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if((*pi)->IsLocked() == lock)
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetLocked(lock);
	return true;
}

bool AuScParts::SetStopName(quint16 partId, const QString& name, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if(!(*pi)->CanChangeStopName())
		return false;

	if((*pi)->StopName() == name)
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetStopName(name);
	return true;
}

bool AuScParts::SetOmitted(quint16 partId, bool omit, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if((*pi)->IsOmitted() == omit)
		return false;

	// Grab a clone before we change it. It might not change. Some parts insist upon being omitted.
	AuScPart* c = (*pi)->Clone();

	(*pi)->SetOmitted(omit);

	if((*pi)->IsOmitted() == omit)
	{
		// The part accepted the change.
		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), c);
		return true;
	}
	else
	{
		// The part did not accept the change.
		delete c;
		return false;
	}
}

bool AuScParts::SetSealed(quint16 partId, bool seal, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if((*pi)->IsSealed() == seal)
		return false;

	// Grab a clone before we change it. It might not change. Some parts insist upon being sealed.
	AuScPart* c = (*pi)->Clone();

	(*pi)->SetSealed(seal);

	if((*pi)->IsSealed() == seal)
	{
		// The part accepted the change.
		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), c);
		return true;
	}
	else
	{
		// The part did not accept the change.
		delete c;
		return false;
	}
}

bool AuScParts::SetState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if(!(*pi)->CanChangeState())
		return false;

	if((*pi)->State() == state)
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetState(state);
	return true;
}

bool AuScParts::AddFaces(quint16 partId, const AuScFaceList& faces, AuScParts& partsChanged)
{
	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	if((*hi)->IsLocked() || (*hi)->IsHidden())
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(partId, (*hi)->Clone());

	// Copy faces from 's' to 'hi'. We don't really copy edges or vertices, just whole faces.
	(*hi)->AddFaces(faces);
	return true;
}

bool AuScParts::AddState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	AuScPart* c = (*hi)->Clone();

	if(!(*hi)->AddState(state))
	{
		delete c;
		return false;
	}

	// Store part clone made prior to change, for undo.
	partsChanged.insert(partId, c);
	return true;
}

bool AuScParts::DeleteState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	AuScPart* c = (*hi)->Clone();

	if(!(*hi)->DeleteState(state))
	{
		delete c;
		return false;
	}

	// Store part clone made prior to change, for undo.
	partsChanged.insert(partId, c);
	return true;
}

void AuScParts::Faces(AuScFaceList& faces, const AuScSelection& sel) const
{
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		const_iterator pi = constFind(si.key());
		if(pi != cend())
			(*pi)->Faces(faces, *si);
	}
}

void AuScParts::VisibleFaces(AuScFaceList& faces) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsHidden())
			continue;
		if(!(*pi)->IsVisibleInFinalModel())
			continue;
		AuScFaceList partFaces;
		(*pi)->Faces(partFaces);
		faces.AddFaces(partFaces);
	}
}

void AuScParts::VisibleFaces(AuScFaceList& faces, const QString& groupName) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsHidden())
			continue;
		if(!(*pi)->IsVisibleInFinalModel())
			continue;
		if((*pi)->GroupName() != groupName)
			continue;
		AuScFaceList partFaces;
		(*pi)->Faces(partFaces);
		faces.AddFaces(partFaces);
	}
}

void AuScParts::CutFaces(const AuScSelection& sel, const AuPointArray& cutLine, AuScParts& partsChanged)
{
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		// Grab a clone for the undo.
		AuScPart* c = (*pi)->Clone();

		if((*pi)->CutFaces(cutLine))
			partsChanged.insert(pi.key(), c);	// The part was cut.
		else
			delete c;							// The part was not cut.
	}
}

void AuScParts::PunchFaces(const AuScSelection& sel, const AuPointArray& cutBnd, AuScParts& partsChanged)
{
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		// Grab a clone for the undo.
		AuScPart* c = (*pi)->Clone();

		if((*pi)->PunchFaces(cutBnd))
			partsChanged.insert(pi.key(), c);	// The part was cut.
		else
			delete c;							// The part was not cut.
	}
}

void AuScParts::Loop(const AuScSelection& sel, const AuPoint& loc, AuPointArray& loop) const
{
	// Gather the faces from the selection into a single list.
	AuScFaceList selFaces;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		const_iterator pi = find(si.key());
		if(pi != end())
			(*pi)->Faces(selFaces);
	}

	// We have all the faces. Try to find a loop around them. Any successful loop will do. We're not too picky.
	loop.append(loc);
	AuSegment edgeAcross;
	QSet<AuScFace*> crossedIt;
	for(AuScFaceList::const_iterator fi = selFaces.cbegin(); fi != selFaces.cend(); fi++)
		if((*fi)->Cross(loop, edgeAcross))
		{
			crossedIt.insert(*fi);
			break;
		}

	// Did we find our way across the first face?
	if(crossedIt.empty())
	{
		// We did not find a face to cross.
		loop.clear();
		return;
	}

	// We found our way across a face. Keep going from the end of the loop until there are no more faces to cross.
	bool crossedOne;
	do
	{
		crossedOne = false;
		for(AuScFaceList::const_iterator fi = selFaces.cbegin(); fi != selFaces.cend(); fi++)
		{
			// Skip faces we have already crossed.
			if(crossedIt.contains(*fi))
				continue;

			if((*fi)->Cross(loop, edgeAcross, edgeAcross))
			{
				crossedIt.insert(*fi);
				crossedOne = true;
				break;
			}
		}
	}
	while(crossedOne);
}

// Converts each selected part into a nav light if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeNavLightLens(const AuScSelection& sel, const QColor& color, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScNavLightLens part;
		part.SetColor(color);
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a door if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeDecal(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScDecal part;
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a door if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeDoor(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScDoor part;
		part.AddFaces(faces);
		part.AddState(AuScDoor::Open);	// Add empty open state.
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a headlight if possible. Converted parts are deleted, moved to 'partsDeleted'.
/*
bool AuScParts::MakeHeadlightLens(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScHeadlightLens part;
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}
*/

// Converts each selected part into a street light if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeLightLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScLightLens part;
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;
		part.SetLightColor(lightColor);

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a landin gear if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeHull(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScHull part;
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a landin gear if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeLandingGear(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScLandingGear part;
		part.AddFaces(faces);
		part.AddState(AuScLandingGear::Retracted);	// Add empty retracted state.
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a landin gear if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeMesh(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScMesh part;
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a landin gear if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeRoom(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScRoom part;
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a rotating beacon if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeRotatingBeaconLens(const AuScSelection& sel, const QColor& color, const AuPoint& rotationAxis,
		AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScRotatingBeaconLens part;
		part.SetColor(color);
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		part.SetLightRotationAxis(rotationAxis);
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a strobe if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeStrobeLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScStrobeLens part;
		part.SetColor(lightColor);
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

bool AuScParts::MakeTurboLift(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	quint16 roomId = 0;
	quint16 roomVoidId = 0;
	quint16 jigId = 0;
	AuScRoom* room = 0;
	AuScRoomVoid* roomVoid = 0;
	AuScDoorJig* jig = 0;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		if(!room)
		{
			room = dynamic_cast<AuScRoom*>(*pi);
			if(room)
				roomId = pi.key();
		}

		if(!roomVoid)
		{
			roomVoid = dynamic_cast<AuScRoomVoid*>(*pi);
			if(roomVoid)
				roomVoidId = pi.key();
		}

		if(!jig)
		{
			jig = dynamic_cast<AuScDoorJig*>(*pi);
			if(jig)
				jigId = pi.key();
		}

		if(room && roomVoid && jig)
			break;
	}

	// Did we find a room, a room void, and door jig?
	if(!room || !roomVoid || !jig)
		return false;

	// Get the faces of the selected parts.
	AuScFaceList roomFaces;
	room->Faces(roomFaces);
	if(roomFaces.empty())
		return false;

	AuScFaceList roomVoidFaces;
	roomVoid->Faces(roomVoidFaces);
	if(roomVoidFaces.empty())
		return false;

	AuScFaceList jigFaces;
	jig->JigFaces(jigFaces);
	if(jigFaces.empty())
		return false;

	AuScLift lift;
	lift.SetGravity(roomVoid->GravityType(), roomVoid->GravityUp(AuMatrix4x4()));
	lift.AddFaces(roomFaces);			// This will be the Car state by default.
	lift.SetState(AuScLift::RoomVoid);
	lift.AddFaces(roomVoidFaces);		// Add the room void.
	lift.SetState(AuScLift::DoorJig);
	lift.AddFaces(jigFaces);			// Add the jig.
	lift.SetState(AuScLift::Car);		// Leave this thing in the Car state so it doesn't disappear.
	//lift.SetGroupName(room->GroupName());
	if(!lift.IsValid())
		return false;

	// The lift was created successfully from the faces of the selected part.

	// Add the new lift to my list of parts.
	bool result = AddPart(lift, partsAdded);

	// Delete the room, room void, and door jig.
	DeletePart(roomId, partsDeleted);
	DeletePart(roomVoidId, partsDeleted);
	DeletePart(jigId, partsDeleted);
	return result;
}

// Converts each selected part into a turret if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeTurret(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScTurret part;
		part.SetBasePivot(basePivot);
		part.SetBaseAxis(baseAxis);
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

// Converts each selected part into a window if possible. Converted parts are deleted, moved to 'partsDeleted'.
bool AuScParts::MakeWindow(const AuScSelection& sel, float opacity, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	bool anyMade = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		if(!(*pi)->CanMakeIntoOtherPart())
			continue;	// Selected part does not allow itself to be turned into something else.

		// Get the faces of the selected parts.
		AuScFaceList faces;
		(*pi)->Faces(faces);
		if(faces.empty())
			continue;

		// Create the new part from those faces.
		AuScWindow part;
		part.AddFaces(faces);
		part.SetGroupName((*pi)->GroupName());
		part.SetOpacity(opacity);		// Sets opacity of all faces.
		if(!part.IsValid())
			continue;

		// The part was created successfully from the faces of the selected part.
		// Move the selected part to the deleted list.
		partsDeleted.insert(pi.key(), *pi);
		erase(pi);

		// Add the new part to my list of parts last, so as not to mess up the iterator.
		AddPart(part, partsAdded);
		anyMade = true;
	}
	return anyMade;
}

bool AuScParts::SetTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsChanged)
{
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		const_iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		AuScTurret* turret = dynamic_cast<AuScTurret*>(*pi);
		if(!turret)
			continue;	// Selected part is not a turret.

		// Store part prior to change, for undo.
		partsChanged.insert(si.key(), (*pi)->Clone());

		turret->SetBasePivot(basePivot);
		turret->SetBaseAxis(baseAxis);

		// We only do one at a time.
		return true;
	}
	return false;
}

bool AuScParts::SetTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis, const AuPoint& gunEmitLoc,
		const AuPoint& gunEmitVec, const AuPoint& gunSightLoc, AuScParts& partsChanged)
{
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		const_iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		AuScTurret* turret = dynamic_cast<AuScTurret*>(*pi);
		if(!turret)
			continue;	// Selected part is not a turret.

		// Store part prior to change, for undo.
		partsChanged.insert(si.key(), (*pi)->Clone());

		turret->SetGunPivot(gunPivot);
		turret->SetGunAxis(gunAxis);
		turret->SetGunEmitLoc(gunEmitLoc);
		turret->SetGunEmitVec(gunEmitVec);
		turret->SetGunSightLoc(gunSightLoc);

		// We only do one at a time.
		return true;
	}
	return false;
}

bool AuScParts::SetTurretStation(const AuScSelection& sel, /*AuScTurret::GUNNER_POSITION*/quint8 gunnerPos,
		const AuPoint& gunnerLoc, const AuPoint& gunnerFacing, const AuPoint& corner1, const AuPoint& corner2,
		const AuMatrix4x4& gridPlane, COMMODITYID module, AuScParts& partsChanged)
{
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		const_iterator pi = find(si.key());
		if(pi == end())
			continue;	// Selected part id was not found.

		AuScTurret* turret = dynamic_cast<AuScTurret*>(*pi);
		if(!turret)
			continue;	// Selected part is not a turret.

		// Store part prior to change, for undo.
		partsChanged.insert(si.key(), (*pi)->Clone());

		turret->MakeScreen(corner1, corner2, gridPlane);
		turret->SetGunner((AuScTurret::GUNNER_POSITION)gunnerPos, gunnerLoc, gunnerFacing);
		turret->SetModule(module);

		// We only do one at a time.
		return true;
	}
	return false;
}

void AuScParts::Import3DS(const QString& fileName, double scale)
{
	//qDebug() << "AuScParts::Import3DS" << fileName << "scale" << scale;
	Au3dsGLSLModel model;
	if(!model.Load(fileName, scale))
		return;

	QList<Au3dsGLSLTriTexList> models;
	model.m_Objects.LoadModels(models, model.m_Materials, false, model.m_ModelType, GL_ONE_MINUS_SRC_ALPHA);

	for(QList<Au3dsGLSLTriTexList>::const_iterator mi = models.cbegin(); mi != models.cend(); mi++)
	{
		AuScFaceList faces;
		for(Au3dsGLSLTriTexList::const_iterator ti = (*mi).cbegin(); ti != (*mi).cend(); ti++)
		{
			AuScPoint sv1((*ti).m_V0);
			AuScPoint sv2((*ti).m_V1);
			AuScPoint sv3((*ti).m_V2);

			faces.AddFace(AuScFaceTri(sv1, sv2, sv3));
		}

		//faces.JoinFaces();
		//faces.FlattenVertexNormals();

		AuScMesh mesh;
		mesh.AddFaces(faces);
		AddPart(mesh);
	}

	/*
	for(Au3dsGLSLObjectArray::const_iterator oi = model.m_Objects.cbegin(); oi != model.m_Objects.cend(); oi++)
	{
		AuScFaceList faces;
		for(Au3dsGLSLFaceArray::const_iterator fi = (*oi).m_Faces.cbegin(); fi != (*oi).m_Faces.cend(); fi++)
		{
			const Au3dsGLSLVertex& v1 = (*fi).Vertex(0, (*oi).m_Vertices);
			const Au3dsGLSLVertex& v2 = (*fi).Vertex(1, (*oi).m_Vertices);
			const Au3dsGLSLVertex& v3 = (*fi).Vertex(2, (*oi).m_Vertices);

			AuScPoint sv1(v1.Coord(), v1.U(),v1.V(), (*fi).m_normals[0]);
			AuScPoint sv2(v2.Coord(), v2.U(),v2.V(), (*fi).m_normals[1]);
			AuScPoint sv3(v3.Coord(), v3.U(),v3.V(), (*fi).m_normals[2]);

			faces.AddFace(AuScFaceTri(sv1, sv3, sv2));
		}

		faces.JoinFaces();
		faces.FlattenVertexNormals();

		AuScMesh mesh;
		mesh.AddFaces(faces);
		AddPart(mesh);
	}
	*/
}

bool AuScParts::SetOpacity(quint16 partId, float opacity, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if(!(*pi)->CanChangeOpacity())
		return false;

	if((*pi)->Opacity() == opacity)
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetOpacity(opacity);
	return true;
}

bool AuScParts::AddStop(quint16 partId, const QString& stop, AuScParts& partsChanged)
{
	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	AuScPart* c = (*hi)->Clone();

	if(!(*hi)->AddStop(stop))
	{
		delete c;
		return false;
	}

	// Store part clone made prior to change, for undo.
	partsChanged.insert(partId, c);

	// Make it the current stop.
	(*hi)->SetStop(stop);

	return true;
}

bool AuScParts::DeleteStop(quint16 partId, const QString& stop, AuScParts& partsChanged)
{
	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	AuScPart* c = (*hi)->Clone();

	if(!(*hi)->DeleteStop(stop))
	{
		delete c;
		return false;
	}

	// Store part clone made prior to change, for undo.
	partsChanged.insert(partId, c);
	return true;
}

bool AuScParts::RenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName, AuScParts& partsChanged)
{
	if(oldStopName == nuStopName)
		return false;
	if(nuStopName.isEmpty() || !IsNameValid(nuStopName))
		return false;

	// Find the existing part.
	iterator hi = find(partId);
	if(hi == end())
		return false;

	AuScPart* c = (*hi)->Clone();

	if(!(*hi)->RenameStop(oldStopName, nuStopName))
	{
		delete c;
		return false;
	}

	// Store part clone made prior to change, for undo.
	partsChanged.insert(partId, c);

	// Change corresponding lift door.
	for(iterator pi = begin(); pi != end(); pi++)
	{
		if(!(*pi)->IsTurboLiftDoor())
			continue;
		if((*pi)->StopName() != oldStopName)
			continue;

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetStopName(nuStopName);
	}
	return true;
}

bool AuScParts::SetStop(quint16 partId, const QString& stop, AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if(!(*pi)->CanChangeStop())
		return false;

	if((*pi)->AtStop() == stop)
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetStop(stop);
	return true;
}

quint16 AuScParts::FindPart(/*AuScPart::PART_TYPE*/quint8 partType) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if((*pi)->Type() == (AuScPart::PART_TYPE)partType)
			return pi.key();
	return 0;
}

bool AuScParts::SetLight(quint16 partId, const QColor& color, float kL, float kQ, float cosOuterCone, float cosInnerCone,
		AuScParts& partsChanged)
{
	iterator pi = find(partId);
	if(pi == end())
		return false;

	if(!(*pi)->CanChangeLight())
		return false;

	// Store part prior to change, for undo.
	partsChanged.insert(pi.key(), (*pi)->Clone());

	(*pi)->SetLight(color, kL, kQ, cosOuterCone, cosInnerCone);
	return true;
}

bool AuScParts::MoveDoorSwitches(const AuScSelection& sel, const AuPoint& switch1, const AuPoint& switch2, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetDoorOpLocs(switch1, switch2);
	}
	return true;
}

bool AuScParts::SetRoomProperties(const AuScSelection& sel, bool isSunlit, INTERIOR_LIGHT initLighting, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetInteriorLighting(isSunlit, initLighting);
	}
	return true;
}

bool AuScParts::SetDoorProperties(const AuScSelection& sel, Qt::CheckState isAutomatic, Qt::CheckState initLocked,
		Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetDoorProperties(isAutomatic, initLocked, remoteSpacecraft, remoteVehicle);
	}
	return true;
}

bool AuScParts::SetSiteProperties(const AuScSelection& sel, quint8 grade, quint8 grass, quint8 pavement, quint8 rocks,
		quint8 shrubs, quint8 terBlend, quint8 trees, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetSiteProperties(grade, grass, pavement, rocks, shrubs, terBlend, trees);
	}
	return true;
}

bool AuScParts::SetLightBulbProperties(const AuScSelection& sel, const QColor& color, float kL, float kQ, float cosOuterCone,
		float cosInnerCone, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;
		if(!(*pi)->CanChangeLight())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());
		(*pi)->SetLight(color, kL, kQ, cosOuterCone, cosInnerCone);
	}
	return true;
}

bool AuScParts::SetLightLensProperties(const AuScSelection& sel, const QColor& color, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;
		if(!(*pi)->IsLightLens())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());
		(*pi)->SetLightColor(color);
	}
	return true;
}

bool AuScParts::SetVoidProperties(const AuScSelection& sel, GRAVITY gravityType, const AuPoint& gravityUp, HOLD_ACCESS holdAccess,
		quint8 personnelAccess, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetGravity(gravityType, gravityUp);
		(*pi)->SetHoldAccess(holdAccess);
		(*pi)->SetPersonnelAccess(personnelAccess);
	}
	return true;
}

bool AuScParts::SetFaceSelfIllumination(const AuScSelection& sel, float selfIllum, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetFaceSelfIllumination(*si, selfIllum);
	}
	return true;
}

bool AuScParts::SetFaceOpacity(const AuScSelection& sel, float opacity, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetFaceOpacity(*si, opacity);
	}
	return true;
}

bool AuScParts::TextureUnwrap(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		quint8 unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureUnwrap(*si, texIndex, texSize, (AuScFaceList::UNWRAP_METHOD)unwrap, angleTol, gridAlign, alignAxis, gridPlane);
	}
	return true;
}

bool AuScParts::TextureMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureMove(*si, vec);
	}
	return true;
}

bool AuScParts::TextureRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureRotate(*si, pivot, angle);
	}
	return true;
}

bool AuScParts::TextureStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureStretch(*si, base, scaleX, scaleY);
	}
	return true;
}

bool AuScParts::TextureFaces(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureFaces(*si, texIndex);
	}
	return true;
}

// Returns true if any changes are made.
// 'partsChanged' is filled with clones for caller creation of undo data.
bool AuScParts::TextureFacesCylindrical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureFacesCylindrical(*si, texIndex, texRepsAtEquator, texSize, gridPlane);
	}
	return true;
}

// Returns true if any changes are made.
// 'partsChanged' is filled with clones for caller creation of undo data.
bool AuScParts::TextureFacesProject(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureFacesProject(*si, texIndex, texSize, gridPlane);
	}
	return true;
}

// Returns true if any changes are made.
// 'partsChanged' is filled with clones for caller creation of undo data.
bool AuScParts::TextureFacesSpherical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TextureFacesSpherical(*si, texIndex, texRepsAtEquator, gridPlane);
	}
	return true;
}

bool AuScParts::EffectFaces(const AuScSelection& sel, SURFACE_EFFECT effect, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectFaces(*si, effect);
	}
	return true;
}

// Returns true if any changes are made.
// 'partsChanged' is filled with clones for caller creation of undo data.
bool AuScParts::EffectFacesCylindrical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectFacesCylindrical(*si, effect, texRepsAtEquator, texSize, gridPlane);
	}
	return true;
}

// Returns true if any changes are made.
// 'partsChanged' is filled with clones for caller creation of undo data.
bool AuScParts::EffectFacesProject(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectFacesProject(*si, effect, texSize, gridPlane);
	}
	return true;
}

// Returns true if any changes are made.
// 'partsChanged' is filled with clones for caller creation of undo data.
bool AuScParts::EffectFacesSpherical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectFacesSpherical(*si, effect, texRepsAtEquator, gridPlane);
	}
	return true;
}

bool AuScParts::EffectUnwrap(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		quint8 unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectUnwrap(*si, effect, texSize, (AuScFaceList::UNWRAP_METHOD)unwrap, angleTol, gridAlign, alignAxis, gridPlane);
	}
	return true;
}

SC_TEXTURE_INDEX AuScParts::TextureIndex(const AuScSelection& sel) const
{
	if(sel.empty())
		return SCTEXI_Count;

	qint8 texI = -1;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		const_iterator pi = find(si.key());
		if(pi == cend())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		qint8 pTexI = (*pi)->TextureIndex(*si);
		if(pTexI == -2)
			continue;					// Part has no faces in the selection.

		if(texI == -1)
			texI = pTexI;				// First tex index.
		else if(texI != pTexI)
			return SCTEXI_Count;		// Different tex index. No need to keep looking.
	}

	if(texI == -1)
		return SCTEXI_Count;

	return (SC_TEXTURE_INDEX)texI;
}

SURFACE_EFFECT AuScParts::Effect(const AuScSelection& sel) const
{
	if(sel.empty())
		return EFF_Count;

	qint8 texI = -1;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		const_iterator pi = find(si.key());
		if(pi == cend())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		qint8 pTexI = (*pi)->Effect(*si);
		if(pTexI == -2)
			continue;					// Part has no faces in the selection.

		if(texI == -1)
			texI = pTexI;				// First tex index.
		else if(texI != pTexI)
			return EFF_Count;		// Different tex index. No need to keep looking.
	}

	if(texI == -1)
		return EFF_Count;

	return (SURFACE_EFFECT)texI;
}

bool AuScParts::EffectMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectMove(*si, vec);
	}
	return true;
}

bool AuScParts::EffectRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectRotate(*si, pivot, angle);
	}
	return true;
}

bool AuScParts::EffectStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY, AuScParts& partsChanged)
{
	if(sel.empty())
		return false;

	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->EffectStretch(*si, base, scaleX, scaleY);
	}
	return true;
}

void AuScParts::RefreshObstructionsCache()
{
	if(!g_Au.IsSceneApp())
		qWarning() << "error: AuScParts::RefreshObstructionsCache must only be called on scene client.";

	for(iterator pi = begin(); pi != end(); pi++)
		if(!(*pi)->IsOmitted())
			(*pi)->RefreshObstructionsCache(pi.key(), *this);
}

void AuScParts::RefreshObstructionsCache(QProgressDialog& progress)
{
	if(!g_Au.IsSceneApp())
		qWarning() << "error: AuScParts::RefreshObstructionsCache must only be called on scene client.";

	for(iterator pi = begin(); pi != end(); pi++)
	{
		if(!(*pi)->IsOmitted())
			(*pi)->RefreshObstructionsCache(pi.key(), *this);
		progress.setValue(progress.value() + 1);
		if(progress.wasCanceled())
			return;
	}
}

void AuScParts::RefreshObstructionsCacheForFinalize(QProgressDialog& progress)
{
	if(!g_Au.IsSceneApp())
		qWarning() << "error: AuScParts::RefreshObstructionsCacheForFinalize must only be called on scene client.";

	for(iterator pi = begin(); pi != end(); pi++)
	{
		if(!(*pi)->IsOmitted())
			(*pi)->RefreshObstructionsCacheIfNeeded(pi.key(), *this);
		progress.setValue(progress.value() + 1);
		if(progress.wasCanceled())
			return;
	}
}

bool AuScParts::RefreshObstructionsCacheIfNeeded()
{
	if(!g_Au.IsSceneApp())
		qWarning() << "error: AuScParts::RefreshObstructionsCacheIfNeeded must only be called on scene client.";

	bool needed = false;
	for(iterator pi = begin(); pi != end(); pi++)
		if(!(*pi)->IsOmitted())
			if((*pi)->RefreshObstructionsCacheIfNeeded(pi.key(), *this))
				needed = true;
	return needed;
}

bool AuScParts::IsPathEverUnobstructed(const AuPoint& loc) const
{
	// Test all barriers. They take precedence.
	/*
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsBarrier())
			continue;
		if((*pi)->IsInsideObstructionModel(loc))
			return false;				// Location blocked by the barrier.
	}
	*/

	// Test all voids.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideVoidEver(loc))
			return true;				// Location is inside the void, in at least one of its stops or states.
	}

	// Test all solids, except barriers that have already been checked.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsBarrier())
			continue;
		if((*pi)->IsInsideObstructionModel(loc))
			return false;				// Location blocked by the solid.
	}

	// Location was not inside the hull at all.
	return true;
}

bool AuScParts::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	// Test extents box.
	AuPoint min,max;
	HullExtents(min, max);
	min -= AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	max += AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	AuBox extbox(min, max);
	if(fromLoc.FuzzyEquals(toLoc))
	{
		if(!extbox.IsPointInside(toLoc))
			return false;				// Points are not inside my extents.

		// Check barriers first because they override voids.
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if(!(*pi)->IsBarrier())
				continue;
			if((*pi)->IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
				return true;				// Movement blocked by the solid.
		}

		// Check voids next because they override solids.
		bool stepInVoid = false;
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if((*pi)->IsInsideVoid(toLoc))
			{
				stepInVoid = true;
				break;
			}
			else if((*pi)->InBuildZone(toLoc, d_BuildZoneHeight))
			{
				// Build zone acts like a hull void above it.
				stepInVoid = true;
				break;
			}
		}
		// Step into a void space is not blocked.
		if(!stepInVoid)
		{
			// Check obstruction by solid things, except for barriers that have already been checked.
			for(const_iterator pi = cbegin(); pi != cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsBarrier())
					continue;
				if((*pi)->IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
					return true;				// Movement blocked by the part.
			}
		}
	}
	else
	{
		AuPoint int0, int1;
		if(!extbox.SegmentIntersection(fromLoc, toLoc, int0, int1))
		{
			// Locations did not intersect my extents. They could both be inside.
			if(!extbox.IsPointInside(fromLoc) || !extbox.IsPointInside(toLoc))
				return false;			// Points are not inside my extents.
		}

		// Integrate along the segment to see if we hit any solids. We will assume the 'fromLoc' is ok and step from there.
		double dist = fromLoc.Distance(toLoc);
		int steps = ceil(dist);				// Take 1' steps.
		AuPoint stepLoc(fromLoc);
		AuPoint stepVec = toLoc - fromLoc;
		if(steps > 1)
			stepVec /= steps;
		for(int step=0; step<steps; step++)
		{
			stepLoc += stepVec;

			// Check barriers first because they override voids.
			for(const_iterator pi = cbegin(); pi != cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if(!(*pi)->IsBarrier())
					continue;
				if((*pi)->IsPathBlocked(fromLoc, stepLoc, hitLoc, hitNormal))
					return true;				// Movement blocked by the solid.
			}

			// Check voids next because they override solids.
			bool stepInVoid = false;
			for(const_iterator pi = cbegin(); pi != cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsInsideVoid(stepLoc))
				{
					stepInVoid = true;
					break;
				}
				else if((*pi)->InBuildZone(toLoc, d_BuildZoneHeight))
				{
					// Build zone acts like a hull void above it.
					stepInVoid = true;
					break;
				}
			}

			// Step into a void space is not blocked.
			if(!stepInVoid)
			{
				// Check obstruction by solid things, except for barriers that have already been checked.
				for(const_iterator pi = cbegin(); pi != cend(); pi++)
				{
					if((*pi)->IsOmitted())
						continue;
					if((*pi)->IsBarrier())
						continue;
					if((*pi)->IsPathBlocked(fromLoc, stepLoc, hitLoc, hitNormal))
						return true;				// Movement blocked by the part.
				}
			}
		}
	}
	return false;	// Movement not blocked.
}

bool AuScParts::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	// Test extents box.
	AuPoint min,max;
	HullExtents(min, max, landingGearStates);
	min -= AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	max += AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	AuBox extbox(min, max);
	if(fromLoc.FuzzyEquals(toLoc))
	{
		if(!extbox.IsPointInside(toLoc))
			return false;				// Points are not inside my extents.

		// Check barriers first because they override voids.
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if(!(*pi)->IsBarrier())
				continue;
			if((*pi)->IsPathBlocked(pi.key(), fromLoc, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops))
				return true;				// Movement blocked by the solid.
		}

		// Check voids next because they override solids.
		bool stepInVoid = false;
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if((*pi)->IsInsideVoid(pi.key(), toLoc, doorStates, landingGearStates, liftStops))
			{
				stepInVoid = true;
				break;
			}
			else if((*pi)->InBuildZone(toLoc, d_BuildZoneHeight))
			{
				// Build zone acts like a hull void above it.
				stepInVoid = true;
				break;
			}
		}

		// Step into a void space is not blocked.
		if(!stepInVoid)
		{
			// Check obstruction by solid things, except for barriers that have already been checked.
			for(const_iterator pi = cbegin(); pi != cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsBarrier())
					continue;
				if((*pi)->IsPathBlocked(pi.key(), fromLoc, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops))
					return true;				// Movement blocked by the part.
			}
		}
	}
	else
	{
		AuPoint int0, int1;
		if(!extbox.SegmentIntersection(fromLoc, toLoc, int0, int1))
		{
			// Locations did not intersect my extents. They could both be inside.
			if(!extbox.IsPointInside(fromLoc) || !extbox.IsPointInside(toLoc))
				return false;				// Points are not inside my extents.
		}

		// Integrate along the segment to see if we hit any solids. We will assume the 'fromLoc' is ok and step from there.
		double dist = fromLoc.Distance(toLoc);
		int steps = ceil(dist);				// Take 1' steps.
		AuPoint stepLoc(fromLoc);
		AuPoint stepVec = toLoc - fromLoc;
		if(steps > 1)
			stepVec /= steps;
		for(int step=0; step<steps; step++)
		{
			stepLoc += stepVec;

			// Check barriers first because they override voids.
			for(const_iterator pi = cbegin(); pi != cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if(!(*pi)->IsBarrier())
					continue;
				if((*pi)->IsPathBlocked(pi.key(), fromLoc, stepLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops))
					return true;				// Movement blocked by the solid.
			}

			// Check voids next because they override solids.
			bool stepInVoid = false;
			for(const_iterator pi = cbegin(); pi != cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsInsideVoid(pi.key(), stepLoc, doorStates, landingGearStates, liftStops))
				{
					stepInVoid = true;
					break;
				}
				else if((*pi)->InBuildZone(toLoc, d_BuildZoneHeight))
				{
					// Build zone acts like a hull void above it.
					stepInVoid = true;
					break;
				}
			}

			// Step into a void space is not blocked.
			if(!stepInVoid)
			{
				// Check obstruction by solid things, except for barriers that have already been checked.
				for(const_iterator pi = cbegin(); pi != cend(); pi++)
				{
					if((*pi)->IsOmitted())
						continue;
					if((*pi)->IsBarrier())
						continue;
					if((*pi)->IsPathBlocked(pi.key(), fromLoc, stepLoc, hitLoc, hitNormal, doorStates, landingGearStates,
								liftStops))
						return true;				// Movement blocked by the part.
				}
			}
		}
	}
	return false;	// Movement not blocked.
}

bool AuScParts::IsInsideHullExtents(const AuPoint& loc) const
{
	AuPoint min,max;
	HullExtents(min, max);
	min -= AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	max += AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	AuBox hullbox(min, max);
	return hullbox.IsPointInside(loc);
}

bool AuScParts::IsInsideHullExtents(const AuPoint& loc, const AuLandingGearStateHash& landingGearStates) const
{
	AuPoint min,max;
	HullExtents(min, max, landingGearStates);
	min -= AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	max += AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	AuBox hullbox(min, max);
	return hullbox.IsPointInside(loc);
}

bool AuScParts::IsInsideHull(const AuPoint& loc) const
{
	if(!IsInsideObstructionModel(loc))
		return false;

	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsHullVoid())
			continue;
		if((*pi)->IsInsideVoid(loc, *this))
			return false;
	}
	return true;
}

bool AuScParts::IsInsideHull(const AuPoint& loc, const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates,
		const AuLiftHash& liftStops) const
{
	if(!IsInsideObstructionModel(loc, doorStates, landingGearStates))
		return false;

	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->InBuildZone(loc, d_BuildZoneHeight))
			return false;
		if(!(*pi)->IsHullVoid())
			continue;
		if((*pi)->IsInsideVoid(pi.key(), loc, doorStates, landingGearStates, liftStops))
			return false;
	}
	return true;
}

bool AuScParts::IsInsideObstructionModel(const AuPoint& loc) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideObstructionModel(loc))
			return true;
	}
	return false;
}

bool AuScParts::IsInsideObstructionModel(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideObstructionModel(pi.key(), loc, doorStates, landingGearStates))
			return true;
	}
	return false;
}

GRAVITY AuScParts::QueryGravity(const AuPoint& loc, AuPoint& floorLoc, AuPoint& gravityN, AuPoint& terN) const
{
	// No gravity unless loc is inside the overall extents of the model. In the designer, the obstruction model may not exist.
	// There might not be a hull at all.
	AuPoint min,max;
	ModelExtents(min, max);
	min -= AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	max += AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	AuBox extbox(min, max);
	if(!extbox.IsPointInside(loc))
		return G_Count;

	// Find a void that contains loc.
	quint16 voidId = 0;
	GRAVITY gravity = G_Count;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideVoid(loc, *this))
		{
			// Found a void that contains 'loc'.
			voidId = pi.key();

			// Get floor location under 'loc', in the down direction according to gravity.
			const AuMatrix4x4& stopM = (*pi)->StopMatrixDesign();
			gravity = (*pi)->GravityType();
			gravityN = (*pi)->GravityUp(gravityN, stopM);
			floorLoc = (*pi)->FaceLoc(loc, -gravityN, terN, stopM);
			break;
		}
	}
	if(!voidId)
		return G_Count;

	// Keep falling downward to determine the floor height in case there are more voids below.
	// We keep the downward direction established by the first void; that is where we are falling from.
	QSet<quint16> inVoids;
	inVoids.insert(voidId);
	bool voidBelow;
	do
	{
		voidBelow = false;
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			// Skip voids we already checked.
			if(inVoids.contains(pi.key()))
				continue;

			if((*pi)->IsInsideVoid(floorLoc, *this))
			{
				// Found another void that contains 'floorLoc'.
				voidBelow = true;

				// Get floor location under 'floorLoc', in the down direction according to gravity.
				floorLoc = (*pi)->FaceLoc(floorLoc, -gravityN, terN, (*pi)->StopMatrixDesign());
				inVoids.insert(pi.key());						// Don't do the same void again.
				break;
			}
		}
	}
	while(voidBelow);

	// Account for barriers between loc and the floor loc.
	BlockSegment(loc, floorLoc, terN);

	// We found the floor under 'loc'.
	return gravity;
}

double AuScParts::FenceVolume() const
{
	double fvol = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		fvol += (*pi)->FenceVolume();
	}
	return fvol;
}

quint32 AuScParts::HullVolume() const
{
	// Collect voxels from all of my hull parts.
	QMap<AuPoint, bool> voxels;
	//double voxelSize = 1;	// cubic feet
	double voxelSize = FeetPerMeter();	// cubic meters
	quint32 sumHullVol = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		sumHullVol += (*pi)->HullVolumeVoxels(pi.key(), *this, voxels, voxelSize);
	}
	//quint32 voxVol = (int)ceil(voxels.size() / CubicFeetPerCubicMeter());	// Volume of voxels in cubic meters.
	quint32 voxVol = voxels.size();	// Volume of voxels in cubic meters.

	// The volume of the parts cannot exceed the box volume of the hull extents. It could happen due to overlapping parts.
	AuPoint min,max;
	HullExtents(min, max);
	AuPoint extvec(max - min);
	double extVol = extvec.x * extvec.y * extvec.z;			// Volume of extents in cubic feet.
	quint32 boxVol = (quint32)ceil(extVol * CubicMetersPerCubicFoot());

	// Return the result in cubic meters. The smallest of the three is usually closest to being correct.
	//qDebug() << "AuScParts::HullVolume box volume" << boxVol << "sum of parts" << sumHullVol << "voxel volume" << voxVol;
	if(boxVol < voxVol && boxVol < sumHullVol)
		return boxVol;					// Return box volume of hull extents.
	else if(voxVol < sumHullVol)
		return voxVol;					// Return volume of voxels.
	return sumHullVol;					// Return sum of volumes of all hull parts.
}

quint32 AuScParts::HullVolume(QProgressDialog& progress) const
{
	// Collect voxels from all of my hull parts.
	QMap<AuPoint, bool> voxels;
	//double voxelSize = 1;	// cubic feet
	double voxelSize = FeetPerMeter();	// cubic meters
	quint32 sumHullVol = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if(!(*pi)->IsOmitted())
			sumHullVol += (*pi)->HullVolumeVoxels(pi.key(), *this, voxels, voxelSize);
		progress.setValue(progress.value() + 1);
		if(progress.wasCanceled())
			return 0;
	}
	//quint32 voxVol = (int)ceil(voxels.size() / CubicFeetPerCubicMeter());	// Volume of voxels in cubic meters.
	quint32 voxVol = voxels.size();	// Volume of voxels in cubic meters.

	// The volume of the parts cannot exceed the box volume of the hull extents. It could happen due to overlapping parts.
	AuPoint min,max;
	HullExtents(min, max);
	AuPoint extvec(max - min);
	double extVol = extvec.x * extvec.y * extvec.z;			// Volume of extents in cubic feet.
	quint32 boxVol = (quint32)ceil(extVol * CubicMetersPerCubicFoot());

	// Return the result in cubic meters. The smallest of the three is usually closest to being correct.
	//qDebug() << "AuScParts::HullVolume box volume" << boxVol << "sum of parts" << sumHullVol << "voxel volume" << voxVol;
	if(boxVol < voxVol && boxVol < sumHullVol)
		return boxVol;					// Return box volume of hull extents.
	else if(voxVol < sumHullVol)
		return voxVol;					// Return volume of voxels.
	return sumHullVol;					// Return sum of volumes of all hull parts.
}

quint16 AuScParts::CaptBerths() const
{
	quint16 berths = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		berths += (*pi)->CaptBerths();
	}
	return berths;
}

void AuScParts::BarrierFaces(AuScFaceList& barrierFaces, const QString& groupName) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsBarrier())
			continue;
		if((*pi)->GroupName() != groupName)
			continue;
		(*pi)->Faces(barrierFaces);
	}
}

quint16 AuScParts::Beds() const
{
	quint16 berths = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		berths += (*pi)->Beds();
	}
	return berths;
}

quint16 AuScParts::CrewBerths() const
{
	quint16 berths = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		berths += (*pi)->CrewBerths();
	}
	return berths;
}

quint16 AuScParts::OfficerBerths() const
{
	quint16 berths = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		berths += (*pi)->OfficerBerths();
	}
	return berths;
}

quint16 AuScParts::PassengerBerths() const
{
	quint16 berths = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		berths += (*pi)->PassengerBerths();
	}
	return berths;
}

quint16 AuScParts::TroopBerths() const
{
	quint16 berths = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		berths += (*pi)->TroopBerths();
	}
	return berths;
}

quint16 AuScParts::Turrets() const
{
	quint16 berths = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		berths += (*pi)->Turrets();
	}
	return berths;
}

void AuScParts::BaysAndFTLDrives(AuScPartSet& bays, AuScPartSet& ftlDrives)
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;

		SYSTEM system = (*pi)->ModuleSystem();
		switch(system)
		{
		case SYS_Weapon:
			bays.insert(pi.key());
			break;

		case SYS_FTLDrive:
			if((*pi)->Module() != CID_Invalid)
				ftlDrives.insert(pi.key());
			break;

		default:
			break;
		}
	}
}

COMMODITYID AuScParts::Module(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return CID_Invalid;
	return (*pi)->Module();
}

QString AuScParts::ReportDesc(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return "";
	return (*pi)->ReportDesc();
}

QString AuScParts::ReportParts(const AuId& archId) const
{
	QString partsList;
	partsList += QObject::tr("<hr><b>DESIGN PARTS</b>");
	partsList += "<div style=\"margin-left: 10px;\">";
	partsList += QObject::tr("%1 Total Parts<br>").arg(size());
	partsList += QString("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" "
			"style=\"vertical-align: top; background-color: %1;\">"
				"<tbody>").arg(AuStyleSheet::HazeronBlue(.5f).name());

	QMultiMap<QString, const_iterator> alphabetizedParts;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		QString name = QString("%1.%2").arg((*pi)->FullName(*this).toUpper()).arg(pi.key());
		alphabetizedParts.insert(name, pi);
	}

	// Column Headers
	partsList += QString("<tr>"
			"<td><b>Id</b>"
			"</td>"
			"<td><b>Name</b>"
			"</td>"
			"<td><b>V</b>"
			"</td>"
			"<td><b>L</b>"
			"</td>"
			"<td><b>O</b>"
			"</td>"
			"<td><b>S</b>"
			"</td>"
			"</tr>");

	for(QMultiMap<QString, const_iterator>::const_iterator ai = alphabetizedParts.cbegin(); ai != alphabetizedParts.cend(); ai++)
	{
		const_iterator pi = *ai;

		QString name = (*pi)->FullName(*this);

		QString show;
		if((*pi)->IsHidden())
			show = ":data/ScHide.png";
		else
			show = ":data/ScShow.png";

		QString lock;
		if((*pi)->IsLocked())
			lock = ":data/ScLock.png";
		else
			lock = ":data/ScUnlock.png";

		QString omit;
		if((*pi)->IsOmitted())
			omit = ":data/ScOmit.png";
		else
			omit = ":data/ScInclude.png";

		QString seal;
		if((*pi)->IsSealed())
			seal = ":data/ScSeal.png";
		else
			seal = ":data/ScUnseal.png";

		QString arch;
		if(!(*pi)->ArchId().IsNull() && (*pi)->ArchId() != archId)
			arch = QString("%1, <i>%2</i>")
				.arg((*pi)->ArchName())
				.arg((*pi)->ArchId().StringId());

		partsList += QString("<tr>"
				"<td>%1"
				"</td>"
				"<td><img height=\"16\" src=\"qrc%2\"> %3"
				"</td>"
				"<td><img height=\"16\" src=\"qrc%4\">"
				"</td>"
				"<td><img height=\"16\" src=\"qrc%5\">"
				"</td>"
				"<td><img height=\"16\" src=\"qrc%6\">"
				"</td>"
				"<td><img height=\"16\" src=\"qrc%7\"> %8"
				"</td>"
				"</tr>")
			.arg(pi.key())
			.arg((*pi)->TypeIcon())
			.arg(name)
			.arg(show)
			.arg(lock)
			.arg(omit)
			.arg(seal)
			.arg(arch);
	}
	partsList += "</tbody>"
		"</table>";
	partsList += "</div>";
	return partsList;
}

void AuScParts::HullExtents(AuPoint& min, AuPoint& max) const
{
	bool firstOne = true;
	AuPoint pmin, pmax;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;

		if(!(*pi)->HullExtents(pmin, pmax))
			continue;

		if(firstOne)
		{
			min = pmin;
			max = pmax;
			firstOne = false;
		}
		else
		{
			min = min.Min(pmin);
			max = max.Max(pmax);
		}
	}
}

void AuScParts::HullExtents(AuPoint& min, AuPoint& max, const AuLandingGearStateHash& landingGearStates) const
{
	bool firstOne = true;
	AuPoint pmin, pmax;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;

		if(!(*pi)->HullExtents(pi.key(), pmin, pmax, landingGearStates))
			continue;

		if(firstOne)
		{
			min = pmin;
			max = pmax;
			firstOne = false;
		}
		else
		{
			min = min.Min(pmin);
			max = max.Max(pmax);
		}
	}
}

void AuScParts::SiteExtents(AuPoint& min, AuPoint& max) const
{
	bool firstOne = true;
	AuPoint pmin, pmax;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;

		if(!(*pi)->SiteExtents(pmin, pmax))
			continue;

		if(firstOne)
		{
			min = pmin;
			max = pmax;
			firstOne = false;
		}
		else
		{
			min = min.Min(pmin);
			max = max.Max(pmax);
		}
	}
}

quint16 AuScParts::GroundVehicleParkingSpots() const
{
	quint16 spots = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		spots += (*pi)->GroundVehicleParkingSpots();
	}
	return spots;
}

void AuScParts::ModelExtents(AuPoint& min, AuPoint& max) const
{
	bool firstOne = true;
	AuPoint pmin, pmax;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;

		if(!(*pi)->ModelExtents(pmin, pmax))
			continue;

		if(firstOne)
		{
			min = pmin;
			max = pmax;
			firstOne = false;
		}
		else
		{
			min = min.Min(pmin);
			max = max.Max(pmax);
		}
	}
}

quint16 AuScParts::RocketParkingSpots() const
{
	quint16 spots = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		spots += (*pi)->RocketParkingSpots();
	}
	return spots;
}

quint16 AuScParts::SpaceVehicleParkingSpots() const
{
	quint16 spots = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		spots += (*pi)->SpaceVehicleParkingSpots();
	}
	return spots;
}

quint16 AuScParts::WaterVehicleParkingSpots() const
{
	quint16 spots = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		spots += (*pi)->WaterVehicleParkingSpots();
	}
	return spots;
}

void AuScParts::ServicePanels(AuScReportServicePanelHash& panels) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsServicePanel())
			continue;
		panels.insert(pi.key(), AuScReportServicePanel((*pi)->Type(), (*pi)->ReportDesc()));
	}
}

void AuScParts::StatusDisplays(AuScReportStatusDisplayHash& displays) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsStatusDisplay())
			continue;

		// Life support status needs the room name for quick lookup of room air pressure.
		QString roomName;
		if((*pi)->AssociatedPartId() && (*pi)->Type() == AuScPart::Part_StatusLifeSupport)
		{
			const_iterator ri = find((*pi)->AssociatedPartId());
			if(ri != cend())
				roomName = (*ri)->GroupName();
		}

		displays.insert(pi.key(), AuScReportStatusDisplay((*pi)->Type(), (*pi)->ReportDesc(), roomName, (*pi)->AssociatedPartId()));
	}
}

void AuScParts::GameTables(AuScReportGameTableHash& tables) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsGameTable())
			continue;

		quint16 roomId = RoomVoidContainingLoc((*pi)->Center(), .5);
		if(!roomId)
			continue;

		tables.insert(pi.key(), AuScReportGameTable(roomId));
	}
}

void AuScParts::CommandStations(AuScPartSet& stations) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsCommandStation())
			stations.insert(pi.key());
	}
}

void AuScParts::ControlStations(AuScPartSet& stations) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsControlStation())
			stations.insert(pi.key());
	}
}

void AuScParts::DesignerStations(AuScPartSet& stations) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->StationType() == AuStation::Integrated_Designer)
			stations.insert(pi.key());
	}
}

void AuScParts::Turrets(AuScPartSet& turrets) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsTurret())
			turrets.insert(pi.key());
	}
}

void AuScParts::CitizenPosts(AuScPartSet& posts) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsCitizenPost())
			posts.insert(pi.key());
	}
}

void AuScParts::GuardPosts(AuScPartSet& posts) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsGuardPost())
			posts.insert(pi.key());
	}
}

void AuScParts::LivestockPosts(AuScPartSet& posts) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsLivestockPost())
			posts.insert(pi.key());
	}
}

bool AuScParts::HasHelm() const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsHelm())
			return true;
	}
	return false;
}

bool AuScParts::HasEnvironmentVoid() const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsEnvironmentVoid())
			return true;
	}
	return false;
}

bool AuScParts::HasBar() const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->HoldAccess() == HOLD_Bar)
			return true;
	}
	return false;
}

bool AuScParts::HasPantry() const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->HoldAccess() == HOLD_Pantry)
			return true;
	}
	return false;
}

COMMODITYID AuScParts::ShieldModule() const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		SYSTEM system = (*pi)->ModuleSystem();
		if(system == SYS_Shield)
			return (*pi)->Module();
	}
	return CID_Invalid;
}

COMMODITYID AuScParts::SensorModule() const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		SYSTEM system = (*pi)->ModuleSystem();
		if(system == SYS_Sensor)
			return (*pi)->Module();
	}
	return CID_Invalid;
}

void AuScParts::VehicleLaunchers(AuScPartSet& launchers) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->Type() == AuScPart::Part_SpaceVehicleLauncher)
			launchers.insert(pi.key());
	}
}

void AuScParts::VehicleLandingSystems(AuScPartSet& recs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->Type() == AuScPart::Part_SpaceVehicleLandingSystem)
			recs.insert(pi.key());
	}
}

AuPoint AuScParts::BerthLocation(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return AuPoint(0,0);
	return (*pi)->BerthLocation();
}

quint16 AuScParts::EnvironmentVoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideEnvironmentVoid(pi.key(), loc, doorStates, landingGearStates, liftStops))
			return pi.key();
	}
	return 0;
}

quint16 AuScParts::RoomVoidContainingLoc(const AuPoint& loc) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideRoomVoid(loc, *this))
			return pi.key();
	}
	return 0;
}

quint16 AuScParts::RoomVoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideRoomVoid(pi.key(), loc, doorStates, landingGearStates, liftStops))
			return pi.key();
	}
	return 0;
}

AuPoint AuScParts::RoomVoidGravityUp(quint16 roomId, const AuLiftHash& liftStops) const
{
	const_iterator pi = find(roomId);
	if(pi == cend())
		return AuPoint(0,0,1);

	const AuMatrix4x4& stopM = (*pi)->StopMatrixAt(roomId, liftStops);
	return (*pi)->GravityUp(stopM);
}

bool AuScParts::RoomVoidIsSunlit(quint16 roomId) const
{
	const_iterator pi = find(roomId);
	if(pi == cend())
		return false;
	return RoomIsSunlit((*pi)->GroupName());
}

bool AuScParts::RoomIsSunlit(const QString& roomName) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsRoomMesh())
			continue;
		if((*pi)->GroupName() != roomName)
			continue;
		return (*pi)->InteriorIsSunlit();
	}
	return false;
}

INTERIOR_LIGHT AuScParts::RoomLight(const QString& roomName) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsRoomMesh())
			continue;
		if((*pi)->GroupName() != roomName)
			continue;
		return (*pi)->InteriorLight();
	}
	return INTLIGHT_Bright;
}

INTERIOR_LIGHT AuScParts::RoomLight(quint16 roomId) const
{
	const_iterator pi = find(roomId);
	if(pi == cend())
		return INTLIGHT_Dark;

	return (*pi)->InteriorLight();
}

quint16 AuScParts::RoomId(const QString& roomName) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsRoomMesh())
			continue;
		if((*pi)->GroupName() != roomName)
			continue;
		return pi.key();
	}
	return 0;
}

HOLD_ACCESS AuScParts::RoomVoidHoldAccess(quint16 roomId) const
{
	const_iterator pi = find(roomId);
	if(pi == cend())
		return HOLD_Inaccessible;

	return (*pi)->HoldAccess();
}

double AuScParts::RoomVoidHeightAboveFloor(quint16 roomId, const AuPoint& loc, bool getRoomUp, AuPoint& gravityN,
		const AuLiftHash& liftStops) const
{
	const_iterator pi = find(roomId);
	if(pi == cend())
		return 0;
	const AuMatrix4x4& stopM = (*pi)->StopMatrixAt(roomId, liftStops);
	if(getRoomUp)
		gravityN = (*pi)->GravityUp(stopM);
	else
		gravityN = (*pi)->GravityUp(gravityN, stopM);
	AuPoint floorN;
	double ht = (*pi)->HeightAboveFloor(loc, gravityN, stopM, floorN);

	// Account for barriers between loc and the floor loc.
	AuPoint floorLoc = loc - gravityN * ht;
	if(BlockSegment(loc, floorLoc, floorN))
		ht = floorLoc.Distance(loc);
	return ht;
}

bool AuScParts::BlockSegment(const AuPoint& loc, AuPoint& floorLoc, AuPoint& floorN) const
{
	// Check for obstructions between loc and the floor.
	bool blocked = false;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->BlockSegment(loc, floorLoc, floorN))
			blocked = true;
	}
	return blocked;
}

quint16 AuScParts::RoomVoidContainingLoc(const AuPoint& loc, double elevate) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideRoomVoid(loc, elevate, *this))
			return pi.key();
	}
	return 0;
}

quint16 AuScParts::RoomVoidContainingLoc(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsInsideRoomVoid(pi.key(), loc, elevate, doorStates, landingGearStates, liftStops))
			return pi.key();
	}
	return 0;
}

AuPoint AuScParts::OperatorLocation(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return AuPoint(0,0);
	return (*pi)->OperatorLoc();
}

AuPoint AuScParts::WeaponEmitLoc(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return AuPoint(0,0);
	return (*pi)->WeaponEmitLoc();
}

void AuScParts::ListRoomNames(QStringList& roomNames) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->AssociatedPartId())
			continue;	// Skip voids that come and go.
		if(!(*pi)->IsRoomVoid())
			continue;
		QString rnid((*pi)->GroupName());
		if(rnid == d_GroupHull)
			continue;
		if(roomNames.contains(rnid))
			continue;
		roomNames.append(rnid);
	}
}

void AuScParts::ListEnvironmentVoids(AuScParts& envVoids) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsEnvironmentVoid())
			continue;
		envVoids.AddPart(*(*pi));
	}
}

bool AuScParts::RandomRoomLocation(const QString& roomName, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	quint16 exactRoomId = 0;
	quint16 closeRoomId = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->AssociatedPartId())
			continue;	// Skip voids that come and go.
		if(!(*pi)->IsRoomVoid())
			continue;
		QString rnid((*pi)->GroupName());
		if(rnid == roomName)
		{
			exactRoomId = pi.key();
			break;
		}
		else if(!closeRoomId && (rnid.compare(roomName, Qt::CaseInsensitive) == 0 ||
					rnid.startsWith(roomName, Qt::CaseInsensitive) ||
					roomName.startsWith(rnid, Qt::CaseInsensitive)))
			closeRoomId = pi.key();
	}
	if(exactRoomId)
		return RandomRoomLocation(exactRoomId, voidLoc, doorStates, landingGearStates, liftStops);
	if(closeRoomId)
		return RandomRoomLocation(closeRoomId, voidLoc, doorStates, landingGearStates, liftStops);
	return false;
}

bool AuScParts::RandomRoomLocation(quint16 roomId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	const_iterator pi = find(roomId);
	if(pi == cend())
		return false;
	return (*pi)->RandomVoidLocation(pi.key(), voidLoc, doorStates, landingGearStates, liftStops);
}

bool AuScParts::RandomSiteLocation(quint16 siteId, AuPoint& siteLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	const_iterator pi = find(siteId);
	if(pi == cend())
		return false;
	return (*pi)->RandomSiteLocation(*this, siteLoc, doorStates, landingGearStates);
}

quint16 AuScParts::RandomRoomVoidLocation(AuPoint& voidLoc, quint8 personnelAccess, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	QList<quint16> roomVoids;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->AssociatedPartId())
			continue;	// Skip voids that come and go.
		// Check personnel access.
		if(!(*pi)->IsAccessGranted(personnelAccess))
			continue;
		if((*pi)->IsRoomVoid())
			roomVoids.append(pi.key());
	}
	if(roomVoids.empty())
		return 0;

	int tries=10;
	do
	{
		int randomVoid = g_Au.Dice().Roll(1, roomVoids.size(), -1);
		quint16 roomId = roomVoids[randomVoid];
		if(RandomRoomLocation(roomId, voidLoc, doorStates, landingGearStates, liftStops))
			return roomId;
	}
	while(tries--);
	return 0;
}

bool AuScParts::RandomBuildZoneLocation(quint16 zoneId, AuPoint& zoneLoc) const
{
	const_iterator pi = find(zoneId);
	if(pi == cend())
		return false;
	return (*pi)->RandomBuildZoneLocation(zoneLoc);
}

quint16 AuScParts::RandomBuildZoneLocation(AuPoint& voidLoc) const
{
	QList<quint16> bzones;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsBuildZone())
			bzones.append(pi.key());
	}
	if(bzones.empty())
		return 0;

	int tries=10;
	do
	{
		int randomZone = g_Au.Dice().Roll(1, bzones.size(), -1);
		quint16 zoneId = bzones[randomZone];
		if(RandomBuildZoneLocation(zoneId, voidLoc))
			return zoneId;
	}
	while(tries--);
	return 0;
}

quint16 AuScParts::RandomEnvironmentVoidLocation(AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	QList<quint16> roomVoids;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->AssociatedPartId())
			continue;	// Skip voids that come and go.
		if((*pi)->IsEnvironmentVoid())
			roomVoids.append(pi.key());
	}
	if(roomVoids.empty())
		return 0;

	int tries=10;
	do
	{
		int randomVoid = g_Au.Dice().Roll(1, roomVoids.size(), -1);
		quint16 roomId = roomVoids[randomVoid];
		if(RandomRoomLocation(roomId, voidLoc, doorStates, landingGearStates, liftStops))
			return roomId;
	}
	while(tries--);
	return 0;
}

quint16 AuScParts::RandomSiteLocation(AuPoint& siteLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	QList<quint16> sites;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsSite())
			sites.append(pi.key());
	}
	if(sites.empty())
		return 0;

	int tries=10;
	do
	{
		int randomSite = g_Au.Dice().Roll(1, sites.size(), -1);
		quint16 siteId = sites[randomSite];
		if(RandomSiteLocation(siteId, siteLoc, doorStates, landingGearStates))
			return siteId;
	}
	while(tries--);
	return 0;
}

bool AuScParts::SurgeryPatientPos(quint16 partId, AuPoint& patientLoc, quint8& patientPos) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return false;
	if((*pi)->IsOmitted())
		return false;

	patientLoc = (*pi)->PatientLoc();
	patientPos = (*pi)->PatientPos();
	return true;
}

bool AuScParts::SurgeryUnitInRoom(quint16 roomId, AuPoint& patientLoc) const
{
	// Find the room.
	const_iterator ri = find(roomId);
	if(ri == cend())
		return false;
	if(!(*ri)->IsRoomVoid())
		return false;

	// Find a surgery station or console in the room.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		AuPoint ploc = (*pi)->PatientLoc();
		if((*ri)->IsInsideRoomVoid(ploc, *this))
		{
			patientLoc = ploc;
			return true;
		}
	}

	return false;
}

bool AuScParts::SurgeryUnitInRoom(quint16 roomId, AuPoint& patientLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	// Find the room.
	const_iterator ri = find(roomId);
	if(ri == cend())
		return false;
	if(!(*ri)->IsRoomVoid())
		return false;

	// Find a surgery station or console in the room.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		AuPoint ploc = (*pi)->PatientLoc();
		if((*ri)->IsInsideRoomVoid(ri.key(), ploc, doorStates, landingGearStates, liftStops))
		{
			patientLoc = ploc;
			return true;
		}
	}

	return false;
}

void AuScParts::CitizenPostLocs(QHash<quint16, AuPoint>& locs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsCitizenPost())
			locs.insert(pi.key(), (*pi)->GuardLoc());
	}
}

void AuScParts::GuardPostLocs(QHash<quint16, AuPoint>& locs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsGuardPost())
			locs.insert(pi.key(), (*pi)->GuardLoc());
	}
}

void AuScParts::LivestockPostLocs(QHash<quint16, AuPoint>& locs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsLivestockPost())
			locs.insert(pi.key(), (*pi)->GuardLoc());
	}
}

AuPoint AuScParts::HelmLocation() const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsHelm())
			return (*pi)->OperatorLoc();
	}
	return AuPoint(0,0);
}

AuPoint AuScParts::TargetLoc() const
{
	// Shoot at the center of the largest hull chunk.
	quint32 bhvol = 0;
	const_iterator bhi = cend();
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsHullMesh())
			continue;
		/*
		quint32 hvol = (*pi)->HullVolume(pi.key(), *this);
		*/
		AuPoint hmin,hmax;
		if(!(*pi)->HullExtents(hmin,hmax))
			continue;
		AuPoint v(hmax - hmin);
		quint32 hvol = (quint32)ceil(v.x * v.y * v.z);
		if(hvol > bhvol)
		{
			bhvol = hvol;
			bhi = pi;
		}
	}
	if(bhi != cend())
	{
		// Check center of largest hull part to see if it is in fact inside the hull.
		AuPoint min, max;
		(*bhi)->HullExtents(min, max);
		AuPoint hcen = (min + max)/2;
		if(IsInsideHull(hcen))
			return hcen;

		// Check center of any hull mass for the first one that is inside the hull.
		AuPoint acen(hcen);
		int hvols = 1;
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if(!(*pi)->IsHullMesh())
				continue;
			if(pi == bhi)
				continue;	// Already checked that one.
			AuPoint min, max;
			(*pi)->HullExtents(min, max);
			AuPoint hcen = (min + max)/2;
			if(IsInsideHull(hcen))
				return hcen;
			acen += hcen;
			hvols++;
		}

		// Really? Try average center of hull masses; it could be inside the hull.
		acen /= hvols;
		if(IsInsideHull(acen))
			return acen;
	}

	// No hull chunks? Shoot at the helm location.
	if(HasHelm())
		return HelmLocation();

	// No hull or helm? Not likely to get this far. Model origin will have to do.
	return AuPoint(0,0);
}

void AuScParts::ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestPowerPlant, AuPoint& closestSensor) const
{
	double bestP2 = -1;
	double bestS2 = -1;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		switch((*pi)->Sound())
		{
		case AuScPart::SOUND_Silent:
		case AuScPart::SOUND_FTLDrive:
		case AuScPart::SOUND_ManeuverDrive:
		case AuScPart::SOUND_Siren:
			break;
		case AuScPart::SOUND_PowerPlant:
			{
				AuPoint emitLoc = (*pi)->SoundEmitLoc();
				double dist2 = emitLoc.Distance2(searchLoc);
				if(bestP2 == -1 || dist2 < bestP2)
				{
					bestP2 = dist2;
					closestPowerPlant = emitLoc;
				}
			}
			break;
		case AuScPart::SOUND_Sensor:
			{
				AuPoint emitLoc = (*pi)->SoundEmitLoc();
				double dist2 = emitLoc.Distance2(searchLoc);
				if(bestS2 == -1 || dist2 < bestS2)
				{
					bestS2 = dist2;
					closestSensor = emitLoc;
				}
			}
			break;
		}
	}
}

void AuScParts::ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestManeuverDrive, AuPoint& closestPowerPlant,
		AuPoint& closestSensor, AuPoint& closestFTLDrive, AuPoint& closestSiren) const
{
	double bestF2 = -1;
	double bestM2 = -1;
	double bestP2 = -1;
	double bestR2 = -1;
	double bestS2 = -1;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		switch((*pi)->Sound())
		{
		case AuScPart::SOUND_Silent:
			break;
		case AuScPart::SOUND_Siren:
			{
				AuPoint emitLoc = (*pi)->SoundEmitLoc();
				double dist2 = emitLoc.Distance2(searchLoc);
				if(bestR2 == -1 || dist2 < bestR2)
				{
					bestR2 = dist2;
					closestSiren = emitLoc;
				}
			}
			break;
		case AuScPart::SOUND_FTLDrive:
			{
				AuPoint emitLoc = (*pi)->SoundEmitLoc();
				double dist2 = emitLoc.Distance2(searchLoc);
				if(bestF2 == -1 || dist2 < bestF2)
				{
					bestF2 = dist2;
					closestFTLDrive = emitLoc;
				}
			}
			break;
		case AuScPart::SOUND_ManeuverDrive:
			{
				AuPoint emitLoc = (*pi)->SoundEmitLoc();
				double dist2 = emitLoc.Distance2(searchLoc);
				if(bestM2 == -1 || dist2 < bestM2)
				{
					bestM2 = dist2;
					closestManeuverDrive = emitLoc;
				}
			}
			break;
		case AuScPart::SOUND_PowerPlant:
			{
				AuPoint emitLoc = (*pi)->SoundEmitLoc();
				double dist2 = emitLoc.Distance2(searchLoc);
				if(bestP2 == -1 || dist2 < bestP2)
				{
					bestP2 = dist2;
					closestPowerPlant = emitLoc;
				}
			}
			break;
		case AuScPart::SOUND_Sensor:
			{
				AuPoint emitLoc = (*pi)->SoundEmitLoc();
				double dist2 = emitLoc.Distance2(searchLoc);
				if(bestS2 == -1 || dist2 < bestS2)
				{
					bestS2 = dist2;
					closestSensor = emitLoc;
				}
			}
			break;
		}
	}

	if(bestM2 >= 0)
	{
		// A maneuver drive was found.
		if(bestP2 < 0)
			closestPowerPlant = closestManeuverDrive;			// No power plant found so emit its sound from the maneuver drive.
		if(bestF2 < 0)
			closestFTLDrive = closestManeuverDrive;				// No FTL drive found so emit its sound from the maneuver drive.
	}
}

bool AuScParts::HasSealedParts(const AuId& archId) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if(!(*pi)->IsSealed())
			continue;
		if((*pi)->ArchId().IsNull())
			continue;
		if((*pi)->ArchId() != archId)
			return true;
	}

	// No part sealed by another architect was found.
	return false;
}

bool AuScParts::HasSealedParts(const AuId& archId, const AuScSelection& sel) const
{
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		const_iterator pi = find(si.key());
		if(pi == cend())
			continue;

		if(!(*pi)->IsSealed())
			continue;
		if((*pi)->ArchId().IsNull())
			continue;
		if((*pi)->ArchId() != archId)
			return true;
	}

	// No part sealed by another architect was found.
	return false;
}

void AuScParts::ClearArchitectForFileSave(const AuId& id)
{
	for(iterator pi = begin(); pi != end(); pi++)
		(*pi)->ClearArchitectForFileSave(id);
}

void AuScParts::SetArchitectAfterFileLoad(const AuId& id, const QString& name)
{
	for(iterator pi = begin(); pi != end(); pi++)
		(*pi)->SetArchitectAfterFileLoad(id, name);
}

void AuScParts::PartArchitects(const AuId& archId, AuPartArchitectHash& partArchs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if((*pi)->ArchId() != archId &&
				(*pi)->ArchId() != AuId::Silverwill() &&
				!(*pi)->ArchId().IsNull() &&
				!(*pi)->ArchName().isEmpty() &&
				!partArchs.Contains((*pi)->ArchId()))
			partArchs.Insert((*pi)->ArchId(), (*pi)->ArchName());
}

void AuScParts::PathSegments(AuScPathSegList& segs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		for(int seg=0; seg<(*pi)->PathSegments(); seg++)
			segs.append((*pi)->PathSegment(seg));
}

void AuScParts::Finalize(bool shipHull, QProgressDialog* progress)
{
	iterator pi = begin();
	while(pi != end())
	{
		if((*pi)->Finalize(pi.key(), *this, shipHull))
			pi++;
		else
		{
			delete *pi;
			pi = erase(pi);
		}
		if(progress)
		{
			progress->setValue(progress->value() + 1);
			if(progress->wasCanceled())
				return;
		}
	}
}

void AuScParts::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if(!(*pi)->IsOmitted())
			(*pi)->ListComponents(pc, streamline, armor, hullMaterial);
}

quint16 AuScParts::TurboLiftForDoor(quint16 doorId, const AuScDoor& door) const
{
	// The most reliable method is to find a path associated with the door, then we can see if either end of the path is inside a
	// room void that is a state of a turbo lift. That is definitely a door at a turbo lift stop.
	// We will also watch for lifts that happen to have a stop at the door, as a fallback. The architect might not have made paths.
	quint16 liftId = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsPath())
		{
			if((*pi)->AssociatedPartId() != doorId)
				continue;
			if((*pi)->PathSegments() != 1)
				continue;

			const AuScPathSeg& seg = (*pi)->PathSegment(0);
			// Check the room at the first end.
			quint16 room1 = RoomVoidContainingLoc(seg.Loc1(), .5);
			if(room1)
			{
				const_iterator ri = find(room1);
				if(ri != cend() && (*ri)->IsTurboLiftMesh())
					return ri.key();	// Path associated with the door ends in a turbo lift.
			}

			// Check the room at the other end.
			quint16 room2 = RoomVoidContainingLoc(seg.Loc2(), .5);
			if(room2)
			{
				const_iterator ri = find(room2);
				if(ri != cend() && (*ri)->IsTurboLiftMesh())
					return ri.key();	// Path associated with the door ends in a turbo lift.
			}
		}
		else if((*pi)->IsTurboLiftMesh())
		{
			// We found a turbo lift. It is the likely one if it has a stop at the door. The door name is supposed match the stop.
			QStringList stopNames = (*pi)->StopNames();
			if(stopNames.contains(door.StopName()))
				liftId = pi.key();
		}
	}

	// Didn't find the turbo lift using the path. Return the likely lift, if we found one.
	return liftId;
}

quint16 AuScParts::LauncherForDoor(quint16 doorId, const AuScDoor& door) const
{
	// Treat the door as closed.
	AuDoorStateHash doorStates;
	doorStates.insert(doorId, AuScDoorState(AuScDoor::Closed));

	// Look for a launcher path that is blocked by the closed state of the door.
	AuMatrix4x4 launchPadM;
	AuPoint launchExit;
	AuPoint hitLoc, hitNormal;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->LaunchPosition(launchPadM, launchExit))
			continue;	// Not a launcher.
		AuPoint fromLoc = launchPadM.Translation();
		if(door.IsPathBlocked(doorId, fromLoc, launchExit, hitLoc, hitNormal, doorStates, AuLandingGearStateHash(), AuLiftHash()))
			return pi.key();			// Found a launcher whose path goes through the closed door.
	}
	return 0;
}

// This is used when collecting the initial states of all doors for a newly created ship.
void AuScParts::DoorStates(AuDoorStateHash& doors) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsDoor())
			continue;
		const AuScDoor* door = dynamic_cast<const AuScDoor*>(*pi);
		if(!door)
			continue;

		// Get the door switch locations.
		AuPoint opLoc1, opLoc2;
		door->DoorOpLocs(opLoc1, opLoc2);

		// Locate the lift for this door, if any.
		quint16 liftId;
		if(door->IsTurboLiftDoor())
			liftId = TurboLiftForDoor(pi.key(), *door);	// This door has a name so it is a lift door.
		else
			liftId = 0;

		// If it's not a lift door, it could be a launch tube hull door.
		quint16 launcherId;
		if(!liftId)
			launcherId = LauncherForDoor(pi.key(), *door);
		else
			launcherId = 0;

		// What kind of door is this?
		AuScDoorState::DOORTYPE doorType;
		quint16 partId = 0;
		bool isManual = false;
		if(liftId)
		{
			if(door->IsHullDoor())
				doorType = AuScDoorState::DOOR_HullLift;		// Hull turbo lift door.
			else
				doorType = AuScDoorState::DOOR_InteriorLift;	// Interior turbo lift door.
			partId = liftId;
		}
		else if(launcherId)
		{
			doorType = AuScDoorState::DOOR_Launcher;			// Launch Tube door.
			partId = launcherId;
		}
		else if(door->IsHullDoor())
			doorType = AuScDoorState::DOOR_Hull;				// Hull door.
		else
		{
			doorType = AuScDoorState::DOOR_Interior;			// Interior door.
			isManual = !door->IsAutomaticDoor();				// Only plain interior doors can be manual vs automatic.
		}

		AuScDoorState doorState(doorType, opLoc1, opLoc2, partId, door->State(), isManual, door->IsRemoteSpacecraftDoor(),
				door->IsRemoteVehicleDoor());
		doorState.SetLock(door->IsInitiallyLocked());
		doors.insert(pi.key(), doorState);
	}
}

void AuScParts::LandingGear(AuScPartSet& landingGear) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsLandingGear())
			continue;
		landingGear.insert(pi.key());
	}
}

void AuScParts::RoomVoidNames(QStringList& rooms) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsRoomVoid())
			continue;
		QString rnid((*pi)->GroupName());
		if(rnid == d_GroupHull)
			continue;
		if(rooms.contains(rnid))
			continue;
		rooms.append(rnid);
	}
}

void AuScParts::TurboLifts(AuLiftHash& lifts) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsTurboLiftMesh())
			continue;
		lifts.insert(pi.key(), (*pi)->AtStop());
	}
}

void AuScParts::TurboLifts(AuScReportLiftHash& lifts) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsTurboLiftMesh())
			continue;

		AuLiftStopHash liftStops;
		TurboLiftStops(pi.key(), liftStops);
		lifts.insert(pi.key(), liftStops);
	}
}

void AuScParts::TurboLiftStopMatrix(quint16 liftId, const QString& stopName, AuMatrix4x4& stopM) const
{
	const_iterator pi = find(liftId);
	if(pi == cend())
		return;
	if((*pi)->IsOmitted())
		return;
	stopM = (*pi)->StopMatrixAt(stopName);
}

void AuScParts::TurboLiftStops(quint16 liftId, AuLiftStopHash& liftStops) const
{
	const_iterator pi = find(liftId);
	if(pi == cend())
		return;
	if((*pi)->IsOmitted())
		return;
	if(!(*pi)->IsTurboLiftMesh())
		return;

	// Build the list of stops.
	QStringList stopNames = (*pi)->StopNames();
	for(QStringList::const_iterator si = stopNames.cbegin(); si != stopNames.cend(); si++)
	{
		// Locate a door and a room void with the same name as the lift stop.
		quint16 doorId = 0;
		for(const_iterator di = cbegin(); di != cend() && !doorId; di++)
		{
			if((*di)->IsOmitted())
				continue;
			if((*di)->IsDoor())
			{
				if((*di)->StopName() == *si)
					doorId = di.key();	// Found a door with a matching stop name.
			}
		}
		liftStops.insert(*si, AuLiftStop(doorId));
	}
}

double AuScParts::GroundHeightAboveCG(GROUND_ORIENT gori) const
{
	AuPoint min, max;
	HullExtents(min, max);
	switch(gori)
	{
	case GORI_ZPosUp:
		break;
	case GORI_ZNegUp:
		return max.z;
	case GORI_YPosUp:
		return -min.y;
	case GORI_YNegUp:
		return max.y;
	case GORI_XPosUp:
		return -min.x;
	case GORI_XNegUp:
		return max.x;
	case GORI_Count:
		break;
	}
	return -min.z;
}

double AuScParts::GroundHeightAboveCG(const AuLandingGearStateHash& landingGearStates, LANDING_ORIENT lori) const
{
	AuPoint min, max;
	HullExtents(min, max, landingGearStates);
	switch(lori)
	{
	case LORI_Belly:
		break;
	case LORI_Tail:
		return -min.y;
	case LORI_Count:
		break;
	}
	return -min.z;
}

quint16 AuScParts::VoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, quint16 exceptVoidId) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(pi.key() == exceptVoidId)
			continue;
		if((*pi)->IsInsideVoid(pi.key(), loc, doorStates, landingGearStates, liftStops))
			return pi.key();
	}
	return 0;
}

quint16 AuScParts::VoidContainingLocConstant(const AuPoint& loc, quint16 exceptVoidId) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(pi.key() == exceptVoidId)
			continue;
		if((*pi)->AssociatedPartId())
			continue;	// A void that is associated to a part is variable.
		if((*pi)->IsTurboLiftMesh())
			continue;	// Caller does not want turbo lift voids.
		if((*pi)->IsInsideVoid(loc))
			return pi.key();
	}
	return 0;
}

quint16 AuScParts::VoidContainingLocVariable(const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, quint16 exceptVoidId) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(pi.key() == exceptVoidId)
			continue;
		if(!(*pi)->AssociatedPartId())
			continue;	// A void that is not associated to a part is constant.
		if((*pi)->IsTurboLiftMesh())
			continue;	// Caller does not want turbo lift voids.
		if((*pi)->IsInsideVoid(pi.key(), loc, doorStates, landingGearStates, liftStops))
			return pi.key();
	}
	return 0;
}

bool AuScParts::IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	// Find the closest ladder that has an end point at 'ladderPathLoc'.
	bool foundLadder = false;
	double dist = 1;					// Closest ladder we will find.
	AuMatrix4x4 m;						// Matrix returned by FindLadder.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if(!(*pi)->IsLadderPath())
			continue;
		const AuScPathSeg& seg = (*pi)->PathSegment(0);
		if(!seg.IsAt(ladderPathLoc))
			continue;	// The ladder path does not terminate at the 'ladderPathLoc'.

		// The path could be associated with a landing gear or a door that is not opened.
		quint16 assid = (*pi)->AssociatedPartId();
		if(assid)
		{
			// The path segment is associated with the state of a landing gear or door.
			QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(assid);
			if(gi != landingGearStates.m_States.cend())
			{
				// Skip path if associated landing gear is not in the desired state.
				if(!(*pi)->CompareAssociatedLandingGearState(*gi))
					continue;
			}
			else
			{
				// Check door state.
				AuDoorStateHash::const_iterator di = doorStates.find(assid);
				if(di != doorStates.cend())
				{
					// Skip path if associated door is not in the desired state.
					if(!(*pi)->CompareAssociatedDoorState((*di).State()))
						continue;
				}
			}
		}

		// Finds a ladder within 'dist' distance. Updates dist if one is found.
		double d = dist;				// Distance returned by FindLadder.
		if((*pi)->FindLadder(fromLoc, m, d))
		{
			// Found a ladder that is close enough.
			foundLadder = true;
			dist = d;				// Update distance to closest ladder.
		}
	}
	return foundLadder;
}

bool AuScParts::FindLadderToGrab(const AuPoint& fromLoc, AuMatrix4x4& mLadder, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	bool foundLadder = false;
	double dist = FeetPerMeter();		// Closest ladder we will find.
	AuMatrix4x4 m;						// Matrix returned by FindLadder.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		// The path could be associated with a landing gear or a door that is not opened.
		quint16 assid = (*pi)->AssociatedPartId();
		if(assid)
		{
			// The path segment is associated with the state of a landing gear or door.
			QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(assid);
			if(gi != landingGearStates.m_States.cend())
			{
				// Skip path if associated landing gear is not in the desired state.
				if(!(*pi)->CompareAssociatedLandingGearState(*gi))
					continue;
			}
			else
			{
				// Check door state.
				AuDoorStateHash::const_iterator di = doorStates.find(assid);
				if(di != doorStates.cend())
				{
					// Skip path if associated door is not in the desired state.
					if(!(*pi)->CompareAssociatedDoorState((*di).State()))
						continue;
				}
			}
		}

		// Finds a ladder within 'dist' distance. Updates dist if one is found.
		double d = dist;				// Distance returned by FindLadder.
		if((*pi)->FindLadder(fromLoc, m, d))
		{
			// Found a ladder that is close enough.
			// Check to see if the path is blocked to the ladder location.
			AuPoint toLoc = m.Translation();
			AuPoint hitLoc, hitNormal;
			if(!IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops))
			{
				// Path to the ladder is not blocked.
				foundLadder = true;
				dist = d;				// Update distance to unblocked ladder.
				mLadder = m;			// Update matrix of unblocked ladder.
			}
		}
	}
	return foundLadder;
}

bool AuScParts::FindLadderToGrabAt(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	bool foundLadder = false;
	double dist = FeetPerMeter();		// Closest ladder we will find.
	AuMatrix4x4 m;						// Matrix returned by FindLadder.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if(!(*pi)->IsLadderPath())
			continue;
		const AuScPathSeg& seg = (*pi)->PathSegment(0);
		if(!seg.IsAt(ladderPathLoc))
			continue;	// The ladder path does not terminate at the 'ladderPathLoc'.

		// The path could be associated with a landing gear or a door that is not opened.
		quint16 assid = (*pi)->AssociatedPartId();
		if(assid)
		{
			// The path segment is associated with the state of a landing gear or door.
			QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(assid);
			if(gi != landingGearStates.m_States.cend())
			{
				// Skip path if associated landing gear is not in the desired state.
				if(!(*pi)->CompareAssociatedLandingGearState(*gi))
					continue;
			}
			else
			{
				// Check door state.
				AuDoorStateHash::const_iterator di = doorStates.find(assid);
				if(di != doorStates.cend())
				{
					// Skip path if associated door is not in the desired state.
					if(!(*pi)->CompareAssociatedDoorState((*di).State()))
						continue;
				}
			}
		}

		// Finds a ladder within 'dist' distance. Updates dist if one is found.
		double d = dist;				// Distance returned by FindLadder.
		if((*pi)->FindLadder(fromLoc, m, d))
		{
			// Found a ladder that is close enough.
			// Check to see if the path is blocked to the ladder location.
			AuPoint toLoc = m.Translation();
			AuPoint hitLoc, hitNormal;
			if(!IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops))
			{
				// Path to the ladder is not blocked.
				foundLadder = true;
				dist = d;				// Update distance to unblocked ladder.
				mLadder = m;			// Update matrix of unblocked ladder.
			}
		}
	}
	return foundLadder;
}

bool AuScParts::Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, LANDING_ORIENT orient, bool canUnlockDoors,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::Route check direct path" << AuTime::Now();
	// Check for clear path directly to the destination.
	AuPoint hitLoc, hitNormal;
	if(IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops))
	{
		if(g_Au.m_LogMessages & LOG_Path)
			qDebug() << "AuScParts::Route direct path blocked from" << fromLoc << "to" << toLoc << AuTime::Now();
		// Direct path to destination is blocked.
		// Our strategy changes if we know we are moving to or from the outside.
		bool fromInside = IsInsideObstructionModel(fromLoc, doorStates, landingGearStates);
		bool toInside = IsInsideObstructionModel(toLoc, doorStates, landingGearStates);
		if(!fromInside && !toInside)
		{
			// Both points are outside my obstruction model, plot a path around me.
			if(g_Au.m_LogMessages & LOG_Path)
				qDebug() << "AuScParts::Route around" << AuTime::Now();
			if(!RouteAround(route, fromLoc, toLoc, landingGearStates))
				return false;
		}
		else if(fromInside && !toInside)
		{
			// Path from inside to outside.
			if(g_Au.m_LogMessages & LOG_Path)
				qDebug() << "AuScParts::Route inside to outside" << AuTime::Now();
			// Find the closest unblocked path node to the fromLoc.
			AuPoint fromNode;
			if(!ClosestPathNode(fromLoc, doorStates, landingGearStates, liftStops, fromNode))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no node near fromLoc" << fromLoc << AuTime::Now();
				return false;
			}

			// Find the closest exit node to the toLoc.
			AuPoint exitNode;
			if(!ClosestHullExitNode(toLoc, orient, doorStates, landingGearStates, liftStops, exitNode))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no hull exit node near toLoc" << toLoc << AuTime::Now();
				return false;
			}

			// Start by walking to the closest unobstructed from node.
			route.append(AuScRouteLoc(fromNode));

			// Walk the path network to the exit node. Adds nothing if they are the same.
			if(!RouteToNode(route, exitNode, canUnlockDoors, doorStates, landingGearStates))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no route to exit node" << exitNode << AuTime::Now();
				return false;
			}

			// Move from the exit to the destination.
			if(IsPathBlocked(exitNode, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops) &&
					!RouteAround(route, exitNode, toLoc, landingGearStates))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route failed pathBlocked"
						<< IsPathBlocked(exitNode, toLoc, hitLoc, hitNormal, doorStates, landingGearStates, liftStops)
						<< "routeAround" << RouteAround(route, exitNode, toLoc, landingGearStates) << AuTime::Now();
				return false;
			}
		}
		else if(!fromInside && toInside)
		{
			// Path from outside to inside.
			if(g_Au.m_LogMessages & LOG_Path)
				qDebug() << "AuScParts::Route outside to inside" << AuTime::Now();
			// Find the closest exit node to the fromLoc.
			AuPoint entranceNode;
			if(!ClosestHullExitNode(fromLoc, orient, doorStates, landingGearStates, liftStops, entranceNode))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no hull exit node near fromLoc" << fromLoc << AuTime::Now();
				return false;
			}

			// Find the closest unblocked path node to the toLoc.
			AuPoint toNode;
			if(!ClosestPathNode(toLoc, doorStates, landingGearStates, liftStops, toNode))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no node near toLoc" << toLoc << AuTime::Now();
				return false;
			}

			// Move from the start to the entrance.
			if(IsPathBlocked(fromLoc, entranceNode, hitLoc, hitNormal, doorStates, landingGearStates, liftStops) &&
					!RouteAround(route, fromLoc, entranceNode, landingGearStates))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route failed pathBlocked"
						<< IsPathBlocked(fromLoc, entranceNode, hitLoc, hitNormal, doorStates, landingGearStates, liftStops)
						<< "routeAround" << RouteAround(route, fromLoc, entranceNode, landingGearStates) << AuTime::Now();
				return false;
			}

			// Start by walking to the closest unobstructed from node.
			route.append(AuScRouteLoc(entranceNode));

			// Walk the path network from the best from node to the best to node. Adds nothing if they are the same.
			if(!RouteToNode(route, toNode, canUnlockDoors, doorStates, landingGearStates))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no route to node" << toNode << AuTime::Now();
				return false;
			}
		}
		else
		{
			// Path from one inside location to another.
			if(g_Au.m_LogMessages & LOG_Path)
				qDebug() << "AuScParts::Route inside. Locating fromNode" << AuTime::Now();
			// Find the closest unblocked path node to the fromLoc.
			AuPoint fromNode;
			if(!ClosestPathNode(fromLoc, doorStates, landingGearStates, liftStops, fromNode))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no node near fromLoc" << fromLoc << AuTime::Now();
				return false;
			}

			if(g_Au.m_LogMessages & LOG_Path)
				qDebug() << "AuScParts::Route inside. Locating toNode" << AuTime::Now();
			// Find the closest unblocked path node to the toLoc.
			AuPoint toNode;
			if(!ClosestPathNode(toLoc, doorStates, landingGearStates, liftStops, toNode))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no node near toLoc" << toLoc << AuTime::Now();
				return false;
			}

			// Start by walking to the closest unobstructed from node.
			route.append(AuScRouteLoc(fromNode));

			if(g_Au.m_LogMessages & LOG_Path)
				qDebug() << "AuScParts::Route inside route to node" << AuTime::Now();
			// Walk the path network from the best from node to the best to node. Adds nothing if they are the same.
			if(!RouteToNode(route, toNode, canUnlockDoors, doorStates, landingGearStates))
			{
				if(g_Au.m_LogMessages & LOG_Path)
					qDebug() << "AuScParts::Route no route to node" << toNode << AuTime::Now();
				return false;
			}
		}
	}
	else
		if(g_Au.m_LogMessages & LOG_Path)
			qDebug() << "AuScParts::Route direct path not blocked" << AuTime::Now();

	// Successful ending walks to the toLoc.
	if(route.empty() || route.last() != toLoc)
		route.append(AuScRouteLoc(toLoc));

	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::Route complete" << route << AuTime::Now();

	// Route is complete.
	return true;
}

bool AuScParts::ClosestHullExitNode(const AuPoint& toLoc, LANDING_ORIENT orient, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& closestLoc) const
{
	double closestTo = -1;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if(!(*pi)->IsPath())
			continue;

		const AuScPathSeg& seg = (*pi)->PathSegment(0);

		// Check for closest end to the loc that is not inside the hull.
		if(!IsInsideHull(seg.Loc1(), doorStates, landingGearStates, liftStops))
		{
			double d = toLoc.Distance(seg.Loc1());
			if(closestTo < 0 || d < closestTo)
			{
				closestTo = d;
				closestLoc = seg.Loc1();
			}
		}
		if(!IsInsideHull(seg.Loc2(), doorStates, landingGearStates, liftStops))
		{
			double d = toLoc.Distance(seg.Loc2());
			if(closestTo < 0 || d < closestTo)
			{
				closestTo = d;
				closestLoc = seg.Loc2();
			}
		}
	}
	if(closestTo < 0)
		return false;

	// An exit node was found.
	if(orient == LORI_Tail)
		FollowExteriorLadderBack(closestLoc, toLoc.y, landingGearStates, false);
	else
		FollowExteriorLadderDown(closestLoc, toLoc.z, landingGearStates, false);

	return true;
}

bool AuScParts::ClosestPathNode(const AuPoint& toLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& closestLoc) const
{
	QMap<double, AuPoint> pathDistLocs;	// Key is distance squared from toLoc to segment end loc.
	QMap<AuPoint, bool> pathLocs;		// Key is every loc stored in pathDistLocs.
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		int pathSegments = (*pi)->PathSegments();
		for(int segIndex=0; segIndex<pathSegments; segIndex++)
		{
			// Calculate distance to segment end locs.
			const AuScPathSeg& seg = (*pi)->PathSegment(segIndex);
			if(!pathLocs.contains(seg.Loc1()))
			{
				double d = toLoc.Distance2(seg.Loc1());
				pathDistLocs.insert(d, seg.Loc1());
				pathLocs.insert(seg.Loc1(), true);
			}
			if(!pathLocs.contains(seg.Loc2()))
			{
				double d = toLoc.Distance2(seg.Loc2());
				pathDistLocs.insert(d, seg.Loc2());
				pathLocs.insert(seg.Loc2(), true);
			}
		}
	}

	// Check path locations, from nearest to farthest. Accept the first one that is not blocked.
	AuPoint hitLoc, hitNormal;
	for(QMap<double, AuPoint>::const_iterator pli = pathDistLocs.cbegin(); pli != pathDistLocs.cend(); pli++)
	{
		if(!IsPathBlocked(toLoc, *pli, hitLoc, hitNormal, doorStates, landingGearStates, liftStops))
		{
			closestLoc = *pli;
			return true;
		}
	}
	return false;
}

bool AuScParts::RouteToNode(AuScRoute& route, const AuPoint& toNode, bool canUnlockDoors, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	// Route to 'toNode' from the current end of the route.
	Q_ASSERT(!route.empty());
	QList<AuScRouteLoc>::const_iterator rli = route.cend() - 1;
	if((*rli).FuzzyEquals(toNode))
		return true;

	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::RouteToNode" << toNode << "canUnlock" << canUnlockDoors << "route size" << route.size()
			<< "collecting pathSegs" << AuTime::Now();
	// Get a list of all path segments.
	QMultiHash<AuScPart*, AuScPathSeg> pathSegs;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		int pathSegments = (*pi)->PathSegments();
		if(!pathSegments)
			continue;

		// The path could be associated with a landing gear or a door that cannot be opened.
		quint16 assid = (*pi)->AssociatedPartId();
		if(assid)
		{
			// The path segment is associated with the state of a landing gear or door.
			QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(assid);
			if(gi != landingGearStates.m_States.cend())
			{
				// Skip path if associated landing gear is not in the desired state.
				if(!(*pi)->CompareAssociatedLandingGearState(*gi))
					continue;
			}
			else if(!canUnlockDoors)
			{
				// Check door state.
				AuDoorStateHash::const_iterator di = doorStates.find(assid);
				if(di != doorStates.cend() && (*di).IsLocked())
				{
					// Skip path if associated door is not in the desired state.
					if(!(*pi)->CompareAssociatedDoorState((*di).State()))
						continue;
				}
			}
		}

		// Get all path segments from the part.
		for(int segIndex=0; segIndex<pathSegments; segIndex++)
			pathSegs.insert(*pi, (*pi)->PathSegment(segIndex));
	}
	if(pathSegs.empty())
		return false;
	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::RouteToNode pathSegs" << pathSegs.size() << AuTime::Now();

	// Classic traveling salesman algorithm.
	// Init travel cost on all segments that end at the ending point.
	QMultiHash<AuScPart*, AuScPathSeg>::iterator firstSeg = pathSegs.end();
	for(QMultiHash<AuScPart*, AuScPathSeg>::iterator si = pathSegs.begin(); si != pathSegs.end(); si++)
		if((*si).IsAt(toNode))
		{
			(*si).m_Cost = 1;

			// Did we reach our starting point?
			if((*si).IsAt(*rli))
			{
				firstSeg = si;
				break;
			}
		}
	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::RouteToNode initial cost assigned" << AuTime::Now();

	if(firstSeg == pathSegs.end())
	{
		// The starting point was not at the first segment.
		// Set path costs on adjoining segments until we reach the starting point.
		int curCost = 1;
		bool anySet;
		do
		{
			anySet = false;
			for(QMultiHash<AuScPart*, AuScPathSeg>::iterator si = pathSegs.begin();
					si != pathSegs.end() && firstSeg == pathSegs.end(); si++)
			{
				// Find seg at current cost.
				if((*si).m_Cost != curCost)
					continue;

				// Find adjoining segments with no cost applied and assign them the next higher step cost.
				for(QMultiHash<AuScPart*, AuScPathSeg>::iterator ai = pathSegs.begin(); ai != pathSegs.end(); ai++)
				{
					if((*ai).m_Cost)
						continue;		// Skip seg with cost already applied.
					if((*ai).IsAt(*si))
					{
						(*ai).m_Cost = curCost + 1;		// Segment abuts a segment at current cost.
						anySet = true;

						// Did we reach our starting point?
						if((*ai).IsAt(*rli))
						{
							firstSeg = ai;
							break;
						}
					}
				}
			}
			curCost++;
		}
		while(anySet && firstSeg == pathSegs.end());
	}
	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::RouteToNode all costs assigned." << (firstSeg != pathSegs.end() ? "path found" : "no path")
			<< AuTime::Now();

	// Did we find a path?
	if(firstSeg == pathSegs.end())
		return false;			// No path was found.

	// Remove path segments that did not get a cost assigned, so they don't slow down the next step.
	QMultiHash<AuScPart*, AuScPathSeg>::iterator ai = pathSegs.begin();
	while(ai != pathSegs.end())
		if((*ai).m_Cost)
			ai++;
		else
			ai = pathSegs.erase(ai);
	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::RouteToNode path distilled." << pathSegs.size() << AuTime::Now();

	// Walk the path from the beginning to the end, finding adjoining segments with lower step costs.
	QString liftStop;
	AuPoint stepLoc = (*firstSeg).PathStepLoc(*rli, liftStop);
	bool isLadder = firstSeg.key()->IsLadderPath();
	if(isLadder)
		(*(route.end()-1)).SetLadder(true);	// Set ladder flag on previous route step because it began the ladder.
	quint16 liftId = (*firstSeg).LiftId();
	if(liftId)
		(*(route.end()-1)).SetLift(liftId, liftStop);	// Set stop info on prev step because it began the lift segment.
	quint16 assid = firstSeg.key()->AssociatedPartId();
	if(assid && doorStates.contains(assid))
		(*(route.end()-1)).SetDoorId(assid);	// Set door id on previous route step because it began the door.
	route.append(AuScRouteLoc(stepLoc));
	for(int curCost = (*firstSeg).m_Cost - 1; curCost; curCost--)
	{
		// Find seg at current cost that shares an end loc with the next step location.
		for(QMultiHash<AuScPart*, AuScPathSeg>::iterator si = pathSegs.begin(); si != pathSegs.end(); si++)
		{
			if((*si).m_Cost != curCost)
				continue;

			if(!(*si).IsAt(stepLoc))
				continue;

			stepLoc = (*si).PathStepLoc(stepLoc, liftStop);
			isLadder = si.key()->IsLadderPath();
			if(isLadder)
				(*(route.end()-1)).SetLadder(true);	// Set ladder flag on previous route step because it began the ladder.
			liftId = (*si).LiftId();
			if(liftId)
				(*(route.end()-1)).SetLift(liftId, liftStop);	// Set stop info on prev step because it began the lift segment.
			assid = si.key()->AssociatedPartId();
			if(assid && doorStates.contains(assid))
				(*(route.end()-1)).SetDoorId(assid);	// Set door id on previous route step because it began the door.
			route.append(AuScRouteLoc(stepLoc));
			break;
		}
	}
	if(g_Au.m_LogMessages & LOG_Path)
		qDebug() << "AuScParts::RouteToNode walked path back, route size" << route.size() << AuTime::Now();
	return true;
}

bool AuScParts::RouteAround(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc,
		const AuLandingGearStateHash& landingGearStates) const
{
	// Both points are outside my obstruction model, plot a path around me.
	AuPoint min,max;
	HullExtents(min, max, landingGearStates);			// 3D extents of my geometry.
	min -= AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	max += AuPoint(d_ObstructMargin, d_ObstructMargin, d_ObstructMargin);
	AuPoint extCen((min + max)/2);						// Center of extents.
	double extRad = extCen.Distance_2D(max);			// Radius of circle centered on extents that contains extents.
	extRad += FeetPerMeter() * 2;						// Margin

	AuPoint fvec(fromLoc - extCen);
	fvec.z = 0;
	double fd = fvec.Length_2D();
	fvec /= fd;

	AuPoint tvec(toLoc - extCen);
	tvec.z = 0;
	double td = tvec.Length_2D();
	tvec /= td;

	// Move away from the geometry if necessary.
	fvec *= extRad;
	if(fd < extRad)
		route.append(AuScRouteLoc(extCen + fvec));

	if(tvec.Dot_2D(fvec) > 0)
	{
		// The path does not cross my extents circle.
		route.append(AuScRouteLoc(extCen + tvec * extRad));
		return true;
	}

	AuPoint vec(toLoc - fromLoc);
	vec.z = 0;
	double dist = vec.Length_2D();
	vec /= dist;

	// Move around my extents.
	AuPoint pvec(vec.RotateZ(PI/2));
	AuPoint cvec(extCen - fromLoc);
	double pd = cvec.Dot_2D(pvec);						// Positive if extCen is to the left of my path.
	float c = (float)pd / (float)extRad;				// Cosine of the angle between perp and line to fromLoc from extCen.
	float a = acosf(c);									// Angle between perp and line to fromLoc.
	float sweep = fabsf(a * 2);							// Sweep angle to walk around my extents.
	int stepsAround = sweep * 4 / PI + 1;				// 1 to 5 steps around my extents.
	float stepAng = sweep / stepsAround;
	for(int step=0; step<stepsAround; step++)
	{
		if(pd > 0)
			fvec = fvec.RotateZ(stepAng);				// Go around me counter-clockwise.
		else
			fvec = fvec.RotateZ(-stepAng);				// Go around me clockwise.
		route.append(AuScRouteLoc(extCen + fvec));
	}
	return true;
}

void AuScParts::ContrailEmitters(QMap<AuPoint, float>& contrails) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->IsContrailEmitter())
			continue;

		contrails.insert((*pi)->SoundEmitLoc(), (*pi)->EmitterDiameter());
	}
}

bool AuScParts::ClosestEntrance(AuPoint& entranceLoc, quint16& doorId, quint16& ladderId, const AuPoint& loc, LANDING_ORIENT orient,
		bool useGroundLevelDoors, bool hullDoorsUsable, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	doorId = 0;

	AuPoint tloc(loc);

	AuPoint min, max;
	if(useGroundLevelDoors)
	{
		HullExtents(min, max, landingGearStates);
		tloc.z = min.z;
	}

	double closestTo = -1;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if(!(*pi)->IsPath())
			continue;

		bool isDoor = false;
		if((*pi)->AssociatedPartId())
		{
			// Path segment may be a door.
			AuDoorStateHash::const_iterator di = doorStates.find((*pi)->AssociatedPartId());
			if(di != doorStates.cend())
			{
				// The path segment is associated with a door. Is it a hull door?
				if(!(*di).IsHullDoor())
					continue;	// Skip interior door path segment.
				if(!hullDoorsUsable && (*di).IsClosed())
					continue;	// Skip hull door path segment.
				isDoor = true;
			}
		}

		const AuScPathSeg& seg = (*pi)->PathSegment(0);

		if(!isDoor)
		{
			// The path segment is not a door. Skip it if neither end is inside a room void. A path segment with one end inside
			// a room void and the other end outside the hull is an entrance.
			if(!RoomVoidContainingLoc(seg.Loc1(), doorStates, landingGearStates, liftStops) &&
					!RoomVoidContainingLoc(seg.Loc2(), doorStates, landingGearStates, liftStops))
				continue;	// Neither end of the non-door segment is inside a room void.
		}

		// Check for closest end to the loc that is not inside the hull. If neither end is outside the hull, the segment is skipped.
		{
			// End 1
			// Check ground proximity when it is a factor.
			bool groundOk = true;
			if(useGroundLevelDoors)
			{
				// Entrance cannot be below the ship or above a reasonable ground height.
				AuPoint gloc(seg.Loc1());
				if(orient == LORI_Tail)
				{
					if(FollowExteriorLadderBack(gloc, tloc.y, landingGearStates, true))
					{
						if(gloc.y < min.y || gloc.y > min.y + FeetPerMeter()*2)
							groundOk = false;
					}
					else
					{
						if(seg.Loc1().y < min.y || seg.Loc1().y > min.y + FeetPerMeter()*2)
							groundOk = false;
					}
				}
				else
				{
					if(FollowExteriorLadderDown(gloc, tloc.z, landingGearStates, true))
					{
						if(gloc.z < min.z || gloc.z > min.z + FeetPerMeter()*2)
							groundOk = false;
					}
					else
					{
						if(seg.Loc1().z < min.z || seg.Loc1().z > min.z + FeetPerMeter()*2)
							groundOk = false;
					}
				}
			}

			if(groundOk && !IsInsideHull(seg.Loc1(), doorStates, landingGearStates, liftStops))
			{
				double d = tloc.Distance(seg.Loc1());
				if(closestTo < 0 || d < closestTo)
				{
					closestTo = d;
					entranceLoc = seg.Loc1();
					if(isDoor)
						doorId = pi.key();
				}
			}
		}

		{
			// End 2
			// Check ground proximity when it is a factor.
			bool groundOk = true;
			if(useGroundLevelDoors)
			{
				// Entrance cannot be below the ship or above a reasonable ground height.
				AuPoint gloc(seg.Loc2());
				if(orient == LORI_Tail)
				{
					if(FollowExteriorLadderBack(gloc, tloc.y, landingGearStates, true))
					{
						if(gloc.y < min.y || gloc.y > min.y + FeetPerMeter()*2)
							groundOk = false;
					}
					else
					{
						if(seg.Loc2().y < min.y || seg.Loc2().y > min.y + FeetPerMeter()*2)
							groundOk = false;
					}
				}
				else
				{
					if(FollowExteriorLadderDown(gloc, tloc.z, landingGearStates, true))
					{
						if(gloc.z < min.z || gloc.z > min.z + FeetPerMeter()*2)
							groundOk = false;
					}
					else
					{
						if(seg.Loc2().z < min.z || seg.Loc2().z > min.z + FeetPerMeter()*2)
							groundOk = false;
					}
				}
			}

			if(groundOk && !IsInsideHull(seg.Loc2(), doorStates, landingGearStates, liftStops))
			{
				double d = tloc.Distance(seg.Loc2());
				if(closestTo < 0 || d < closestTo)
				{
					closestTo = d;
					entranceLoc = seg.Loc2();
					if(isDoor)
						doorId = pi.key();
				}
			}
		}
	}
	if(closestTo < 0)
		return false;

	// An exit node was found.
	// We're done if any door will do.
	if(!useGroundLevelDoors)
		return true;

	// Follow any ladders down from the exit. We don't want to return an entrance point that is out of reach.
	if(orient == LORI_Tail)
		ladderId = FollowExteriorLadderBack(entranceLoc, tloc.y, landingGearStates, useGroundLevelDoors);
	else
		ladderId = FollowExteriorLadderDown(entranceLoc, tloc.z, landingGearStates, useGroundLevelDoors);
	if(ladderId)
		doorId = 0;	// We found a ladder so the entrance loc is no longer at the door.

	return true;
}

quint16 AuScParts::FollowExteriorLadderBack(AuPoint& ladderLoc, double targetY, const AuLandingGearStateHash& landingGearStates,
		bool locAtTargetY) const
{
	// Ladder loc may already be low enough for the target.
	if(ladderLoc.y < targetY)
		return 0;

	// Return value will be part id of the last ladder segment, if we climb down any ladders.
	quint16 wentDownLadder = 0;

	// Follow ladders down from the ladder loc.
	bool foundLadderDown;
	QSet<quint16> laddersTraversed;
	AuPoint topLoc;
	do
	{
		foundLadderDown = false;
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if(!(*pi)->IsLadderPath())
				continue;
			if(laddersTraversed.contains(pi.key()))
				continue;

			// Check ladder associated landing gear.
			if((*pi)->AssociatedPartId())
			{
				// Skip ladder paths that are not usable due to landing gear state.
				QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find((*pi)->AssociatedPartId());
				if(!(*pi)->CompareAssociatedLandingGearState(*gi))
					continue;
			}

			const AuScPathSeg& seg = (*pi)->PathSegment(0);
			if(seg.Loc1().y > seg.Loc2().y)
			{
				// Segment loc 1 is the top end.
				if(ladderLoc.FuzzyEquals(seg.Loc1()))
				{
					topLoc = seg.Loc1();
					ladderLoc = seg.Loc2();
					laddersTraversed.insert(pi.key());
					foundLadderDown = true;
					wentDownLadder = pi.key();
					break;
				}
			}
			else
			{
				// Segment loc 2 is the top end or the ladder is horizontal.
				if(ladderLoc.FuzzyEquals(seg.Loc2()))
				{
					topLoc = seg.Loc2();
					ladderLoc = seg.Loc1();
					laddersTraversed.insert(pi.key());
					foundLadderDown = true;
					wentDownLadder = pi.key();
					break;
				}
			}
		}
	}
	while(foundLadderDown && ladderLoc.y > targetY);

	if(locAtTargetY && foundLadderDown)
	{
		// Calculate point on final ladder segment that is at the target Y.
		double drop = topLoc.y - ladderLoc.y;
		if(drop > 0)
		{
			double dest = topLoc.y - targetY;
			double frac = dest / drop;
			AuPoint lvec(ladderLoc - topLoc);
			ladderLoc = topLoc + lvec * frac;
		}
	}

	return wentDownLadder;
}

quint16 AuScParts::FollowExteriorLadderDown(AuPoint& ladderLoc, double targetZ, const AuLandingGearStateHash& landingGearStates,
		bool locAtTargetZ) const
{
	// Ladder loc may already be low enough for the target.
	if(ladderLoc.z < targetZ)
		return 0;

	// Return value will be part id of the last ladder segment, if we climb down any ladders.
	quint16 wentDownLadder = 0;

	// Follow ladders down from the ladder loc.
	bool foundLadderDown;
	QSet<quint16> laddersTraversed;
	AuPoint topLoc;
	do
	{
		foundLadderDown = false;
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if(!(*pi)->IsLadderPath())
				continue;
			if(laddersTraversed.contains(pi.key()))
				continue;

			// Check ladder associated landing gear.
			if((*pi)->AssociatedPartId())
			{
				// Skip ladder paths that are not usable due to landing gear state.
				QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find((*pi)->AssociatedPartId());
				if(!(*pi)->CompareAssociatedLandingGearState(*gi))
					continue;
			}

			const AuScPathSeg& seg = (*pi)->PathSegment(0);
			if(seg.Loc1().z > seg.Loc2().z)
			{
				// Segment loc 1 is the top end.
				if(ladderLoc.FuzzyEquals(seg.Loc1()))
				{
					topLoc = seg.Loc1();
					ladderLoc = seg.Loc2();
					laddersTraversed.insert(pi.key());
					foundLadderDown = true;
					wentDownLadder = pi.key();
					break;
				}
			}
			else
			{
				// Segment loc 2 is the top end or the ladder is horizontal.
				if(ladderLoc.FuzzyEquals(seg.Loc2()))
				{
					topLoc = seg.Loc2();
					ladderLoc = seg.Loc1();
					laddersTraversed.insert(pi.key());
					foundLadderDown = true;
					wentDownLadder = pi.key();
					break;
				}
			}
		}
	}
	while(foundLadderDown && ladderLoc.z > targetZ);

	if(locAtTargetZ && foundLadderDown)
	{
		// Calculate point on final ladder segment that is at the target Z.
		double drop = topLoc.z - ladderLoc.z;
		if(drop > 0)
		{
			double dest = topLoc.z - targetZ;
			double frac = dest / drop;
			AuPoint lvec(ladderLoc - topLoc);
			ladderLoc = topLoc + lvec * frac;
		}
	}

	return wentDownLadder;
}

bool AuScParts::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	bool hitMe = false;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if((*pi)->Sight(pi.key(), eyeLoc, sightLoc, doorStates, landingGearStates, liftStops, sightSurfaceNormal))
		{
			// As we hit each face, sightLoc is shortened. By the end, sightPart will contain the closest part that was hit.
			sightPart = pi.key();
			hitMe = true;
		}
	return hitMe;
}

bool AuScParts::SightBuildZone(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart) const
{
	bool hitMe = false;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if((*pi)->SightBuildZone(eyeLoc, sightLoc))
		{
			// As we hit each face, sightLoc is shortened. By the end, sightPart will contain the closest part that was hit.
			sightPart = pi.key();
			hitMe = true;
		}
	return hitMe;
}

AuSysId AuScParts::SightedSystem(quint16 partId) const
{
	const_iterator pi = find(partId);
	if(pi == cend())
		return AuSysId();
	return (*pi)->SightedSystem(partId);
}

void AuScParts::GroundVehicleParkingSpots(QList<AuMatrix4x4>& spots) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->GroundVehicleParkingSpots())
			continue;
		spots.append((*pi)->PositionMatrix());
	}
}

void AuScParts::RocketParkingSpots(QList<AuMatrix4x4>& spots) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->RocketParkingSpots())
			continue;
		spots.append((*pi)->PositionMatrix());
	}
}

quint16 AuScParts::SpacecraftParkingSpots() const
{
	quint16 spots = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		spots += (*pi)->SpacecraftParkingSpots();
	}
	return spots;
}

void AuScParts::SpacecraftParkingSpots(QList<AuMatrix4x4>& spots) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->SpacecraftParkingSpots())
			continue;
		spots.append((*pi)->PositionMatrix());
	}
}

void AuScParts::SpaceVehicleParkingSpots(QList<AuMatrix4x4>& spots) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->SpaceVehicleParkingSpots())
			continue;
		spots.append((*pi)->PositionMatrix());
	}
}

void AuScParts::WaterVehicleParkingSpots(QList<AuMatrix4x4>& spots) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->WaterVehicleParkingSpots())
			continue;
		spots.append((*pi)->PositionMatrix());
	}
}

void AuScParts::GroundVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	const_iterator ri = find(roomId);
	if(ri == cend())
		return;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->GroundVehicleParkingSpots())
			continue;
		AuMatrix4x4 posM = (*pi)->PositionMatrix();
		AuPoint loc  = posM.Translation();
		if(!(*ri)->IsInsideRoomVoid(loc, *this))
			continue;
		spots.append(posM);
	}
}

void AuScParts::RocketParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	const_iterator ri = find(roomId);
	if(ri == cend())
		return;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->RocketParkingSpots())
			continue;
		AuMatrix4x4 posM = (*pi)->PositionMatrix();
		AuPoint loc  = posM.Translation();
		if(!(*ri)->IsInsideRoomVoid(loc, *this))
			continue;
		spots.append(posM);
	}
}

void AuScParts::SpaceVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	const_iterator ri = find(roomId);
	if(ri == cend())
		return;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->SpaceVehicleParkingSpots())
			continue;
		AuMatrix4x4 posM = (*pi)->PositionMatrix();
		AuPoint loc  = posM.Translation();
		if(!(*ri)->IsInsideRoomVoid(loc, *this))
			continue;
		spots.append(posM);
	}
}

void AuScParts::WaterVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	const_iterator ri = find(roomId);
	if(ri == cend())
		return;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(!(*pi)->WaterVehicleParkingSpots())
			continue;
		AuMatrix4x4 posM = (*pi)->PositionMatrix();
		AuPoint loc  = posM.Translation();
		if(!(*ri)->IsInsideRoomVoid(loc, *this))
			continue;
		spots.append(posM);
	}
}

AuPoint AuScParts::CenterOfSelection(const AuScSelection& sel) const
{
	if(sel.empty())
		return AuPoint(0,0);

	AuPoint accumCenter;	// The accumulated center of all selected parts, for calculating the average.
	int acens = 0;			// number of part center points added to 'center'.
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		const_iterator pi = find(si.key());
		if(pi == cend())
			continue;

		accumCenter += (*pi)->Center();
		acens++;
	}

	if(!acens)
		return AuPoint(0,0);

	accumCenter /= acens;
	return accumCenter;
}

bool AuScParts::SetTransporterPadLocs(const AuScSelection& sel, const AuPointArray& padLocs, AuScParts& partsChanged)
{
	bool anyChange = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		AuPointArray pads;
		(*pi)->PadLocs(pads);
		if(pads == padLocs)
			continue;

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetPadLocs(padLocs);
		anyChange = true;
	}
	return anyChange;
}

bool AuScParts::SetWeaponEmitLoc(const AuScSelection& sel, const AuPoint& emitLoc, AuScParts& partsChanged)
{
	bool anyChange = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		// Find the selected part.
		iterator pi = find(si.key());
		if(pi == end())
			continue;

		if((*pi)->WeaponEmitLoc() == emitLoc)
			continue;

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetEmitLoc(emitLoc);
		anyChange = true;
	}
	return anyChange;
}

bool AuScParts::IsOpenDoorVoid(quint16 roomId, quint16 doorId) const
{
	const_iterator pi = find(roomId);
	if(pi == cend())
		return false;
	if(!(*pi)->IsRoomVoid())
		return false;
	return (*pi)->AssociatedPartId() == doorId && (*pi)->AssociatedPartState() == AuScDoor::Open;
}

void AuScParts::LaunchPosition(quint16 launcherId, AuMatrix4x4& launchPadM, AuPoint& launchExit) const
{
	const_iterator pi = find(launcherId);
	if(pi == cend())
		return;
	if((*pi)->IsOmitted())
		return;
	(*pi)->LaunchPosition(launchPadM, launchExit);
}

void AuScParts::LandingSystemPosition(quint16 landerId, AuMatrix4x4& landerPosM) const
{
	const_iterator pi = find(landerId);
	if(pi == cend())
		return;
	if((*pi)->IsOmitted())
		return;
	(*pi)->LandingSystemPosition(landerPosM);
}

double AuScParts::HullEffectSize() const
{
	// The texture is cylindrically wrapped four times around my Y axis.
	AuPoint min,max;
	HullExtents(min, max);
	double wid = max.x - min.x;
	double ht = max.z - min.z;
	if(wid > ht)
		return wid * 2;
	else
		return ht * 2;
}

quint32 AuScParts::CountFaces() const
{
	quint32 faceCount = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		faceCount += (*pi)->CountFaces();
	}
	return faceCount;
}

quint32 AuScParts::CountTriangles() const
{
	quint32 tris = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		tris += (*pi)->CountTriangles();
	}
	return tris;
}

double AuScParts::FieldArea() const
{
	double area = 0;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		area += (*pi)->FieldArea();
	}
	return area;
}

quint16 AuScParts::InBuildZone(const AuPoint& loc) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->InBuildZone(loc))
			return pi.key();
	}
	return 0;
}

bool AuScParts::InBuildingSite(const AuPoint& loc) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->InBuildingSite(loc))
			return true;
	}
	return false;
}

bool AuScParts::InBuildingSite(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->InBuildingSite(loc, siteGround, hideGrass))
			return true;
	}
	return false;
}

void AuScParts::BuildingConstructionSegments(AuSegmentArray& edges) const
{
	AuSegmentArray usegs;	// unsorted edge segments
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		(*pi)->BuildingConstructionSegments(usegs);
	}

	QMultiMap<double, AuSegment> ssegs;	// Sort the edge segments by lowest Z.
	for(AuSegmentArray::const_iterator si = usegs.cbegin(); si != usegs.cend(); si++)
		if((*si).m_fLoc1.z < (*si).m_fLoc2.z)
			ssegs.insert((*si).m_fLoc1.z, *si);
		else
			ssegs.insert((*si).m_fLoc2.z, *si);
	for(QMultiMap<double, AuSegment>::const_iterator si = ssegs.cbegin(); si != ssegs.cend(); si++)
		edges.AddIfUnique(*si);
}

void AuScParts::BuildingCursorSegments(AuPointArray& edges) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		(*pi)->BuildingCursorSegments(edges);
	}
}

void AuScParts::BuildingCursorSiteSegments(AuPointArray& edges) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		(*pi)->BuildingCursorSiteSegments(edges);
	}
}

void AuScParts::BuildZoneCheckLocs(AuPointArray& locs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		(*pi)->BuildZoneCheckLocs(locs);
	}
}

float AuScParts::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		float siteI = (*pi)->BuildingSiteInfluence(loc, siteLoc, terBlend);
		if(siteI > 0)
			return siteI;
	}
	return 0;
}

float AuScParts::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		        bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree,
				quint8& terBlend, const AuZoneResource& res) const
{
	float siteI = 0;
	bool anyDecals = false;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->IsDecal())
		{
			anyDecals = true;
			continue;
		}
		// We cannot stop at the first part that influences the location because other parts can contribute shrubs and trees.
		if(siteI == 0)
		{
			// To match the results of the other BuildingSiteInfluence function, we will only preserve the result of the first site
			// that influences the location, in case of overlapping sites.
			siteI = (*pi)->BuildingSiteInfluence(loc, siteLoc, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType,
					farmShrub, orchardTree, terBlend, res);
		}
		else
		{
			// A site mesh already influenced the location.
			AuPoint pSiteLoc;
			(*pi)->BuildingSiteInfluence(loc, pSiteLoc, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType,
					farmShrub, orchardTree, terBlend, res);
		}
	}
	// Decals are done second so farm trees and shrubs can give the right results.
	if(anyDecals)
	{
		for(const_iterator pi = cbegin(); pi != cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if(!(*pi)->IsDecal())
				continue;
			// A site mesh may have already influenced the location. I don't care because decals do not influence the terrain.
			AuPoint pSiteLoc;
			(*pi)->BuildingSiteInfluence(loc, pSiteLoc, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType,
					farmShrub, orchardTree, terBlend, res);
		}
	}
	return siteI;
}

quint64 AuScParts::MemoryUsage() const
{
	quint64 mem = sizeof(AuScParts);
	mem += sizeof(m_PartId);
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		mem += sizeof(quint16) + (*pi)->MemoryUsage();
	return mem;
}

void AuScParts::PaintInUse(QList<QColor>& paintInUse) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		(*pi)->PaintInUse(paintInUse);
}

AuPoint AuScParts::RemoteCameraLocation(quint16 cameraId) const
{
	const_iterator pi = find(cameraId);
	if(pi == cend())
		return AuPoint();
	return (*pi)->Center();
}

quint16 AuScParts::RemoteCameraNext(quint16 currentCamera) const
{
	QMap<quint16, bool> cams;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if((*pi)->IsCamera())
			cams.insert(pi.key(), true);
	if(cams.empty())
		return 0;
	if(!currentCamera)
		return cams.cbegin().key();	// Start the sequence at the beginning.
	QMap<quint16, bool>::const_iterator ci = cams.find(currentCamera);
	if(ci == cams.cend())
		return cams.cbegin().key();	// currentCamera not found
	ci++;
	if(ci == cams.cend())
		return cams.cbegin().key();	// Wrap to beginning.
	return ci.key();
}

quint16 AuScParts::RemoteCameraPrevious(quint16 currentCamera) const
{
	QMap<quint16, bool> cams;
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
		if((*pi)->IsCamera())
			cams.insert(pi.key(), true);
	if(cams.empty())
		return 0;
	if(!currentCamera)
		return (cams.cend()-1).key();	// Start the sequence at the end.
	QMap<quint16, bool>::const_iterator ci = cams.find(currentCamera);
	if(ci == cams.cend())
		return (cams.cend()-1).key();	// currentCamera not found
	if(ci == cams.cbegin())
		return (cams.cend()-1).key();	// Wrap to beginning.
	ci--;
	return ci.key();
}

bool AuScParts::RemoteCameraVerify(quint16 cameraId) const
{
	const_iterator pi = find(cameraId);
	if(pi == cend())
		return false;
	return (*pi)->IsCamera();
}

void AuScParts::ExternalReferences(QSet<quint32>& xrefs) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if((*pi)->FenceStyle() != AuScFence::Custom)
			continue;
		if(!(*pi)->FenceDesignId())
			continue;
		xrefs.insert((*pi)->FenceDesignId());
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScMeshBase::AuScMeshBase()
{
}

AuScMeshBase::AuScMeshBase(const AuScMeshBase& c) : AuScPart(c), m_Faces(c.m_Faces)
{
}

AuScMeshBase::AuScMeshBase(const AuScMeshBase& c, const AuScSelectInfo& selInfo) : AuScPart(c), m_Faces(c.m_Faces, selInfo)
{
}

void AuScMeshBase::operator>>(QDataStream& out) const
{
	AuScPart::operator>>(out);
	out << m_Faces;
}

void AuScMeshBase::operator<<(QDataStream& in)
{
	AuScPart::operator<<(in);
	in >> m_Faces;
}

bool AuScMeshBase::operator==(const AuScPart& p) const
{
	try
	{
		const AuScMeshBase& c = dynamic_cast<const AuScMeshBase&>(p);
		if(!AuScPart::operator==(c))
			return false;

		if(m_Faces != c.m_Faces)
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuScMeshBase::EdgeIds(QSet<quint32>& edgeIds) const
{
	m_Faces.EdgeIds(edgeIds);
}

void AuScMeshBase::FaceIds(QSet<quint32>& faceIds) const
{
	m_Faces.FaceIds(faceIds);
}

void AuScMeshBase::VertexIds(QSet<quint32>& vertexIds) const
{
	m_Faces.VertexIds(vertexIds);
}

AuScPoint AuScMeshBase::VertexLoc(quint32 vertexIndex) const
{
	return m_Faces.VertexLocation(vertexIndex);
}

bool AuScMeshBase::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces);
}

bool AuScMeshBase::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.InViewRectangle(mvp, pixelRect, selInfo, findVertices, findEdges, findFaces);
}

bool AuScMeshBase::IsValid() const
{
	return !m_Faces.empty();
}

void AuScMeshBase::EdgeSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

void AuScMeshBase::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;
	m_Faces.FaceSpriteLocs(selFaces, selSize, selLocs, unselSize, unselLocs);
}

void AuScMeshBase::VertexSpriteLocs(const QSet<quint32>& selVerts,
		QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;
	m_Faces.VertexSpriteLocs(selVerts, selSize, selLocs, unselSize, unselLocs);
}

void AuScMeshBase::Transform(const AuMatrix4x4& t)
{
	m_Faces.Transform(t);
}

void AuScMeshBase::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	m_Faces.Transform(t, selInfo.m_Vertices, selInfo.m_Edges, selInfo.m_Faces);
}

void AuScMeshBase::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	m_Faces.TransformCopy(t, selInfo);
}

double AuScMeshBase::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.EdgeDistance(mvp, pixel, snapLoc);
}

double AuScMeshBase::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.EdgeDistance(mvp, pixel, edgeSeg);
}

double AuScMeshBase::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.FaceDistance(mvp, pixel, faceIndex, gripLoc);
}

double AuScMeshBase::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.VertexDistance(mvp, pixel, vertexIndex, snapLoc);
}

bool AuScMeshBase::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;
	return m_Faces.SelectContiguous(angleTol, selInfo);
}

bool AuScMeshBase::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectEdgesAt(edgeSeg, selInfo);
}

bool AuScMeshBase::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectFaceAt(atLoc, selInfo);
}

bool AuScMeshBase::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectFacesAt(atLoc, selInfo);
}

bool AuScMeshBase::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAt(atLoc, selInfo);
}

bool AuScMeshBase::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScMeshBase::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScMeshBase::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAtTextureCoord(texLoc, selInfo);
}

bool AuScMeshBase::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAtTextureCoord(texLoc, selInfo);
}

bool AuScMeshBase::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectEdgesAt(edgeSeg, selInfo);
}

bool AuScMeshBase::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectFacesAt(atLoc, selInfo);
}

bool AuScMeshBase::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAt(atLoc, selInfo);
}

QString AuScMeshBase::SelectedDesc(const AuScParts& parts) const
{
	return QObject::tr("%1, %L2 faces")
		.arg(FullName(parts))
		.arg(m_Faces.size());
}

void AuScMeshBase::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	m_Faces.RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScMeshBase::MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
		bool closeEnds)
{
	m_Faces.Clear();
	m_Faces.MakeExtrusion(bnd, extVec, gridPlane, steps, closeEnds);
}

void AuScMeshBase::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSpindle(profile, steps, gridPlane);
}

void AuScMeshBase::MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSphereLongLat(center, radius, lonSteps, latSteps, gridPlane);
}

void AuScMeshBase::MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSphereUniform(center, radius, equatorSteps, textureRepetitions, gridPlane);
}

void AuScMeshBase::MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides)
{
	m_Faces.Clear();
	m_Faces.MakeTube(path, diameter, sides, smoothSides);
}

void AuScMeshBase::MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides)
{
	m_Faces.Clear();
	m_Faces.MakeTube(path, shape, smoothSides);
}

void AuScMeshBase::Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
{
	m_Faces.Faces(faces, selInfo);
}

void AuScMeshBase::Faces(AuScFaceList& faces) const
{
	faces = m_Faces;
}

void AuScMeshBase::AddFaces(const AuScFaceList& faces)
{
	m_Faces.AddFaces(faces);
}

void AuScMeshBase::DeleteFaces(const QSet<quint32>& faceIds)
{
	m_Faces.DeleteFaces(faceIds);
}

void AuScMeshBase::ReverseVertexNormals()
{
	m_Faces.ReverseVertexNormals();
}

void AuScMeshBase::ReverseFaces()
{
	m_Faces.ReverseTriangleFacing();
	m_Faces.ReverseVertexNormals();
}

void AuScMeshBase::ReverseFaces(const AuScSelectInfo& selInfo)
{
	m_Faces.ReverseTriangleFacing(selInfo);
	m_Faces.ReverseVertexNormals(selInfo);
}

void AuScMeshBase::DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions)
{
	m_Faces.DivideFaces(selInfo, divisions);
}

void AuScMeshBase::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	m_Faces.PolishFaces(selInfo, shine, reflectivity);
}

void AuScMeshBase::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	m_Faces.EffectIntensityVertices(selInfo, intensity);
}

void AuScMeshBase::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	m_Faces.GlowVertices(selInfo, glow);
}

void AuScMeshBase::PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
{
	m_Faces.PaintFaces(selInfo, paint);
}

void AuScMeshBase::PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	m_Faces.PaintFaces(selInfo, theme, opacity);
}

void AuScMeshBase::PaintInUse(QList<QColor>& paintInUse) const
{
	m_Faces.PaintInUse(paintInUse);
}

void AuScMeshBase::FlattenVertices()
{
	m_Faces.FlattenVertexNormals();
}

void AuScMeshBase::FlattenVertices(const AuScSelectInfo& selInfo)
{
	m_Faces.FlattenVertexNormals(selInfo);
}

void AuScMeshBase::SmoothVertices()
{
	m_Faces.SmoothVertices();
}

void AuScMeshBase::SmoothVertices(const AuScSelectInfo& selInfo)
{
	m_Faces.SmoothVertices(selInfo);
}

bool AuScMeshBase::CutFaces(const AuPointArray& cutLine)
{
	return m_Faces.CutFaces(cutLine);
}

bool AuScMeshBase::PunchFaces(const AuPointArray& cutBnd)
{
	return m_Faces.PunchFaces(cutBnd);
}

bool AuScMeshBase::FractureFaces(const AuScSelectInfo& selInfo)
{
	return m_Faces.Fracture(selInfo);
}

bool AuScMeshBase::FuseFaces(const AuScSelectInfo& selInfo)
{
	return m_Faces.Fuse(selInfo);
}

void AuScMeshBase::SetFaceSelfIllumination(float selfIllum)
{
	m_Faces.SetSelfIllumination(selfIllum);
}

void AuScMeshBase::SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum)
{
	m_Faces.SetSelfIllumination(selInfo, selfIllum);
}

void AuScMeshBase::SetFaceOpacity(float opacity)
{
	m_Faces.SetOpacity(opacity);
}

void AuScMeshBase::SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	m_Faces.SetOpacity(selInfo, opacity);
}

void AuScMeshBase::EffectFaces(SURFACE_EFFECT effect)
{
	m_Faces.SetEffect(effect);
}

void AuScMeshBase::EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	m_Faces.SetEffect(selInfo, effect);
}

void AuScMeshBase::EffectFacesCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectCylindrical(selInfo, effect, texRepsAtEquator, texSize, gridPlane);
}

void AuScMeshBase::EffectFacesProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectProject(selInfo, effect, texSize, gridPlane);
}

void AuScMeshBase::EffectFacesSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectSpherical(selInfo, effect, texRepsAtEquator, gridPlane);
}

void AuScMeshBase::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectUnwrap(selInfo, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

void AuScMeshBase::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	m_Faces.EffectMove(selInfo, vec);
}

void AuScMeshBase::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	m_Faces.EffectRotate(selInfo, pivot, angle);
}

void AuScMeshBase::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	m_Faces.EffectStretch(selInfo, base, scaleX, scaleY);
}

SURFACE_EFFECT AuScMeshBase::Effect(const AuScSelectInfo& selInfo) const
{
	qint8 texI = m_Faces.Effect(selInfo);
	if(texI == -2)
		texI = EFF_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = EFF_Count;	// Face texture indexes differ.
	return (SURFACE_EFFECT)texI;
}

void AuScMeshBase::TextureFaces(SC_TEXTURE_INDEX texIndex)
{
	m_Faces.SetTextureIndex(texIndex);
}

void AuScMeshBase::TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex)
{
	m_Faces.SetTextureIndex(selInfo, texIndex);
}

void AuScMeshBase::TextureFacesCylindrical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureCylindrical(selInfo, texIndex, texRepsAtEquator, texSize, gridPlane);
}

void AuScMeshBase::TextureFacesProject(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureProject(selInfo, texIndex, texSize, gridPlane);
}

void AuScMeshBase::TextureFacesSpherical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureSpherical(selInfo, texIndex, texRepsAtEquator, gridPlane);
}

void AuScMeshBase::TextureUnwrap(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureUnwrap(selInfo, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

void AuScMeshBase::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	m_Faces.TextureMove(selInfo, vec);
}

void AuScMeshBase::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	m_Faces.TextureRotate(selInfo, pivot, angle);
}

void AuScMeshBase::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	m_Faces.TextureStretch(selInfo, base, scaleX, scaleY);
}

SC_TEXTURE_INDEX AuScMeshBase::TextureIndex(const AuScSelectInfo& selInfo) const
{
	qint8 texI = m_Faces.TextureIndex(selInfo);
	if(texI == -2)
		texI = SCTEXI_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = SCTEXI_Count;	// Face texture indexes differ.
	return (SC_TEXTURE_INDEX)texI;
}

bool AuScMeshBase::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	return m_Faces.Sight(eyeLoc, sightLoc, sightSurfaceNormal);
}

AuSysId AuScMeshBase::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Hull;
}

AuPoint AuScMeshBase::Center() const
{
	AuPoint min,max;
	m_Faces.Extents(min,max);
	return (min + max)/2;
}

void AuScMeshBase::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

void AuScMeshBase::EffectHullFTL(double texSize, bool shipHull)
{
	m_Faces.ApplyHullFTLEffect(texSize, shipHull);
}

bool AuScMeshBase::ModelExtents(AuPoint& min, AuPoint& max) const
{
	if(m_Faces.empty())
		return false;
	m_Faces.Extents(min, max);
	return true;
}

quint64 AuScMeshBase::MemoryUsage() const
{
	quint64 mem = AuScPart::MemoryUsage();
	mem += m_Faces.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScMesh::AuScMesh()
{
}

AuScMesh::AuScMesh(const AuScMesh& c) : AuScMeshBase(c)
{
}

AuScMesh::AuScMesh(const AuScMesh& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScMesh::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScMesh::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
	if(ReadVersion() >= 0x0c && ReadVersion() < 0x10)
	{
		quint16 partId;
		in >> partId;
		// This is hoky but it will work. We will set the group name using the part id. Then AuScParts can fix me up afterwards.
		QString oldName = QString("AuScMesh:%1").arg(partId);
		SetGroupName(oldName);
	}
}

void AuScMesh::PostReadCleanup(AuScParts& parts)
{
	if(ReadVersion() < 0x10)
	{
		// I am old enough to have used part association to be grouped with a room, for lighting.
		QStringList gn = GroupName().split(":");
		if(gn.size() > 1 && gn[0] == QString("AuScMesh"))
		{
			// I found the appropriate tag.
			quint16 roomId = gn[1].toUInt();
			if(roomId)
			{
				AuScParts::const_iterator ri = parts.find(roomId);
				if(ri != parts.cend())
					SetGroupName((*ri)->GroupName());	// My group name now matches the group name of my associated room.
				else
					SetGroupName("");					// Nonexistent associated part; clear my group name.
			}
			else
				SetGroupName("");						// Bogus room id; clear my group name.
		}
	}
}

bool AuScMesh::operator==(const AuScPart& p) const
{
	try
	{
		const AuScMesh& c = dynamic_cast<const AuScMesh&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

bool AuScMesh::IsValid() const
{
	return AuScMeshBase::IsValid();
}

AuScPart* AuScMesh::Clone() const
{
	return new AuScMesh(*this);
}

AuScPart* AuScMesh::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScMesh(*this, selInfo);
}

QString AuScMesh::TypeIcon() const
{
	return QString(":data/ScMesh.png");
}

QString AuScMesh::TypeName() const
{
	return QObject::tr("Mesh");
}

bool AuScMesh::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	if(IsOmitted())
		return false;
	if(GroupName().isEmpty() || GroupName() == d_GroupHull || GroupName() == d_GroupPost || GroupName() == d_GroupSection)
		EffectHullFTL(parts.HullEffectSize(), shipHull);	// Apply the hull FTL effect because This mesh is lit like the hull.
	return true;
}

bool AuScMesh::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

quint64 AuScMesh::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScDoorJig::AuScDoorJig()
{
}

AuScDoorJig::AuScDoorJig(const AuScDoorJig& c) : AuScMeshBase(c)
{
}

AuScDoorJig::AuScDoorJig(const AuScDoorJig& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScDoorJig::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScDoorJig::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScDoorJig::operator==(const AuScPart& p) const
{
	try
	{
		const AuScDoorJig& c = dynamic_cast<const AuScDoorJig&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScDoorJig::Clone() const
{
	return new AuScDoorJig(*this);
}

AuScPart* AuScDoorJig::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScDoorJig(*this, selInfo);
}

QString AuScDoorJig::TypeIcon() const
{
	return QString(":data/ScJig.png");
}

QString AuScDoorJig::TypeName() const
{
	return QObject::tr("Jig, Door");
}

void AuScDoorJig::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScWindowJig::AuScWindowJig()
{
}

AuScWindowJig::AuScWindowJig(const AuScWindowJig& c) : AuScMeshBase(c)
{
}

AuScWindowJig::AuScWindowJig(const AuScWindowJig& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScWindowJig::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScWindowJig::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScWindowJig::operator==(const AuScPart& p) const
{
	try
	{
		const AuScWindowJig& c = dynamic_cast<const AuScWindowJig&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScWindowJig::Clone() const
{
	return new AuScWindowJig(*this);
}

AuScPart* AuScWindowJig::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScWindowJig(*this, selInfo);
}

QString AuScWindowJig::TypeIcon() const
{
	return QString(":data/ScJig.png");
}

QString AuScWindowJig::TypeName() const
{
	return QObject::tr("Jig, Window");
}

void AuScWindowJig::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScHull::AuScHull()
{
}

AuScHull::AuScHull(const AuScHull& c) : AuScMeshBase(c), m_Silhouette(c.m_Silhouette)
{
}

AuScHull::AuScHull(const AuScHull& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo), m_Silhouette(c.m_Silhouette)
{
}

void AuScHull::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);

	out << m_Silhouette;
}

void AuScHull::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);

	if(ReadVersion() >= 0x0d)
		in >> m_Silhouette;
	else if(ReadVersion() >= 0x02)
		m_Silhouette.ReadOld(in);
}

bool AuScHull::operator==(const AuScPart& p) const
{
	try
	{
		const AuScHull& c = dynamic_cast<const AuScHull&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		//if(m_Silhouette != c.m_Silhouette)
			//return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScHull::Clone() const
{
	return new AuScHull(*this);
}

AuScPart* AuScHull::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScHull(*this, selInfo);
}

QString AuScHull::TypeIcon() const
{
	return QString(":data/ScHull.png");
}

QString AuScHull::TypeName() const
{
	return QObject::tr("Hull");
}

void AuScHull::RefreshObstructionsCache(quint16 partId, const AuScParts& parts)
{
	AuScFaceList faces(m_Faces);

	// Build an initial silhouette. We will use it to find other parts that are inside it.
	m_Silhouette.Build(faces, (float)d_ObstructMargin);

	// Get faces from windows and rooms that have a vertex inside my obstruction model.
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(pi.key() == partId)
			continue;

		// We want doors, windows, rooms and room voids that appear to be a part of this hull.
		bool addToHull = (*pi)->IsDoor() || (*pi)->IsWindow() || (*pi)->IsRoomMesh() || (*pi)->IsRoomVoid();
		if(!addToHull)
			continue;

		// Check the part to see if it is contiguous with the hull we have accumulated so far.
		// Get vertices of the part.
		QSet<quint32> vertexIds;
		(*pi)->VertexIds(vertexIds);

		// Find at least one vertex of the part that is in the obstruction model of this hull.
		bool inHull = false;
		for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend(); vi++)
		{
			AuScPoint p = (*pi)->VertexLoc(*vi);
			if(IsInsideObstructionModel(p))
			{
				// A vertex of the part is inside the obstruction model of this hull.
				inHull = true;
				break;
			}
		}
		if(!inHull)
			continue;	// The part is not in the obstruction model of this hull part.

		// Collect faces from parts that are inside this hull.
		(*pi)->FacesInHullSilhouette(faces);
	}

	// Rebuild the final silhouette for this hull part, using all the faces of this hull and parts that are inside it.
	m_Silhouette.Build(faces, (float)d_ObstructMargin);
}

bool AuScHull::RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts)
{
	if(m_Silhouette.IsValid())
		return false;
	RefreshObstructionsCache(partId, parts);
	return true;
}

bool AuScHull::IsInsideObstructionModel(const AuPoint& loc) const
{
	return m_Silhouette.IsInside(loc);
}

bool AuScHull::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	if(!m_Silhouette.IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
		return false;
	/*
	// Allow movement into the silhouette if there is a floor below.
	AuPoint sLoc(toLoc.x, toLoc.y, toLoc.z - 10000);
	AuPoint sNorm;
	if(!m_Faces.Sight(toLoc, sLoc, sNorm))
		return true;	// No floor ahead.
	if(sNorm.z < 0)
		return true;	// Floor cannot face downward.
		*/
	return true;
}

void AuScHull::EdgeSegments(AuPointArray& edges) const
{
	AuScMeshBase::EdgeSegments(edges);
}

quint32 AuScHull::HullVolume(quint16 partId, const AuScParts& parts) const
{
	AuScFaceList faces(m_Faces);

	// Build an initial silhouette. We will use it to find other parts that are inside it.
	AuScSilhouette3d silhouette;
	silhouette.Build(faces, (float)d_ObstructMargin);

	// Get faces from windows and rooms that have a vertex inside my obstruction model.
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(pi.key() == partId)
			continue;

		// We want doors, windows, rooms and room voids that appear to be a part of this hull.
		bool addToHull = (*pi)->IsDoor() || (*pi)->IsWindow() || (*pi)->IsRoomMesh() || (*pi)->IsRoomVoid();
		if(!addToHull)
			continue;

		// Check the part to see if it is contiguous with the hull we have accumulated so far.
		// Get vertices of the part.
		QSet<quint32> vertexIds;
		(*pi)->VertexIds(vertexIds);

		// Find at least one vertex of the part that is in the obstruction model of this hull.
		bool inHull = false;
		for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend(); vi++)
		{
			AuScPoint p = (*pi)->VertexLoc(*vi);
			if(IsInsideObstructionModel(p))
			{
				// A vertex of the part is inside the obstruction model of this hull.
				inHull = true;
				break;
			}
		}
		if(!inHull)
			continue;	// The part is not in the obstruction model of this hull part.

		// Collect faces from parts that are inside this hull.
		(*pi)->FacesInHullSilhouette(faces);
	}

	// Rebuild the final silhouette for this hull part, using all the faces of this hull and parts that are inside it.
	silhouette.Build(faces, 0);					// Make a silhouette with no margin, for volume calc.
	return (quint32)ceilf(silhouette.Volume() * CubicMetersPerCubicFoot());
}

quint32 AuScHull::HullVolumeVoxels(quint16 partId, const AuScParts& parts, QMap<AuPoint, bool>& voxels, double voxelSize) const
{
	AuScFaceList faces(m_Faces);

	// Build an initial silhouette. We will use it to find other parts that are inside it.
	AuScSilhouette3d silhouette;
	silhouette.Build(faces, (float)d_ObstructMargin);

	// Get faces from windows and rooms that have a vertex inside my obstruction model.
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
	{
		if((*pi)->IsOmitted())
			continue;
		if(pi.key() == partId)
			continue;

		// We want doors, windows, rooms and room voids that appear to be a part of this hull.
		bool addToHull = (*pi)->IsDoor() || (*pi)->IsWindow() || (*pi)->IsRoomMesh() || (*pi)->IsRoomVoid();
		if(!addToHull)
			continue;

		// Check the part to see if it is contiguous with the hull we have accumulated so far.
		// Get vertices of the part.
		QSet<quint32> vertexIds;
		(*pi)->VertexIds(vertexIds);

		// Find at least one vertex of the part that is in the obstruction model of this hull.
		bool inHull = false;
		for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend(); vi++)
		{
			AuScPoint p = (*pi)->VertexLoc(*vi);
			if(IsInsideObstructionModel(p))
			{
				// A vertex of the part is inside the obstruction model of this hull.
				inHull = true;
				break;
			}
		}
		if(!inHull)
			continue;	// The part is not in the obstruction model of this hull part.

		// Collect faces from parts that are inside this hull.
		(*pi)->FacesInHullSilhouette(faces);
	}
	if(faces.empty())
		return 0;

	// Rebuild the final silhouette for this hull part, using all the faces of this hull and parts that are inside it.
	silhouette.Build(faces, 0);					// Make a silhouette with no margin, for volume calc.
	quint32 silVol = (quint32)ceilf(silhouette.Volume() * CubicMetersPerCubicFoot());

	// Make voxels using the silhouette that has no margin.
	AuPoint mymin, mymax;
	m_Faces.Extents(mymin, mymax);	// My face extents only, not including those other faces.
	mymin.Round(voxelSize);
	mymax.Round(voxelSize);
	if(mymin == mymax)
	{
		voxels.insert(mymin, true);
		return silVol;
	}

	int xvox;				// Number of voxels to sample in X.
	int yvox;				// Number of voxels to sample in Y.
	int zvox;				// Number of voxels to sample in Z.
	if(voxelSize == 1)
	{
		xvox = (int)round(mymax.x - mymin.x) + 1;
		yvox = (int)round(mymax.y - mymin.y) + 1;
		zvox = (int)round(mymax.z - mymin.z) + 1;
	}
	else
	{
		xvox = (int)round((mymax.x - mymin.x)/voxelSize) + 1;
		yvox = (int)round((mymax.y - mymin.y)/voxelSize) + 1;
		zvox = (int)round((mymax.z - mymin.z)/voxelSize) + 1;
	}
	for(int ix=0; ix<xvox; ix++)
	{
		for(int iy=0; iy<yvox; iy++)
		{
			for(int iz=0; iz<zvox; iz++)
			{
				AuPoint vox = mymin + AuPoint(ix, iy, iz) * voxelSize;
				if(voxels.contains(vox))
					continue;	// Skip the hard work if we already have the voxel.
				if(silhouette.IsInside(vox))
					voxels.insert(vox, true);
			}
		}
	}
	return silVol;
}

bool AuScHull::HullExtents(AuPoint& min, AuPoint& max) const
{
	if(m_Faces.empty())
		return false;

	m_Faces.Extents(min, max);
	return true;
}

bool AuScHull::HullExtents(quint16 partId, AuPoint& min, AuPoint& max, const AuLandingGearStateHash& landingGearStates) const
{
	Q_UNUSED(partId);
	Q_UNUSED(landingGearStates);
	return HullExtents(min, max);
}

void AuScHull::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(armor, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int metal = (int)surface / d_HullPlatesPerUnit;
	if(metal < 1)
		metal = 1;	// Minimum 1 metal per room.
	pc.AddRequirement(hullMaterial, CE_Required, metal);
}

bool AuScHull::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	if(IsOmitted())
		return false;
	EffectHullFTL(parts.HullEffectSize(), shipHull);
	return true;
}

void AuScHull::BuildingConstructionSegments(AuSegmentArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

void AuScHull::BuildingCursorSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

quint64 AuScHull::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	mem += m_Silhouette.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScRoom::AuScRoom()
{
	m_InitLight = INTLIGHT_Bright;
	m_IsSunlit = true;
}

AuScRoom::AuScRoom(const AuScRoom& c) : AuScMeshBase(c)
{
	m_InitLight = c.m_InitLight;
	m_IsSunlit = c.m_IsSunlit;
}

AuScRoom::AuScRoom(const AuScRoom& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
	m_InitLight = c.m_InitLight;
	m_IsSunlit = c.m_IsSunlit;
}

void AuScRoom::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
	quint8 l = m_InitLight;
	if(m_IsSunlit)
		l |= 0x80;
	out << l;
}

void AuScRoom::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
	if(ReadVersion() < 0x10)
	{
		QString oldName;
		in >> oldName;
		SetGroupName(oldName);
	}

	if(ReadVersion() >= 0x0c)
	{
		quint8 l;
		in >> l;
		m_InitLight = l & 0x0f;
		m_IsSunlit = l & 0x80;
	}
}

bool AuScRoom::operator==(const AuScPart& p) const
{
	try
	{
		const AuScRoom& c = dynamic_cast<const AuScRoom&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		if(m_InitLight != c.m_InitLight)
			return false;
		if(m_IsSunlit != c.m_IsSunlit)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScRoom::Clone() const
{
	return new AuScRoom(*this);
}

AuScPart* AuScRoom::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScRoom(*this, selInfo);
}

QString AuScRoom::TypeIcon() const
{
	return QString(":data/ScRoom.png");
}

QString AuScRoom::TypeName() const
{
	return QObject::tr("Room");
}

void AuScRoom::MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps)
{
	AuScMeshBase::MakeExtrusion(bnd, extVec, gridPlane, steps, true);
	ReverseFaces();
	ReverseVertexNormals();
}

void AuScRoom::MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps,
		const AuMatrix4x4& gridPlane)
{
	AuScMeshBase::MakeSphereLongLat(center, radius, lonSteps, latSteps, gridPlane);
	ReverseFaces();
	ReverseVertexNormals();
}

void AuScRoom::MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
		const AuMatrix4x4& gridPlane)
{
	AuScMeshBase::MakeSphereUniform(center, radius, equatorSteps, textureRepetitions, gridPlane);
	ReverseFaces();
	ReverseVertexNormals();
}

void AuScRoom::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane)
{
	AuScMeshBase::MakeSpindle(profile, steps, gridPlane);
	ReverseFaces();
	ReverseVertexNormals();
}

void AuScRoom::MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides)
{
	AuScMeshBase::MakeTube(path, diameter, sides, smoothSides);
	ReverseFaces();
	ReverseVertexNormals();
}

void AuScRoom::MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides)
{
	AuScMeshBase::MakeTube(path, shape, smoothSides);
	ReverseFaces();
	ReverseVertexNormals();
}

void AuScRoom::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, SLINE_None);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int metal = (int)surface / d_HullPlatesPerUnit;
	if(metal < 1)
		metal = 1;	// Minimum 1 metal per room.
	pc.AddRequirement(hullMaterial, CE_Required, metal);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		// Add plastic for seals and trim.
		int plastic = metal / 20;
		if(!plastic)
			plastic = 1;
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	}
}

void AuScRoom::FacesInHullSilhouette(AuScFaceList& faces) const
{
	faces.AddFaces(m_Faces);
}

QString AuScRoom::SelectedDesc(const AuScParts& parts) const
{
	QString il;
	switch(m_InitLight)
	{
	case INTLIGHT_Bright:
		il = QObject::tr("On");
		break;
	case INTLIGHT_Dim:
		il = QObject::tr("Dim");
		break;
	default:
		il = QObject::tr("Off");
		break;
	}

	QString sun = m_IsSunlit ? QObject::tr("Y") : QObject::tr("N");

	return QObject::tr("%1, %L2 faces, Light=%3, Sun=%4")
		.arg(FullName(parts))
		.arg(m_Faces.size())
		.arg(il)
		.arg(sun);
}

quint64 AuScRoom::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	mem += sizeof(m_InitLight);
	mem += sizeof(m_IsSunlit);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStatePart::AuScStatePart()
{
	m_State = 0;
}

AuScStatePart::AuScStatePart(const AuScStatePart& c) : AuScPart(c), m_StateMeshes(c.m_StateMeshes)
{
	m_State = c.m_State;
}

AuScStatePart::AuScStatePart(const AuScStatePart& c, const AuScSelectInfo& selInfo) : AuScPart(c)
{
	m_State = c.m_State;

	/* Copy all meshes in full except the current state mesh, which gets partially copied based on selection.
	for(QHash<quint8, AuScFaceList>::const_iterator smi = c.m_StateMeshes.cbegin(); smi != c.m_StateMeshes.cend(); smi++)
	{
		if(smi.key() == m_State)
			m_StateMeshes.insert(smi.key(), AuScFaceList(*smi, selInfo));
		else
			m_StateMeshes.insert(smi.key(), *smi);
	}
	*/

	// Copy only the selected state mesh.
	QHash<quint8, AuScFaceList>::const_iterator smi = c.m_StateMeshes.find(m_State);
	if(smi != c.m_StateMeshes.cend())
	{
		AuScFaceList faces(*smi, selInfo);
		m_StateMeshes.insert(smi.key(), faces);
	}
}

void AuScStatePart::operator>>(QDataStream& out) const
{
	AuScPart::operator>>(out);

	out << m_State;
	out << (quint8)m_StateMeshes.size();
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
	{
		out << smi.key();
		out << *smi;
	}
}

void AuScStatePart::operator<<(QDataStream& in)
{
	AuScPart::operator<<(in);

	in >> m_State;

	m_StateMeshes.clear();
	quint8 c;
	in >> c;
	while(c--)
	{
		quint8 state;
		AuScFaceList mesh;

		in >> state;
		in >> mesh;

		m_StateMeshes.insert(state, mesh);
	}
}

bool AuScStatePart::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStatePart& c = dynamic_cast<const AuScStatePart&>(p);
		if(!AuScPart::operator==(c))
			return false;

		if(m_State != c.m_State)
			return false;
		if(m_StateMeshes != c.m_StateMeshes)
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuScStatePart::AddFaces(const AuScFaceList& faces)
{
	if(m_StateMeshes.empty())
	{
		// State mesh is completely empty. We probably just got constructed.
		// Create only the 0 state.
		m_StateMeshes.insert(0, faces);
	}
	else
	{
		// Update current state mesh.
		QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
		if(smi == m_StateMeshes.end())
			m_StateMeshes.insert(m_State, faces);
		else
			(*smi).AddFaces(faces);
	}
}

void AuScStatePart::DeleteFaces(const QSet<quint32>& faceIds)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).DeleteFaces(faceIds);
}

bool AuScStatePart::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).DeselectEdgesAt(edgeSeg, selInfo);
}

bool AuScStatePart::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).DeselectFacesAt(atLoc, selInfo);
}

bool AuScStatePart::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).DeselectVerticesAt(atLoc, selInfo);
}

double AuScStatePart::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).EdgeDistance(mvp, pixel, snapLoc);
}

double AuScStatePart::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).EdgeDistance(mvp, pixel, edgeSeg);
}

void AuScStatePart::EdgeIds(QSet<quint32>& edgeIds) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).EdgeIds(edgeIds);
}

void AuScStatePart::EdgeSegments(AuPointArray& edges) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).EdgeSegments(edges);
}

double AuScStatePart::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return d_SearchTolerance;
	return (*smi).FaceDistance(mvp, pixel, faceIndex, gripLoc);
}

void AuScStatePart::FaceIds(QSet<quint32>& faceIds) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).FaceIds(faceIds);
}

void AuScStatePart::Faces(AuScFaceList& faces) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;
	faces = *smi;
}

void AuScStatePart::Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).Faces(faces, selInfo);
}

void AuScStatePart::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs,
		QList<AuPointAlpha>& unselLocs) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;

	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;

	(*smi).FaceSpriteLocs(selFaces, selSize, selLocs, unselSize, unselLocs);
}

void AuScStatePart::FlattenVertices()
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).FlattenVertexNormals();
}

void AuScStatePart::FlattenVertices(const AuScSelectInfo& selInfo)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).FlattenVertexNormals(selInfo);
}

bool AuScStatePart::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces);
}

bool AuScStatePart::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).InViewRectangle(mvp, pixelRect, selInfo, findVertices, findEdges, findFaces);
}

bool AuScStatePart::IsValid() const
{
	if(m_StateMeshes.empty())
		return false;	// No state meshes.

	// All state meshes may not be empty.
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
		if(!(*smi).empty())
			return true;	// A non-empty state mesh was found.
	return false;
}

void AuScStatePart::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScStatePart::ReverseFaces()
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).ReverseTriangleFacing();
	(*smi).ReverseVertexNormals();
}

void AuScStatePart::ReverseFaces(const AuScSelectInfo& selInfo)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).ReverseTriangleFacing(selInfo);
	(*smi).ReverseVertexNormals(selInfo);
}

void AuScStatePart::DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).DivideFaces(selInfo, divisions);
}

void AuScStatePart::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).PolishFaces(selInfo, shine, reflectivity);
}

void AuScStatePart::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectIntensityVertices(selInfo, intensity);
}

void AuScStatePart::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).GlowVertices(selInfo, glow);
}

void AuScStatePart::PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).PaintFaces(selInfo, paint);
}

void AuScStatePart::PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).PaintFaces(selInfo, theme, opacity);
}

void AuScStatePart::PaintInUse(QList<QColor>& paintInUse) const
{
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
		(*smi).PaintInUse(paintInUse);
}

void AuScStatePart::ReverseVertexNormals()
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).ReverseVertexNormals();
}

QString AuScStatePart::SelectedDesc(const AuScParts& parts) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi != m_StateMeshes.cend())
		return QObject::tr("%1 %2, %L3 faces")
			.arg(FullName(parts))
			.arg(StateName())
			.arg((*smi).size());
	else
		return QObject::tr("%1 %2, No faces")
			.arg(FullName(parts))
			.arg(StateName());
}

bool AuScStatePart::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).SelectContiguous(angleTol, selInfo);
}

bool AuScStatePart::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).SelectEdgesAt(edgeSeg, selInfo);
}

bool AuScStatePart::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).SelectFaceAt(atLoc, selInfo);
}

bool AuScStatePart::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).SelectFacesAt(atLoc, selInfo);
}

bool AuScStatePart::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).SelectVerticesAt(atLoc, selInfo);
}

bool AuScStatePart::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).DeselectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScStatePart::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).SelectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScStatePart::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).DeselectVerticesAtTextureCoord(texLoc, selInfo);
}

bool AuScStatePart::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return false;
	return (*smi).SelectVerticesAtTextureCoord(texLoc, selInfo);
}

void AuScStatePart::SetFaces(const AuScFaceList& faces)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		m_StateMeshes.insert(m_State, faces);
	else
		*smi = faces;
}

void AuScStatePart::SmoothVertices()
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SmoothVertices();
}

void AuScStatePart::SmoothVertices(const AuScSelectInfo& selInfo)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SmoothVertices(selInfo);
}

void AuScStatePart::Transform(const AuMatrix4x4& t)
{
	for(QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.begin(); smi != m_StateMeshes.end(); smi++)
		(*smi).Transform(t);
}

void AuScStatePart::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).Transform(t, selInfo.m_Vertices, selInfo.m_Edges, selInfo.m_Faces);
}

void AuScStatePart::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TransformCopy(t, selInfo);
}

double AuScStatePart::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return d_SearchTolerance;
	return (*smi).VertexDistance(mvp, pixel, vertexIndex, snapLoc);
}

void AuScStatePart::VertexIds(QSet<quint32>& vertexIds) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).VertexIds(vertexIds);
}

AuScPoint AuScStatePart::VertexLoc(quint32 vertexIndex) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return AuScPoint();
	return (*smi).VertexLocation(vertexIndex);
}

void AuScStatePart::VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
		QList<AuPointAlpha>& unselLocs) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return;

	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;

	(*smi).VertexSpriteLocs(selVerts, selSize, selLocs, unselSize, unselLocs);
}

// 'extVec' is relative to the gridPlane.
void AuScStatePart::MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
		bool closeEnds)
{
	AuScFaceList faces;
	faces.MakeExtrusion(bnd, extVec, gridPlane, steps, closeEnds);
	SetFaces(faces);
}

// Makes a lat/long type sphere.
void AuScStatePart::MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps,
		const AuMatrix4x4& gridPlane)
{
	AuScFaceList faces;
	faces.MakeSphereLongLat(center, radius, lonSteps, latSteps, gridPlane);
	SetFaces(faces);
}

// Makes an evenly tessellated sphere.
void AuScStatePart::MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
		const AuMatrix4x4& gridPlane)
{

	AuScFaceList faces;
	faces.MakeSphereUniform(center, radius, equatorSteps, textureRepetitions, gridPlane);
	SetFaces(faces);
}

void AuScStatePart::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane)
{
	AuScFaceList faces;
	faces.MakeSpindle(profile, steps, gridPlane);
	SetFaces(faces);
}

void AuScStatePart::MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides)
{
	AuScFaceList faces;
	faces.MakeTube(path, diameter, sides, smoothSides);
	SetFaces(faces);
}

// 'shape' is expected to be drawn in 2D. The origin is the guide along which 'shape' follows 'path'.
void AuScStatePart::MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides)
{
	AuScFaceList faces;
	faces.MakeTube(path, shape, smoothSides);
	SetFaces(faces);
}

bool AuScStatePart::CutFaces(const AuPointArray& cutLine)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return false;
	return (*smi).CutFaces(cutLine);
}

bool AuScStatePart::PunchFaces(const AuPointArray& cutBnd)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return false;
	return (*smi).PunchFaces(cutBnd);
}

bool AuScStatePart::FractureFaces(const AuScSelectInfo& selInfo)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return false;
	return (*smi).Fracture(selInfo);
}

bool AuScStatePart::FuseFaces(const AuScSelectInfo& selInfo)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return false;
	return (*smi).Fuse(selInfo);
}

void AuScStatePart::SetFaceSelfIllumination(float selfIllum)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetSelfIllumination(selfIllum);
}

void AuScStatePart::SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetSelfIllumination(selInfo, selfIllum);
}

void AuScStatePart::SetFaceOpacity(float opacity)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetOpacity(opacity);
}

void AuScStatePart::SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetOpacity(selInfo, opacity);
}

void AuScStatePart::EffectFaces(SURFACE_EFFECT effect)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetEffect(effect);
}

void AuScStatePart::EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetEffect(selInfo, effect);
}

void AuScStatePart::EffectFacesCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectCylindrical(selInfo, effect, texRepsAtEquator, texSize, gridPlane);
}

void AuScStatePart::EffectFacesProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectProject(selInfo, effect, texSize, gridPlane);
}

void AuScStatePart::EffectFacesSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectSpherical(selInfo, effect, texRepsAtEquator, gridPlane);
}

void AuScStatePart::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectUnwrap(selInfo, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

void AuScStatePart::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectMove(selInfo, vec);
}

void AuScStatePart::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectRotate(selInfo, pivot, angle);
}

void AuScStatePart::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).EffectStretch(selInfo, base, scaleX, scaleY);
}

SURFACE_EFFECT AuScStatePart::Effect(const AuScSelectInfo& selInfo) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return (SURFACE_EFFECT)(EFF_Count+1);	// No faces in the selection.

	qint8 texI = (*smi).Effect(selInfo);
	if(texI == -2)
		texI = EFF_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = EFF_Count;	// Face texture indexes differ.
	return (SURFACE_EFFECT)texI;
}

void AuScStatePart::TextureFaces(SC_TEXTURE_INDEX texIndex)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetTextureIndex(texIndex);
}

void AuScStatePart::TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).SetTextureIndex(selInfo, texIndex);
}

void AuScStatePart::TextureFacesCylindrical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TextureCylindrical(selInfo, texIndex, texRepsAtEquator, texSize, gridPlane);
}

void AuScStatePart::TextureFacesProject(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TextureProject(selInfo, texIndex, texSize, gridPlane);
}

void AuScStatePart::TextureFacesSpherical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TextureSpherical(selInfo, texIndex, texRepsAtEquator, gridPlane);
}

void AuScStatePart::TextureUnwrap(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TextureUnwrap(selInfo, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

void AuScStatePart::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TextureMove(selInfo, vec);
}

void AuScStatePart::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TextureRotate(selInfo, pivot, angle);
}

void AuScStatePart::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.end())
		return;
	(*smi).TextureStretch(selInfo, base, scaleX, scaleY);
}

SC_TEXTURE_INDEX AuScStatePart::TextureIndex(const AuScSelectInfo& selInfo) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi == m_StateMeshes.cend())
		return (SC_TEXTURE_INDEX)(SCTEXI_Count+1);	// No faces in the selection.

	qint8 texI = (*smi).TextureIndex(selInfo);
	if(texI == -2)
		texI = SCTEXI_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = SCTEXI_Count;	// Face texture indexes differ.
	return (SC_TEXTURE_INDEX)texI;
}

AuPoint AuScStatePart::Center() const
{
	AuPoint accumCenter;
	int acens = 0;
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
	{
		if((*smi).empty())
			continue;
		AuPoint min,max;
		(*smi).Extents(min,max);
		accumCenter += (min + max)/2;
		acens++;
	}

	if(!acens)
		return AuPoint(0,0);

	accumCenter /= acens;
	return accumCenter;
}

void AuScStatePart::EffectHullFTL(double texSize, bool shipHull)
{
	for(QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.begin(); smi != m_StateMeshes.end(); smi++)
		(*smi).ApplyHullFTLEffect(texSize, shipHull);
}

quint32 AuScStatePart::CountFaces() const
{
	quint32 fc = 0;
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
		fc += (*smi).size();
	return fc;
}

quint32 AuScStatePart::CountTriangles() const
{
	QColor themeColors[AuScFace::THEME_Count] =
	{
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
	};
	quint32 fc = 0;
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
	{
		AuScTriRenderList opaqueTris;
		AuScTriRenderList transparentTris;
		(*smi).RenderTriangles(opaqueTris, transparentTris, themeColors);
		fc += opaqueTris.size() + transparentTris.size();
	}
	return fc;
}

bool AuScStatePart::ModelExtents(AuPoint& min, AuPoint& max) const
{
	bool firstOne = true;
	AuPoint pmin, pmax;
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
	{
		(*smi).Extents(pmin, pmax);
		if(firstOne)
		{
			min = pmin;
			max = pmax;
			firstOne = false;
		}
		else
		{
			min = min.Min(pmin);
			max = max.Max(pmax);
		}
	}
	return !firstOne;
}

quint64 AuScStatePart::MemoryUsage() const
{
	quint64 mem = AuScPart::MemoryUsage();
	mem += sizeof(m_State);
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
		mem += sizeof(quint8) + (*smi).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScDoor::AuScDoor()
{
	m_Access = DOOR_Automatic;
	m_DoorBits = 0;
}

AuScDoor::AuScDoor(const AuScDoor& c) : AuScStatePart(c), m_Silhouette(c.m_Silhouette)
{
	m_Access = c.m_Access;
	m_DoorBits = c.m_DoorBits;
	m_OpLoc1 = c.m_OpLoc1;
	m_OpLoc2 = c.m_OpLoc2;
	m_StopName = c.m_StopName;
}

AuScDoor::AuScDoor(const AuScDoor& c, const AuScSelectInfo& selInfo) : AuScStatePart(c, selInfo), m_Silhouette(c.m_Silhouette)
{
	m_Access = c.m_Access;
	m_DoorBits = c.m_DoorBits;
	m_OpLoc1 = c.m_OpLoc1;
	m_OpLoc2 = c.m_OpLoc2;
	m_StopName = c.m_StopName;
}

void AuScDoor::operator>>(QDataStream& out) const
{
	AuScStatePart::operator>>(out);

	quint8 hasWhat = 0;
	if(m_DoorBits)
		hasWhat |= 0x01;
	if(!m_StopName.isEmpty())
		hasWhat |= 0x02;
	if(!m_OpLoc1.IsZero())
		hasWhat |= 0x04;
	if(!m_OpLoc2.IsZero())
		hasWhat |= 0x08;
	if(m_Access != DOOR_Automatic)
		hasWhat |= 0x10;

	out << m_Silhouette;
	out << hasWhat;

	if(hasWhat & 0x01)
		out << m_DoorBits;

	if(hasWhat & 0x02)
		out << m_StopName;

	if(hasWhat & 0x04)
		out << m_OpLoc1;

	if(hasWhat & 0x08)
		out << m_OpLoc2;

	if(hasWhat & 0x10)
		out << m_Access;
}

void AuScDoor::operator<<(QDataStream& in)
{
	AuScStatePart::operator<<(in);

	if(ReadVersion() >= 0x0e)
	{
		in >> m_Silhouette;

		quint8 hasWhat;
		in >> hasWhat;

		if(hasWhat & 0x01)
			in >> m_DoorBits;

		if(hasWhat & 0x02)
			in >> m_StopName;

		if(hasWhat & 0x04)
			in >> m_OpLoc1;

		if(hasWhat & 0x08)
			in >> m_OpLoc2;

		if(hasWhat & 0x10)
			in >> m_Access;
	}
	else if(ReadVersion() >= 0x06)
	{
		if(ReadVersion() >= 0x0d)
			in >> m_Silhouette;
		else
			m_Silhouette.ReadOld(in);

		quint8 hasWhat;
		in >> hasWhat;

		// Older versions (<0x0c) store a hull door bit.
		if(hasWhat & 0x01)
			m_Access = DOOR_Hull;

		if(hasWhat & 0x02)
			in >> m_StopName;

		// Older versions (<0x0c) use the name to imply that it is a lift door.
		if(!m_StopName.isEmpty())
		{
			if(m_Access == DOOR_Hull)
				m_Access = DOOR_HullLift;
			else
				m_Access = DOOR_Lift;
		}

		if(hasWhat & 0x04)
			in >> m_OpLoc1;
		else
			m_OpLoc1 = AuPoint(0,0);

		if(hasWhat & 0x08)
			in >> m_OpLoc2;
		else
			m_OpLoc2 = AuPoint(0,0);

		// Later versions (>=0x0c) store the access byte.
		if(hasWhat & 0x10)
			in >> m_Access;

		if(hasWhat & 0x20)
			m_DoorBits |= DOORBIT_InitLocked;

		if(m_Access == DOOR_Hull || m_Access == DOOR_HullLift)
			m_DoorBits |= (DOORBIT_RemoteSpacecraft|DOORBIT_RemoteVehicle);
	}
	else
	{
		if(ReadVersion() >= 0x0d)
			in >> m_Silhouette;
		else if(ReadVersion() >= 0x02)
			m_Silhouette.ReadOld(in);

		if(ReadVersion() >= 0x03)
		{
			quint8 hullDoor;
			in >> hullDoor;
			if(hullDoor)
				m_Access = DOOR_Hull;
			else
				m_Access = DOOR_Automatic;
		}

		if(ReadVersion() >= 0x04)
		{
			in >> m_OpLoc1;
			in >> m_OpLoc2;
		}

		if(m_Access == DOOR_Hull)
			m_DoorBits |= (DOORBIT_RemoteSpacecraft|DOORBIT_RemoteVehicle);
	}
}

bool AuScDoor::operator==(const AuScPart& p) const
{
	try
	{
		const AuScDoor& c = dynamic_cast<const AuScDoor&>(p);
		if(!AuScStatePart::operator==(c))
			return false;

		if(m_Access != c.m_Access)
			return false;
		if(m_DoorBits != c.m_DoorBits)
			return false;
		if(m_OpLoc1 != c.m_OpLoc1)
			return false;
		if(m_OpLoc2 != c.m_OpLoc2)
			return false;
		//if(m_Silhouette != c.m_Silhouette)
			//return false;
		if(m_StopName != c.m_StopName)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScDoor::Clone() const
{
	return new AuScDoor(*this);
}

AuScPart* AuScDoor::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScDoor(*this, selInfo);
}

QString AuScDoor::TypeIcon() const
{
	QString typeIcons[] =
	{
		QObject::tr(":data/ScDoorAuto.png"),
		QObject::tr(":data/ScDoorStd.png"),
		QObject::tr(":data/ScDoorLift.png"),
		QObject::tr(":data/ScDoorHullLift.png"),
		QObject::tr(":data/ScDoorHull.png")
	};
	return typeIcons[m_Access];
}

QString AuScDoor::TypeName() const
{
	QString typeName[] =
	{
		QObject::tr("Automatic Door"),
		QObject::tr("Door"),
		QObject::tr("Turbo Lift Door"),
		QObject::tr("Hull Turbo Lift Door"),
		QObject::tr("Hull Door"),
	};
	return typeName[m_Access];
}

/*
QString AuScDoor::Name() const
{
	if(m_StopName.isEmpty())
		return TypeName();
	else
		return m_StopName;
}
*/

QString AuScDoor::SelectedDesc(const AuScParts& parts) const
{
	QString desc = AuScStatePart::SelectedDesc(parts);
	if(!m_StopName.isEmpty())
		desc += QObject::tr(", Stop=%1").arg(m_StopName);
	if(IsInitiallyLocked())
		desc += QObject::tr(", Locked");
	if(IsRemoteSpacecraftDoor())
		desc += QObject::tr(", RmtS");
	if(IsRemoteVehicleDoor())
		desc += QObject::tr(", RmtV");
	return desc;
}

QString AuScDoor::StateName(quint8 state) const
{
	if(state >= STATES_Count)
		return QObject::tr("Invalid State %1").arg(state);

	QString stateNames[] =
	{
		QObject::tr("Closed"),
		QObject::tr("Open 10%"),
		QObject::tr("Open 20%"),
		QObject::tr("Open 30%"),
		QObject::tr("Open 40%"),
		QObject::tr("Open 50%"),
		QObject::tr("Open 60%"),
		QObject::tr("Open 70%"),
		QObject::tr("Open 80%"),
		QObject::tr("Open 90%"),
		QObject::tr("Open"),
		QObject::tr("Stuck Closed"),
		QObject::tr("Stuck Open"),
		QObject::tr("Locked Closed"),
		QObject::tr("Locked Open"),
	};
	return stateNames[state];
}

QMap<quint8, QString> AuScDoor::StateNames() const
{
	QMap<quint8, QString> stateNames;
	for(quint8 state = Closed; state < STATES_Count; state++)
		if(HasState(state))
			stateNames.insert(state, StateName(state));
	return stateNames;
}

QMap<quint8, QString> AuScDoor::AddableStates() const
{
	QMap<quint8, QString> stateNames;
	for(quint8 state = Closed; state < STATES_Count; state++)
		if(!HasState(state))
			stateNames.insert(state, StateName(state));
	return stateNames;
}

QMap<quint8, QString> AuScDoor::DeletableStates() const
{
	QMap<quint8, QString> stateNames = StateNames();
	stateNames.remove(Closed);	// Cannot remove the closed state of doors.
	return stateNames;
}

bool AuScDoor::AddState(quint8 state)
{
	if(state >= STATES_Count)
		return false;
	if(m_StateMeshes.contains(state))
		return false;

	// Insert a new empty face list.
	m_StateMeshes.insert(state, AuScFaceList());

	// The designer usually wants to start editing this new state right away.
	SetState(state);
	return true;
}

bool AuScDoor::DeleteState(quint8 state)
{
	// Cannot delete the closed state of the door.
	if(state == Closed)
		return false;

	if(!m_StateMeshes.contains(state))
		return false;

	// Remove the face list.
	m_StateMeshes.remove(state);
	return true;
}

void AuScDoor::RefreshObstructionsCache(quint16 partId, const AuScParts& parts)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(Closed);
	if(smi == m_StateMeshes.end())
		return;
	m_Silhouette.Build(*smi, (float)d_ObstructMargin);
}

bool AuScDoor::RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts)
{
	if(m_Silhouette.IsValid())
		return false;
	RefreshObstructionsCache(partId, parts);
	return true;
}

bool AuScDoor::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	if(!IsClosed())
		return false;
	if(!m_Silhouette.IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
		return false;
	/*
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(Closed);
	if(smi == m_StateMeshes.cend())
		return false;
	AuPoint sLoc(toLoc);
	AuPoint sNorm;
	if(!(*smi).Sight(fromLoc, sLoc, sNorm))
		return false;
	hitLoc = sLoc;
	hitNormal = sNorm;
	*/
	return true;
}

bool AuScDoor::IsPathBlocked(quint16 partId, const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	AuDoorStateHash::const_iterator di = doorStates.find(partId);
	if(di == doorStates.cend())
		return false;
	if((*di).IsOpen())
		return false;
	if(!m_Silhouette.IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
		return false;
	/*
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find((*di).State());
	if(smi == m_StateMeshes.cend())
		return false;
	AuPoint sLoc(toLoc);
	AuPoint sNorm;
	if(!(*smi).Sight(fromLoc, sLoc, sNorm))
		return false;
	hitLoc = sLoc;
	hitNormal = sNorm;
	*/
	return true;
}

bool AuScDoor::IsInsideObstructionModel(const AuPoint& loc) const
{
	if(!IsClosed())
		return false;
	return m_Silhouette.IsInside(loc);
}

bool AuScDoor::IsInsideObstructionModel(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	Q_UNUSED(landingGearStates);
	if(doorStates.IsDoorOpen(partId))
		return false;
	return m_Silhouette.IsInside(loc);
}

bool AuScDoor::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	if(IsOmitted())
		return false;
	if(IsHullDoor())
		EffectHullFTL(parts.HullEffectSize(), shipHull);
	// Must not finalize the door in an intermediate state. If it is not all the way open, close it.
	if(State() != Open)
		SetState(Closed);
	return true;
}

void AuScDoor::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = 1;
	QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.find(Closed);
	if(mi != m_StateMeshes.cend())
		surface = (*mi).Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, SLINE_None);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int metal = (int)surface / d_HullPlatesPerUnit;
	if(metal < 1)
		metal = 1;	// Minimum 1 metal per room.
	pc.AddRequirement(hullMaterial, CE_Required, metal);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		// Add plastic for seals and trim.
		int plastic = metal / 20;
		if(!plastic)
			plastic = 1;
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);

		// Add parts for the mechanism.
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 1);
	}
}

void AuScDoor::SetDoorProperties(Qt::CheckState isAutomatic, Qt::CheckState initLocked, Qt::CheckState remoteSpacecraft,
		Qt::CheckState remoteVehicle)
{
	if(isAutomatic != Qt::PartiallyChecked)
	{
		switch(m_Access)
		{
		case DOOR_Automatic:
			if(isAutomatic == Qt::Unchecked)
				m_Access = DOOR_Standard;
			break;
		case DOOR_Standard:
			if(isAutomatic == Qt::Checked)
				m_Access = DOOR_Automatic;
			break;
		case DOOR_Lift:
		case DOOR_HullLift:
		case DOOR_Hull:
			// These doors are not automatic.
			break;
		}
	}

	if(initLocked == Qt::Checked)
		m_DoorBits |= DOORBIT_InitLocked;
	else if(initLocked == Qt::Unchecked)
		m_DoorBits &= ~DOORBIT_InitLocked;

	if(remoteSpacecraft == Qt::Checked)
		m_DoorBits |= DOORBIT_RemoteSpacecraft;
	else if(remoteSpacecraft == Qt::Unchecked)
		m_DoorBits &= ~DOORBIT_RemoteSpacecraft;

	if(remoteVehicle == Qt::Checked)
		m_DoorBits |= DOORBIT_RemoteVehicle;
	else if(remoteVehicle == Qt::Unchecked)
		m_DoorBits &= ~DOORBIT_RemoteVehicle;
}

void AuScDoor::SetHullDoor(bool hullDoor)
{
	if(hullDoor)
	{
		// This door is determined to be a hull door.
		switch(m_Access)
		{
		case DOOR_Automatic:
		case DOOR_Standard:
			m_Access = DOOR_Hull;
			break;
		case DOOR_Lift:
			m_Access = DOOR_HullLift;
			break;
		case DOOR_HullLift:
		case DOOR_Hull:
			break;
		}
	}
	else
	{
		// This door is determined to NOT be a hull door.
		switch(m_Access)
		{
		case DOOR_Automatic:
		case DOOR_Standard:
		case DOOR_Lift:
			break;
		case DOOR_HullLift:
			m_Access = DOOR_Lift;
			break;
		case DOOR_Hull:
			m_Access = DOOR_Automatic;
			break;
		}
	}
}

void AuScDoor::SetDoorOpLocs(const AuPoint& switch1, const AuPoint& switch2)
{
	if(switch1 == AuPoint::Invalid() || switch2 == AuPoint::Invalid())
	{
		// Calculate center of closed door geometry.
		AuPoint c;
		QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.find(Closed);
		if(mi != m_StateMeshes.cend())
		{
			AuPoint min, max;
			(*mi).Extents(min, max);
			c = (min + max)/2;
		}
		else
			c = Center();	// No closed geometry so get the center of all my geometry.

		if(switch1 == AuPoint::Invalid())
			m_OpLoc1 = c;
		else
			m_OpLoc1 = switch1;

		if(switch2 == AuPoint::Invalid())
			m_OpLoc2 = c;
		else
			m_OpLoc2 = switch2;
	}
	else
	{
		m_OpLoc1 = switch1;
		m_OpLoc2 = switch2;
	}
}

void AuScDoor::DoorOpLocs(AuPoint& opLoc1, AuPoint& opLoc2) const
{
	// Door operator locations may never have been set. This is allowed.
	if(m_OpLoc1 == AuPoint(0,0) && m_OpLoc2 == AuPoint(0,0))
	{
		// Door operator locations not set.
		QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.find(Closed);
		if(mi == m_StateMeshes.cend())
		{
			// No closed mesh?
			opLoc1 = m_OpLoc1;
			opLoc2 = m_OpLoc2;
		}
		else
		{
			// Calculate the center of my closed mesh for both operator locations.
			AuPoint min, max;
			(*mi).Extents(min, max);
			opLoc1 = (min + max)/2;
			opLoc2 = opLoc1;
		}
	}
	else
	{
		// Return established door operator locations.
		opLoc1 = m_OpLoc1;
		opLoc2 = m_OpLoc2;
	}
}

void AuScDoor::RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count], quint8 state) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(state);
	if(smi == m_StateMeshes.cend())
	{
		if(state == LockedClosed)
			smi = m_StateMeshes.find(Closed);
		if(smi == m_StateMeshes.cend())
			return;
	}
	(*smi).RenderTriangles(opaqueTris, transparentTris, themeColors);
}

bool AuScDoor::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	AuDoorStateHash::const_iterator di = doorStates.find(partId);
	if(di == doorStates.cend())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find((*di).State());
	if(smi == m_StateMeshes.cend())
	{
		if((*di).State() == LockedClosed)
			smi = m_StateMeshes.find(Closed);
		if(smi == m_StateMeshes.cend())
			return false;
	}

	return (*smi).Sight(eyeLoc, sightLoc, sightSurfaceNormal);
}

AuSysId AuScDoor::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Hull;
}

void AuScDoor::FacesInHullSilhouette(AuScFaceList& faces) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(Closed);
	if(smi == m_StateMeshes.cend())
		return;
	faces.AddFaces(*smi);
}

void AuScDoor::Transform(const AuMatrix4x4& t)
{
	AuScStatePart::Transform(t);
	if(!m_OpLoc1.IsZero())
		m_OpLoc1 = t.Project(m_OpLoc1);
	if(!m_OpLoc2.IsZero())
		m_OpLoc2 = t.Project(m_OpLoc2);
}

void AuScDoor::EdgeSegments(AuPointArray& edges) const
{
	AuScStatePart::EdgeSegments(edges);

	// Get door switch locations, calculated to center of closed model if they have never been set.
	AuPoint opLoc1, opLoc2;
	DoorOpLocs(opLoc1, opLoc2);

	// Draw an X at each door switch.
	double srad = 1.;
	{
		AuPoint p(opLoc1);
		AuPoint x0 = p - AuPoint(srad,0,0);
		AuPoint x1 = p + AuPoint(srad,0,0);
		edges.append(x0);
		edges.append(x1);

		AuPoint y0 = p - AuPoint(0,srad,0);
		AuPoint y1 = p + AuPoint(0,srad,0);
		edges.append(y0);
		edges.append(y1);

		AuPoint z0 = p - AuPoint(0,0,srad);
		AuPoint z1 = p + AuPoint(0,0,srad);
		edges.append(z0);
		edges.append(z1);
	}

	if(opLoc2 != opLoc1)
	{
		AuPoint p(opLoc2);
		AuPoint x0 = p - AuPoint(srad,0,0);
		AuPoint x1 = p + AuPoint(srad,0,0);
		edges.append(x0);
		edges.append(x1);

		AuPoint y0 = p - AuPoint(0,srad,0);
		AuPoint y1 = p + AuPoint(0,srad,0);
		edges.append(y0);
		edges.append(y1);

		AuPoint z0 = p - AuPoint(0,0,srad);
		AuPoint z1 = p + AuPoint(0,0,srad);
		edges.append(z0);
		edges.append(z1);
	}
}

bool AuScDoor::CanChangeStopName() const
{
	return true;
}

void AuScDoor::SetStopName(const QString& name)
{
	if(m_StopName == name)
		return;
	m_StopName = name;
	// Naming a door establishes whether it is a lift door.
	if(m_StopName.isEmpty())
	{
		// Not a lift door.
		if(IsHullDoor())
			m_Access = DOOR_Hull;
		else
			m_Access = DOOR_Automatic;
	}
	else
	{
		// Lift door.
		if(IsHullDoor())
			m_Access = DOOR_HullLift;
		else
			m_Access = DOOR_Lift;
	}
}

quint64 AuScDoor::MemoryUsage() const
{
	quint64 mem = AuScStatePart::MemoryUsage();
	mem += sizeof(m_Access);
	mem += sizeof(m_DoorBits);
	mem += StringMemoryUsage(m_StopName);
	mem += sizeof(m_OpLoc1);
	mem += sizeof(m_OpLoc2);
	mem += m_Silhouette.MemoryUsage();
	return mem;
}

QString AuScDoor::PartName() const
{
	if(m_StopName.isEmpty())
		return AuScStatePart::PartName();
	return QObject::tr("%1 %2").arg(AuScStatePart::PartName()).arg(m_StopName);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScWindow::AuScWindow()
{
}

AuScWindow::AuScWindow(const AuScWindow& c) : AuScMeshBase(c), m_Silhouette(c.m_Silhouette)
{
}

AuScWindow::AuScWindow(const AuScWindow& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo), m_Silhouette(c.m_Silhouette)
{
}

void AuScWindow::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
	out << m_Silhouette;
}

void AuScWindow::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);

	if(ReadVersion() >= 0x0d)
		in >> m_Silhouette;
	else if(ReadVersion() >= 0x02)
		m_Silhouette.ReadOld(in);
	if(ReadVersion() >= 0x03 && ReadVersion() < 0x10)
	{
		QString oldName;
		in >> oldName;
		SetGroupName(oldName);
	}
}

bool AuScWindow::operator==(const AuScPart& p) const
{
	try
	{
		const AuScWindow& c = dynamic_cast<const AuScWindow&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		//if(m_Silhouette != c.m_Silhouette)
			//return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScWindow::Clone() const
{
	return new AuScWindow(*this);
}

AuScPart* AuScWindow::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScWindow(*this, selInfo);
}

QString AuScWindow::SelectedDesc(const AuScParts& parts) const
{
	int t = 100 - (int)(Opacity() * 100);
	if(t >= 100)
		return QObject::tr("%1, Invisible").arg(FullName(parts));
	else if(t > 0)
		return QObject::tr("%1, %2% Transparent").arg(FullName(parts)).arg(t);
	else
		return QObject::tr("%1, Translucent").arg(FullName(parts));
}

QString AuScWindow::TypeIcon() const
{
	return QString(":data/ScWindow.png");
}

QString AuScWindow::TypeName() const
{
	return QObject::tr("Window");
}

float AuScWindow::Opacity() const
{
	if(m_Faces.empty())
		return 1;
	return m_Faces.first()->Opacity();
}

void AuScWindow::SetOpacity(float opacity)
{
	return m_Faces.SetOpacity(opacity);
}

void AuScWindow::RefreshObstructionsCache(quint16 partId, const AuScParts& parts)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	m_Silhouette.Build(m_Faces, (float)d_ObstructMargin);
}

bool AuScWindow::RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts)
{
	if(m_Silhouette.IsValid())
		return false;
	RefreshObstructionsCache(partId, parts);
	return true;
}

bool AuScWindow::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	if(!m_Silhouette.IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
		return false;
	/*
	AuPoint sLoc(toLoc);
	AuPoint sNorm;
	if(!m_Faces.Sight(fromLoc, sLoc, sNorm))
		return false;
	hitLoc = sLoc;
	hitNormal = sNorm;
	*/
	return true;
}

bool AuScWindow::IsInsideObstructionModel(const AuPoint& loc) const
{
	return m_Silhouette.IsInside(loc);
}

void AuScWindow::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, SLINE_None);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		// Add plastic for seals and trim.
		int plastic = (int)surface / d_HullPlatesPerUnit;
		if(plastic < 1)
			plastic = 1;	// Minimum 1 metal per room.

		// Add plastic for seals and trim.
		plastic += plastic / 20;
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	}
}

void AuScWindow::FacesInHullSilhouette(AuScFaceList& faces) const
{
	faces.AddFaces(m_Faces);
}

bool AuScWindow::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	if(IsOmitted())
		return false;
	EffectHullFTL(parts.HullEffectSize(), shipHull);
	return true;
}

quint64 AuScWindow::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	mem += m_Silhouette.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScLandingGear::AuScLandingGear()
{
}

AuScLandingGear::AuScLandingGear(const AuScLandingGear& c) : AuScStatePart(c), m_StateSilhouettes(c.m_StateSilhouettes)
{
}

AuScLandingGear::AuScLandingGear(const AuScLandingGear& c, const AuScSelectInfo& selInfo) : AuScStatePart(c, selInfo),
	m_StateSilhouettes(c.m_StateSilhouettes)
{
}

void AuScLandingGear::operator>>(QDataStream& out) const
{
	AuScStatePart::operator>>(out);

	out << (quint8)m_StateSilhouettes.size();
	for(QHash<quint8, AuScSilhouette3d>::const_iterator si = m_StateSilhouettes.cbegin(); si != m_StateSilhouettes.cend(); si++)
	{
		out << si.key();
		out << *si;
	}
}

void AuScLandingGear::operator<<(QDataStream& in)
{
	AuScStatePart::operator<<(in);

	m_StateSilhouettes.clear();
	quint8 c;
	in >> c;
	while(c--)
	{
		quint8 state;
		AuScSilhouette3d sil;

		in >> state;
		if(ReadVersion() >= 0x0d)
			in >> sil;
		else
			sil.ReadOld(in);

		m_StateSilhouettes.insert(state, sil);
	}
}

bool AuScLandingGear::operator==(const AuScPart& p) const
{
	try
	{
		const AuScLandingGear& c = dynamic_cast<const AuScLandingGear&>(p);
		if(!AuScStatePart::operator==(c))
			return false;

		//if(m_StateSilhouettes != c.m_StateSilhouettes)
			//return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScLandingGear::Clone() const
{
	return new AuScLandingGear(*this);
}

AuScPart* AuScLandingGear::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScLandingGear(*this, selInfo);
}

QString AuScLandingGear::TypeIcon() const
{
	return QString(":data/ScLandingGear.png");
}

QString AuScLandingGear::TypeName() const
{
	return QObject::tr("Landing Gear");
}

QString AuScLandingGear::StateName(quint8 state) const
{
	if(state >= STATES_Count)
		return QObject::tr("Invalid State %1").arg(state);

	QString stateNames[] =
	{
		QObject::tr("Extended"),
		QObject::tr("Retracted 10%"),
		QObject::tr("Retracted 20%"),
		QObject::tr("Retracted 30%"),
		QObject::tr("Retracted 40%"),
		QObject::tr("Retracted 50%"),
		QObject::tr("Retracted 60%"),
		QObject::tr("Retracted 70%"),
		QObject::tr("Retracted 80%"),
		QObject::tr("Retracted 90%"),
		QObject::tr("Retracted"),
		QObject::tr("Stuck Extended"),
		QObject::tr("Stuck Retracted"),
	};
	return stateNames[state];
}

QMap<quint8, QString> AuScLandingGear::StateNames() const
{
	QMap<quint8, QString> stateNames;
	for(quint8 state = Extended; state < STATES_Count; state++)
		if(HasState(state))
			stateNames.insert(state, StateName(state));
	return stateNames;
}

QMap<quint8, QString> AuScLandingGear::AddableStates() const
{
	QMap<quint8, QString> stateNames;
	for(quint8 state = Extended; state < STATES_Count; state++)
		if(!HasState(state))
			stateNames.insert(state, StateName(state));
	return stateNames;
}

QMap<quint8, QString> AuScLandingGear::DeletableStates() const
{
	QMap<quint8, QString> stateNames = StateNames();
	stateNames.remove(Extended);	// Cannot remove the extended state of landing gear.
	return stateNames;
}

bool AuScLandingGear::AddState(quint8 state)
{
	if(state >= STATES_Count)
		return false;
	if(m_StateMeshes.contains(state))
		return false;

	// Insert a new empty face list.
	m_StateMeshes.insert(state, AuScFaceList());

	// The designer usually wants to start editing this new state right away.
	SetState(state);
	return true;
}

bool AuScLandingGear::DeleteState(quint8 state)
{
	// Cannot delete the extended state of the landing gear.
	if(state == Extended)
		return false;

	if(!m_StateMeshes.contains(state))
		return false;

	// Remove the face list.
	m_StateMeshes.remove(state);
	return true;
}

void AuScLandingGear::RefreshObstructionsCache(quint16 partId, const AuScParts& parts)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	m_StateSilhouettes.clear();
	for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
		m_StateSilhouettes.insert(smi.key(), AuScSilhouette3d(*smi, (float)d_ObstructMargin));
}

bool AuScLandingGear::RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts)
{
	bool allValid = true;
	for(QHash<quint8, AuScSilhouette3d>::const_iterator si = m_StateSilhouettes.cbegin(); si != m_StateSilhouettes.cend(); si++)
		if(!(*si).IsValid())
		{
			allValid = false;
			break;
		}
	if(allValid)
		return false;
	RefreshObstructionsCache(partId, parts);
	return true;
}

bool AuScLandingGear::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	QHash<quint8, AuScSilhouette3d>::const_iterator si = m_StateSilhouettes.find(m_State);
	if(si == m_StateSilhouettes.cend())
		return false;
	if(!(*si).IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
		return false;
	/*
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(Extended);
	if(smi == m_StateMeshes.cend())
		return false;
	AuPoint sLoc(toLoc);
	AuPoint sNorm;
	if(!(*smi).Sight(fromLoc, sLoc, sNorm))
		return false;
	hitLoc = sLoc;
	hitNormal = sNorm;
	*/
	return true;
}

bool AuScLandingGear::IsPathBlocked(quint16 partId, const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc,
		AuPoint& hitNormal, const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates,
		const AuLiftHash& liftStops) const
{
	Q_UNUSED(doorStates);
	Q_UNUSED(liftStops);
	QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(partId);
	if(gi == landingGearStates.m_States.cend())
		return IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal);

	QHash<quint8, AuScSilhouette3d>::const_iterator si = m_StateSilhouettes.find(*gi);
	if(si == m_StateSilhouettes.cend())
		return false;					// No obstruction if the state silhouette is empty.
	if(!(*si).IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
		return false;
	/*
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(*gi);
	if(smi == m_StateMeshes.cend())
		return false;
	AuPoint sLoc(toLoc);
	AuPoint sNorm;
	if(!(*smi).Sight(fromLoc, sLoc, sNorm))
		return false;
	hitLoc = sLoc;
	hitNormal = sNorm;
	*/
	return true;
}

bool AuScLandingGear::IsInsideObstructionModel(const AuPoint& loc) const
{
	QHash<quint8, AuScSilhouette3d>::const_iterator si = m_StateSilhouettes.find(m_State);
	if(si == m_StateSilhouettes.cend())
		return false;
	return (*si).IsInside(loc);
}

bool AuScLandingGear::IsInsideObstructionModel(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	Q_UNUSED(doorStates);
	QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(partId);
	if(gi == landingGearStates.m_States.cend())
		return IsInsideObstructionModel(loc);

	QHash<quint8, AuScSilhouette3d>::const_iterator si = m_StateSilhouettes.find(*gi);
	if(si == m_StateSilhouettes.cend())
		return false;					// No obstruction if the state silhouette is empty.

	return (*si).IsInside(loc);
}

bool AuScLandingGear::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	Q_UNUSED(shipHull);
	if(IsOmitted())
		return false;
	// Must not finalize the gear in an intermediate state. If it is not all the way retracted, extend it.
	if(State() != Retracted)
		SetState(Extended);
	return true;
}

void AuScLandingGear::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = 1;
	QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.find(Extended);
	if(mi != m_StateMeshes.cend())
		surface = (*mi).Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(armor, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int metal = (int)surface / d_HullPlatesPerUnit;
	if(metal < 1)
		metal = 1;	// Minimum 1 metal per room.
	pc.AddRequirement(hullMaterial, CE_Required, metal);

	// Add parts for the mechanism.
	int mech = metal / 15;
	if(!mech)
		mech = 1;
	pc.AddRequirement(CID_MechanicalPart, CE_Required, mech);
}

void AuScLandingGear::RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count], quint8 state) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(state);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).RenderTriangles(opaqueTris, transparentTris, themeColors);
}

bool AuScLandingGear::HullExtents(quint16 partId, AuPoint& min, AuPoint& max, const AuLandingGearStateHash& landingGearStates) const
{
	QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(partId);
	if(gi == landingGearStates.m_States.cend())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator si = m_StateMeshes.find(*gi);
	if(si == m_StateMeshes.cend())
		return false;

	if((*si).empty())
		return false;

	(*si).Extents(min, max);
	return true;
}

bool AuScLandingGear::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(doorStates);
	Q_UNUSED(liftStops);
	QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(partId);
	if(gi == landingGearStates.m_States.cend())
		return false;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(*gi);
	if(smi == m_StateMeshes.cend())
		return false;

	return (*smi).Sight(eyeLoc, sightLoc, sightSurfaceNormal);
}

AuSysId AuScLandingGear::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Hull;
}

quint64 AuScLandingGear::MemoryUsage() const
{
	quint64 mem = AuScStatePart::MemoryUsage();
	for(QHash<quint8, AuScSilhouette3d>::const_iterator si = m_StateSilhouettes.cbegin(); si != m_StateSilhouettes.cend(); si++)
		mem += sizeof(quint8) + (*si).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScLift::AuScLift()
{
	m_GravityType = G_Natural;
}

AuScLift::AuScLift(const AuScLift& c) : AuScStatePart(c), m_PathSegments(c.m_PathSegments), m_Stops(c.m_Stops)
{
	m_GravityType = c.m_GravityType;
	m_GravityUp = c.m_GravityUp;
	m_Stop = c.m_Stop;
}

AuScLift::AuScLift(const AuScLift& c, const AuScSelectInfo& selInfo) : AuScStatePart(c, selInfo), m_PathSegments(c.m_PathSegments),
	m_Stops(c.m_Stops)
{
	m_GravityType = c.m_GravityType;
	m_GravityUp = c.m_GravityUp;
	m_Stop = c.m_Stop;
}

void AuScLift::operator>>(QDataStream& out) const
{
	AuScStatePart::operator>>(out);
	out << m_Stop;

	quint8 hasWhat = m_Stops.size() & 0x1f;	// This is why we depend on the number of stops being 31 or less.
	if(m_GravityType != G_Natural)
		hasWhat |= 0x80;
	if(m_GravityUp != AuPoint(0,0,1))
		hasWhat |= 0x40;
	if(!m_PathSegments.empty())
		hasWhat |= 0x20;

	out << hasWhat;
	for(QMap<QString, AuMatrix4x4>::const_iterator si = m_Stops.cbegin(); si != m_Stops.cend(); si++)
	{
		out << si.key();
		out << si.value();
	}

	if(hasWhat & 0x80)
		out << m_GravityType;

	if(hasWhat & 0x40)
		out << m_GravityUp;

	if(hasWhat & 0x20)
		out << m_PathSegments;
}

void AuScLift::operator<<(QDataStream& in)
{
	AuScStatePart::operator<<(in);
	in >> m_Stop;

	m_Stops.clear();
	quint8 hasWhat;
	in >> hasWhat;

	quint8 c = hasWhat & 0x1f;
	QString stop;
	AuMatrix4x4 t;
	while(c--)
	{
		in >> stop;
		in >> t;

		m_Stops.insert(stop, t);
	}

	if(hasWhat & 0x80)
		in >> m_GravityType;
	else
		m_GravityType = G_Natural;
	if(ReadVersion() < 0x0f)
		m_GravityType = G_Artificial;	// Due to a bug, all lifts prior to 0x0f had broken gravity type.

	if(hasWhat & 0x40)
	{
		in >> m_GravityUp;
		if(m_GravityUp.IsZero())
			m_GravityUp = AuPoint(0,0,1);
	}
	else
		m_GravityUp = AuPoint(0,0,1);

	if(hasWhat & 0x20)
		in >> m_PathSegments;
	else
		m_PathSegments.clear();
}

bool AuScLift::operator==(const AuScPart& p) const
{
	try
	{
		const AuScLift& c = dynamic_cast<const AuScLift&>(p);
		if(!AuScStatePart::operator==(c))
			return false;

		if(m_GravityType != c.m_GravityType)
			return false;
		if(m_GravityUp != c.m_GravityUp)
			return false;
		if(m_Stop != c.m_Stop)
			return false;
		if(m_Stops != c.m_Stops)
			return false;
		if(m_PathSegments != c.m_PathSegments)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScLift::Clone() const
{
	return new AuScLift(*this);
}

AuScPart* AuScLift::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScLift(*this, selInfo);
}

QString AuScLift::TypeIcon() const
{
	return QString(":data/ScLift.png");
}

QString AuScLift::TypeName() const
{
	return QObject::tr("Turbo Lift");
}

void AuScLift::MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps)
{
	AuScStatePart::MakeExtrusion(bnd, extVec, gridPlane, steps, true);
	if(State() != RoomVoid)
	{
		ReverseFaces();
		ReverseVertexNormals();
	}
}

void AuScLift::MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps,
		const AuMatrix4x4& gridPlane)
{
	AuScStatePart::MakeSphereLongLat(center, radius, lonSteps, latSteps, gridPlane);
	if(State() != RoomVoid)
	{
		ReverseFaces();
		ReverseVertexNormals();
	}
}

void AuScLift::MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
		const AuMatrix4x4& gridPlane)
{
	AuScStatePart::MakeSphereUniform(center, radius, equatorSteps, textureRepetitions, gridPlane);
	if(State() != RoomVoid)
	{
		ReverseFaces();
		ReverseVertexNormals();
	}
}

void AuScLift::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane)
{
	AuScStatePart::MakeSpindle(profile, steps, gridPlane);
	if(State() != RoomVoid)
	{
		ReverseFaces();
		ReverseVertexNormals();
	}
}

void AuScLift::MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides)
{
	AuScStatePart::MakeTube(path, diameter, sides, smoothSides);
	if(State() != RoomVoid)
	{
		ReverseFaces();
		ReverseVertexNormals();
	}
}

void AuScLift::MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides)
{
	AuScStatePart::MakeTube(path, shape, smoothSides);
	if(State() != RoomVoid)
	{
		ReverseFaces();
		ReverseVertexNormals();
	}
}

QString AuScLift::SelectedDesc(const AuScParts& parts) const
{
	Q_UNUSED(parts);
	QString g = GravityName(GravityType());
	if(m_Stops.empty())
		return QObject::tr("%1, No stops, G=%3")
			.arg(PartName())
			.arg(g);
	else if(m_Stop.isEmpty())
		return QObject::tr("%1, %2 stops, G=%3")
			.arg(PartName())
			.arg(m_Stops.size())
			.arg(g);
	else
		return QObject::tr("%1, at %2, %3 stops, G=%4")
			.arg(PartName())
			.arg(m_Stop)
			.arg(m_Stops.size())
			.arg(g);
}

void AuScLift::JigFaces(AuScFaceList& jigFaces) const
{
	// I must have a current stop to cut a door.
	if(m_Stop.isEmpty())
		return;
	QMap<QString, AuMatrix4x4>::const_iterator ti = m_Stops.find(m_Stop);
	if(ti == m_Stops.cend())
		return;

	// Find the door jig mesh.
	QHash<quint8, AuScFaceList>::const_iterator si = m_StateMeshes.find(DoorJig);
	if(si == m_StateMeshes.cend())
		return;

	// Position the jig to my stop.
	AuScFaceList faces(*si);
	faces.Transform(*ti);

	// Add the jig faces to the list.
	jigFaces.AddFaces(faces);
}

void AuScLift::AddFaces(const AuScFaceList& faces)
{
	if(IsAtDesignStop())
		AuScStatePart::AddFaces(faces);
	else
	{
		AuScFaceList tfaces(faces);
		tfaces.Transform(StopMatrixDesign().Inverse());
		AuScStatePart::AddFaces(tfaces);
	}
}

bool AuScLift::CutFaces(const AuPointArray& cutLine)
{
	if(IsAtDesignStop())
		return AuScStatePart::CutFaces(cutLine);

	AuPointArray cline(cutLine);
	cline.Transform(StopMatrixDesign().Inverse());
	return AuScStatePart::CutFaces(cline);
}

bool AuScLift::PunchFaces(const AuPointArray& cutBnd)
{
	if(IsAtDesignStop())
		return AuScStatePart::PunchFaces(cutBnd);

	AuPointArray cline(cutBnd);
	cline.Transform(StopMatrixDesign().Inverse());
	return AuScStatePart::PunchFaces(cline);
}

bool AuScLift::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		return AuScStatePart::DeselectEdgesAt(edgeSeg, selInfo);

	AuSegment eseg(edgeSeg);
	eseg.Transform(StopMatrixDesign().Inverse());
	return AuScStatePart::DeselectEdgesAt(eseg, selInfo);
}

bool AuScLift::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		return AuScStatePart::DeselectFacesAt(atLoc, selInfo);

	AuPoint aloc = StopMatrixDesign().UnProject(atLoc);
	return AuScStatePart::DeselectFacesAt(aloc, selInfo);
}

bool AuScLift::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		return AuScStatePart::DeselectVerticesAt(atLoc, selInfo);

	AuPoint aloc = StopMatrixDesign().UnProject(atLoc);
	return AuScStatePart::DeselectVerticesAt(aloc, selInfo);
}

double AuScLift::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	if(IsAtDesignStop())
		return AuScStatePart::EdgeDistance(mvp, pixel, snapLoc);

	return AuScStatePart::EdgeDistance(mvp * StopMatrixDesign(), pixel, snapLoc);
}

double AuScLift::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	if(IsAtDesignStop())
		return AuScStatePart::EdgeDistance(mvp, pixel, edgeSeg);

	return AuScStatePart::EdgeDistance(mvp * StopMatrixDesign(), pixel, edgeSeg);
}

void AuScLift::EdgeSegments(AuPointArray& edges) const
{
	if(IsAtDesignStop())
		AuScStatePart::EdgeSegments(edges);
	else
	{
		AuPointArray medges;
		AuScStatePart::EdgeSegments(medges);
		medges.Transform(StopMatrixDesign());
		edges.append(medges);
	}

	if(m_State != RoomVoid)
		return;

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend() || (*smi).empty())
		return;

	AuPoint cen = (*smi).first()->FaceLocation();
	AuPoint gravityUp;
	if(!IsAtDesignStop())
	{
		cen = StopMatrixDesign().Project(cen);
		gravityUp = StopMatrixDesign().ProjectVector(m_GravityUp);
		gravityUp.Unit();
	}
	else
		gravityUp = m_GravityUp;
	AuPoint top = cen + gravityUp * FeetPerMeter();
	AuPoint p;
	gravityUp.ArbitraryAxis(p);
	AuPoint ar1 = cen - (gravityUp * FeetPerMeter() * .2) + p * (FeetPerMeter() * .1);
	AuPoint ar2 = cen - (gravityUp * FeetPerMeter() * .2) - p * (FeetPerMeter() * .1);
	edges.append(top);
	edges.append(ar1);

	edges.append(ar1);
	edges.append(cen);

	edges.append(cen);
	edges.append(ar2);

	edges.append(ar2);
	edges.append(top);
}

double AuScLift::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	if(IsAtDesignStop())
		return AuScStatePart::FaceDistance(mvp, pixel, faceIndex, gripLoc);

	return AuScStatePart::FaceDistance(mvp * StopMatrixDesign(), pixel, faceIndex, gripLoc);
}

void AuScLift::Faces(AuScFaceList& faces) const
{
	if(IsAtDesignStop())
		AuScStatePart::Faces(faces);
	else
	{
		AuScFaceList mfaces;
		AuScStatePart::Faces(mfaces);
		mfaces.Transform(StopMatrixDesign());
		faces.append(mfaces);
		mfaces.clear();
	}
}

void AuScLift::Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		AuScStatePart::Faces(faces, selInfo);
	else
	{
		AuScFaceList mfaces;
		AuScStatePart::Faces(mfaces, selInfo);
		mfaces.Transform(StopMatrixDesign());
		faces.append(mfaces);
		mfaces.clear();
	}
}

void AuScLift::JigCutFaces(AuScFaceList& faces) const
{
	// We keep the original mesh for jig cutting.
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(CarOriginal);
	if(smi == m_StateMeshes.cend())
		smi = m_StateMeshes.find(Car);

	// No original mesh, give them the Car.
	if(smi == m_StateMeshes.cend())
		return;

	faces = *smi;

	if(!IsAtDesignStop())
		faces.Transform(StopMatrixDesign());
}

void AuScLift::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	if(IsAtDesignStop())
		AuScStatePart::FaceSpriteLocs(selFaces, selLocs, unselLocs);
	else
	{
		QList<AuPointAlpha> mselLocs;
		QList<AuPointAlpha> munselLocs;
		AuScStatePart::FaceSpriteLocs(selFaces, mselLocs, munselLocs);

		const AuMatrix4x4& sm = StopMatrixDesign();
		for(QList<AuPointAlpha>::iterator li = mselLocs.begin(); li != mselLocs.end(); li++)
			(*li).Transform(sm);
		selLocs.append(mselLocs);

		for(QList<AuPointAlpha>::iterator li = munselLocs.begin(); li != munselLocs.end(); li++)
			(*li).Transform(sm);
		unselLocs.append(munselLocs);
	}
}

bool AuScLift::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	if(IsAtDesignStop())
		return AuScStatePart::InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces);

	return AuScStatePart::InViewRectangle(mvp * StopMatrixDesign(), pixelRect, findVertices, findEdges, findFaces);
}

bool AuScLift::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	if(IsAtDesignStop())
		return InViewRectangle(mvp, pixelRect, selInfo, findVertices, findEdges, findFaces);

	return AuScStatePart::InViewRectangle(mvp * StopMatrixDesign(), pixelRect, selInfo, findVertices, findEdges, findFaces);
}

void AuScLift::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	if(IsAtDesignStop())
		AuScStatePart::RenderTriangles(opaqueTris, transparentTris, themeColors);
	else
	{
		AuScTriRenderList mopaqueTris, mtransparentTris;
		AuScStatePart::RenderTriangles(mopaqueTris, mtransparentTris, themeColors);
		mopaqueTris.Transform(StopMatrixDesign());
		mtransparentTris.Transform(StopMatrixDesign());
		opaqueTris.Insert(mopaqueTris);
		transparentTris.Insert(mtransparentTris);
	}
}

bool AuScLift::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;
	return AuScStatePart::SelectContiguous(angleTol, selInfo);
}

bool AuScLift::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		return AuScStatePart::SelectEdgesAt(edgeSeg, selInfo);

	AuSegment eseg(edgeSeg);
	eseg.Transform(StopMatrixDesign().Inverse());
	return AuScStatePart::SelectEdgesAt(eseg, selInfo);
}

bool AuScLift::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		return AuScStatePart::SelectFaceAt(atLoc, selInfo);

	AuPoint aloc = StopMatrixDesign().UnProject(atLoc);
	return AuScStatePart::SelectFaceAt(aloc, selInfo);
}

bool AuScLift::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		return AuScStatePart::SelectFaceAt(atLoc, selInfo);

	AuPoint aloc = StopMatrixDesign().UnProject(atLoc);
	return AuScStatePart::SelectFacesAt(aloc, selInfo);
}

bool AuScLift::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	if(IsAtDesignStop())
		return AuScStatePart::SelectVerticesAt(atLoc, selInfo);

	AuPoint aloc = StopMatrixDesign().UnProject(atLoc);
	return AuScStatePart::SelectVerticesAt(aloc, selInfo);
}

void AuScLift::SetFaces(const AuScFaceList& faces)
{
	if(IsAtDesignStop())
		AuScStatePart::SetFaces(faces);
	else
	{
		AuScFaceList mfaces(faces);
		mfaces.Transform(StopMatrixDesign().Inverse());
		AuScStatePart::SetFaces(mfaces);
	}
}

void AuScLift::SetJigCutFaces(const AuScFaceList& faces)
{
	// Find my Car mesh. It is the only one that could be jig cut.
	QHash<quint8, AuScFaceList>::const_iterator csmi = m_StateMeshes.find(Car);
	if(csmi == m_StateMeshes.cend())
		return;	// This thing is broken. D:

	// Preserve my original mesh for future door cuts.
	QHash<quint8, AuScFaceList>::iterator smi = m_StateMeshes.find(CarOriginal);
	if(smi == m_StateMeshes.end())
		m_StateMeshes.insert(CarOriginal, *csmi);

	SetFaces(faces);
}

void AuScLift::TransformLiftStop(const AuMatrix4x4& t)
{
	QMap<QString, AuMatrix4x4>::iterator ti = m_Stops.find(m_Stop);
	if(ti != m_Stops.end())
		*ti = t * *ti;
}

void AuScLift::Transform(const AuMatrix4x4& t)
{
	AuScStatePart::Transform(t);
	m_GravityUp = t.ProjectVector(m_GravityUp);
	m_GravityUp.Unit();
	for(QMap<QString, AuMatrix4x4>::iterator ti = m_Stops.begin(); ti != m_Stops.end(); ti++)
		*ti = t * *ti / t;
}

void AuScLift::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	AuScStatePart::Transform(t, selInfo);
	// In face mode, we transform individual faces; we do not change stop matrices.
}

void AuScLift::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	AuScStatePart::TransformCopy(t, selInfo);
	// In face mode, we transform individual faces; we do not change stop matrices.
}

double AuScLift::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	if(IsAtDesignStop())
		return AuScStatePart::VertexDistance(mvp, pixel, vertexIndex, snapLoc);

	double dist = AuScStatePart::VertexDistance(mvp * StopMatrixDesign(), pixel, vertexIndex, snapLoc);
	if(snapLoc)
		*snapLoc = StopMatrixDesign().Project(*snapLoc);
	return dist;
}

void AuScLift::VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	if(IsAtDesignStop())
		AuScStatePart::VertexSpriteLocs(selVerts, selLocs, unselLocs);
	else
	{
		QList<AuPointAlpha> mselLocs;
		QList<AuPointAlpha> munselLocs;
		AuScStatePart::VertexSpriteLocs(selVerts, mselLocs, munselLocs);

		const AuMatrix4x4& sm = StopMatrixDesign();
		for(QList<AuPointAlpha>::iterator li = mselLocs.begin(); li != mselLocs.end(); li++)
			(*li).Transform(sm);
		selLocs.append(mselLocs);

		for(QList<AuPointAlpha>::iterator li = munselLocs.begin(); li != munselLocs.end(); li++)
			(*li).Transform(sm);
		unselLocs.append(munselLocs);
	}
}

bool AuScLift::AddState(quint8 state)
{
	if(state >= STATES_Count)
		return false;
	if(m_StateMeshes.contains(state))
		return false;

	// Insert a new empty face list.
	m_StateMeshes.insert(state, AuScFaceList());

	// The designer usually wants to start editing this new state right away.
	SetState(state);
	return true;
}

QString AuScLift::StateName(quint8 state) const
{
	if(state >= STATES_Count)
		return QObject::tr("Invalid State %1").arg(state);

	QString stateNames[] =
	{
		QObject::tr("Lift Car"),
		QObject::tr("Door Jig"),
		QObject::tr("Uncut Car"),
		QObject::tr("Room Void"),
	};
	return stateNames[state];
}

QMap<quint8, QString> AuScLift::StateNames() const
{
	QMap<quint8, QString> stateNames;
	for(quint8 state = Car; state < STATES_Count; state++)
		if(HasState(state))
			stateNames.insert(state, StateName(state));
	return stateNames;
}

QMap<quint8, QString> AuScLift::AddableStates() const
{
	QMap<quint8, QString> stateNames;
	for(quint8 state = Car; state < STATES_Count; state++)
		if(state != CarOriginal && !HasState(state))
			stateNames.insert(state, StateName(state));
	return stateNames;
}

QMap<quint8, QString> AuScLift::DeletableStates() const
{
	QMap<quint8, QString> stateNames = StateNames();
	stateNames.remove(Car);			// Cannot remove the 'Car' state of lifts.
	stateNames.remove(CarOriginal);	// Cannot remove the 'CarOriginal' state of lifts.
	stateNames.remove(RoomVoid);	// Cannot remove the 'RoomVoid' state of lifts.
	return stateNames;
}

bool AuScLift::DeleteState(quint8 state)
{
	// Can not delete the Car state of the lift.
	if(state == Car)
		return false;
	else if(state == CarOriginal)
		return false;
	else if(state == RoomVoid)
		return false;

	if(!m_StateMeshes.contains(state))
		return false;

	// Remove the face list.
	m_StateMeshes.remove(state);
	return true;
}

bool AuScLift::CanAddStop() const
{
	// Insure we don't exceed 63, for our stream operators.
	return m_Stops.size() < 63;
}

bool AuScLift::AddStop(const QString& stop)
{
	// Insure we don't exceed 63, for our stream operators.
	if(m_Stops.size() >= 63)
		return false;

	// Current location of geometry is now named 'stop'.
	m_Stops.insert(stop, AuMatrix4x4());
	return true;
}

bool AuScLift::CanDeleteStop() const
{
	return !m_Stops.empty();
}

QStringList AuScLift::DeletableStops() const
{
	return m_Stops.keys();
}

bool AuScLift::DeleteStop(const QString& stop)
{
	if(!m_Stops.contains(stop))
		return false;

	m_Stops.remove(stop);
	if(m_Stop == stop)
	{
		if(m_Stops.empty())
			m_Stop = "";
		else
			m_Stop = m_Stops.begin().key();
	}

	return true;
}

void AuScLift::SetStop(const QString& stop)
{
	if(m_Stops.contains(stop) || stop.isEmpty())
		m_Stop = stop;
}

bool AuScLift::RenameStop(const QString& oldName, const QString& nuName)
{
	if(oldName.isEmpty())// || !IsNameValid(oldName))
		return false;
	if(nuName.isEmpty() || !IsNameValid(nuName))
		return false;
	if(m_Stops.contains(nuName))
		return false;
	QMap<QString, AuMatrix4x4>::iterator si = m_Stops.find(oldName);
	if(si == m_Stops.end())
		return false;
	// Rename the stop.
	AuMatrix4x4 m(*si);			// Preserve the matrix.
	m_Stops.erase(si);			// Remove the old stop.
	m_Stops.insert(nuName, m);	// Insert the matrix using the nu stop name.
	if(m_Stop == oldName)
		m_Stop = nuName;		// Change current stop name.
	return true;
}

QString AuScLift::AtStop() const
{
	return m_Stop;
}

QStringList AuScLift::StopNames() const
{
	return m_Stops.keys();
}

bool AuScLift::CanChangeStop() const
{
	return m_Stops.size() > 1;
}

bool AuScLift::IsTurboLiftMesh() const
{
	return true;
}

bool AuScLift::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(shipHull);
	if(IsOmitted())
		return false;

	// Locate the room void mesh.
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return false;

	// Remove states that are only needed during the design process.
	if(m_StateMeshes.contains(DoorJig))
		m_StateMeshes.remove(DoorJig);
	if(m_StateMeshes.contains(CarOriginal))
		m_StateMeshes.remove(CarOriginal);

	// Build interconnecting paths between all my stops.
	m_PathSegments.clear();

	// Make a list of all stops and their corresponding path locations. This takes some work.
	QMap<QString, AuPoint> stopPathLocs;
	for(QMap<QString, AuMatrix4x4>::const_iterator si = m_Stops.cbegin(); si != m_Stops.cend(); si++)
	{
		// Find the door associated with the stop.
		quint16 doorId = 0;
		for(AuScParts::const_iterator di = parts.cbegin(); di != parts.cend(); di++)
		{
			if((*di)->IsOmitted())
				continue;
			if(!(*di)->IsDoor())
				continue;
			if((*di)->StopName() != si.key())
				continue;
			doorId = di.key();	// Found a door with a matching stop name.
			break;
		}
		if(!doorId)
			continue;	// Skip stop with no door. This should have been caught in AuScReport::OkToManufacture.

		// Find a path associated with the open state of the door that has one end inside the lift room void at the stop.
		quint16 pathId = 0;
		AuPoint pathLoc;
		for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if(!(*pi)->IsPath())
				continue;
			if((*pi)->AssociatedPartId() != doorId || (*pi)->AssociatedPartState() != AuScDoor::Open)
				continue;

			const AuScPathSeg& seg = (*pi)->PathSegment(0);

			// Check to see if either end of the path segment is inside the room void at the stop.
			AuPoint sloc = (*si).UnProject(seg.Loc1());			// Move loc into the space of the stop.
			if((*smi).TestInside(sloc) != 1)
			{
				// Loc is inside the room void at the stop.
				pathId = pi.key();
				pathLoc = seg.Loc1();
				break;
			}
			sloc = (*si).UnProject(seg.Loc2());					// Move loc into the space of the stop.
			if((*smi).TestInside(sloc) != 1)
			{
				// Loc is inside the room void at the stop.
				pathId = pi.key();
				pathLoc = seg.Loc2();
				break;
			}
		}
		if(!pathId)
			continue;	// Skip stop with no path that ends inside the lift there.
		stopPathLocs.insert(si.key(), pathLoc);
	}

	for(QMap<QString, AuPoint>::const_iterator fromi = stopPathLocs.cbegin(); fromi != stopPathLocs.cend(); fromi++)
	{
		for(QMap<QString, AuPoint>::const_iterator toi = stopPathLocs.cbegin(); toi != stopPathLocs.cend(); toi++)
		{
			if(fromi.key() == toi.key())
				continue;	// Don't link a stop to itself.
			AuScPathSeg pathSeg(partId, fromi.key(), *fromi, toi.key(), *toi);
			m_PathSegments.AddIfUniqueSegment(pathSeg);
		}
	}
	return true;
}

void AuScLift::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = 1;
	QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.cbegin();
	if(mi != m_StateMeshes.cend())
		surface = (*mi).Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, SLINE_None);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int metal = (int)surface / d_HullPlatesPerUnit;
	if(metal < 1)
		metal = 1;	// Minimum 1 metal per room.
	pc.AddRequirement(hullMaterial, CE_Required, metal);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		// Add plastic for seals and trim.
		int plastic = metal / 20;
		if(!plastic)
			plastic = 1;
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	}

	// Add parts for the mechanism.
	int mech = m_Stops.size();
	pc.AddRequirement(CID_MechanicalPart, CE_Required, mech);
}

bool AuScLift::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	const AuMatrix4x4& stopM = StopMatrixAt(partId, liftStops);
	AuPoint eLoc = stopM.UnProject(eyeLoc);
	AuPoint sLoc = stopM.UnProject(sightLoc);

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(Car);
	if(smi != m_StateMeshes.cend())
		return false;

	if(!(*smi).Sight(eLoc, sLoc, sightSurfaceNormal))
		return false;

	sightLoc = stopM.Project(sLoc);
	return true;
}

AuSysId AuScLift::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Hull;
}

bool AuScLift::IsInsideVoid(const AuPoint& loc) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return false;

	if(IsAtDesignStop())
		return (*smi).TestInside(loc) != 1;

	// Surface loc counts as being inside, to allow passage from one void to another, in case someone abuts two voids right 
	// against each other.
	AuPoint sloc = StopMatrixDesign().UnProject(loc);	// Move loc into the space of the current stop.
	return (*smi).TestInside(sloc) != 1;
}

bool AuScLift::IsInsideVoid(const AuPoint& loc, const AuScParts& parts) const
{
	Q_UNUSED(parts);
	return IsInsideVoid(loc);
}

bool AuScLift::IsInsideVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return false;

	AuLiftHash::const_iterator li = liftStops.find(partId);
	if(li == liftStops.cend())
		return false;

	QMap<QString, AuMatrix4x4>::const_iterator si = m_Stops.find(*li);
	if(si == m_Stops.cend())
		return false;

	// Surface loc counts as being inside, to allow passage from one void to another, in case someone abuts two voids right 
	// against each other.
	AuPoint sloc = (*si).UnProject(loc);	// Move loc into the space of the current stop.
	return (*smi).TestInside(sloc) != 1;
}

bool AuScLift::IsInsideVoidEver(const AuPoint& loc) const
{
	// Locate the room void mesh.
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return false;

	// Check all my stops.
	for(QMap<QString, AuMatrix4x4>::const_iterator si = m_Stops.cbegin(); si != m_Stops.cend(); si++)
	{
		AuPoint sloc = (*si).UnProject(loc);					// Move loc into the space of the stop.
		if((*smi).TestInside(sloc) != 1)
			return true;										// Loc is inside the room void at the stop.
	}
	return false;
}

bool AuScLift::IsInsideRoomVoid(const AuPoint& loc, const AuScParts& parts) const
{
	return IsInsideVoid(loc, parts);
}

bool AuScLift::IsInsideRoomVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return IsInsideVoid(partId, loc, doorStates, landingGearStates, liftStops);
}

bool AuScLift::IsInsideRoomVoid(const AuPoint& loc, double elevate, const AuScParts& parts) const
{
	if(IsAtDesignStop())
	{
		AuPoint eloc = loc + m_GravityUp * elevate;
		return IsInsideVoid(eloc, parts);
	}

	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return false;

	// Surface loc counts as being inside, to allow passage from one void to another, in case someone abuts two voids right 
	// against each other.
	// Move loc into the space of the current stop.
	AuPoint sloc = StopMatrixDesign().UnProject(loc) + StopMatrixDesign().UnProjectVector(m_GravityUp) * elevate;
	return (*smi).TestInside(sloc) != 1;
}

bool AuScLift::IsInsideRoomVoid(quint16 partId, const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return false;

	AuLiftHash::const_iterator li = liftStops.find(partId);
	if(li == liftStops.cend())
		return false;

	QMap<QString, AuMatrix4x4>::const_iterator si = m_Stops.find(*li);
	if(si == m_Stops.cend())
		return false;

	// Surface loc counts as being inside, to allow passage from one void to another, in case someone abuts two voids right 
	// against each other.
	// Move loc into the space of the current stop.
	AuPoint sloc = (*si).UnProject(loc) + (*si).UnProjectVector(m_GravityUp) * elevate;
	return (*smi).TestInside(sloc) != 1;
}

void AuScLift::FacesInHullSilhouette(AuScFaceList& faces) const
{
	// Locate the lift car faces.
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(Car);
	if(smi == m_StateMeshes.cend())
		return;

	// Collect faces at all my stops.
	for(QMap<QString, AuMatrix4x4>::const_iterator si = m_Stops.cbegin(); si != m_Stops.cend(); si++)
	{
		AuScFaceList stopFaces(*smi);	// Get the faces from the stop.
		stopFaces.Transform(*si);		// Transform the faces to the stop position.
		faces.AddFaces(stopFaces);		// Add them to the big list.
	}
}

double AuScLift::HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM, AuPoint& floorN) const
{
	AuPoint floorLoc = FaceLoc(loc, -gravityN, floorN, stopM);
	return floorLoc.Distance(loc);
}

double AuScLift::HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM) const
{
	AuPoint terN;
	AuPoint ceilLoc = FaceLoc(loc, gravityN, terN, stopM);
	return ceilLoc.Distance(loc);
}

AuPoint AuScLift::FaceLoc(const AuPoint& loc, const AuPoint& vec, AuPoint& terN, const AuMatrix4x4& stopM) const
{
	// Locate the room void faces.
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return loc;

	AuPoint sloc = stopM.UnProject(loc);						// Transform the search location relative to the current stop.
	AuPoint svec = stopM.UnProjectVector(vec);					// Transform the search direction vec relative to the current stop.
	AuPoint hitLoc;
	(*smi).FindNearestFace(sloc, svec, hitLoc, terN);
	hitLoc = stopM.Project(hitLoc);								// Transform the resulting hit loc relative to the design.
	terN = stopM.ProjectVector(terN);							// Transform the resulting surface normal relative to the design.
	terN.Unit();
	return hitLoc;
}

const AuMatrix4x4& AuScLift::StopMatrixAt(quint16 partId, const AuLiftHash& liftStops) const
{
	AuLiftHash::const_iterator li = liftStops.find(partId);
	if(li == liftStops.cend())
		return AuMatrix4x4::IdentityMatrix();
	return StopMatrixAt(*li);
}

quint64 AuScLift::MemoryUsage() const
{
	quint64 mem = AuScStatePart::MemoryUsage();
	mem += sizeof(m_GravityType);
	mem += sizeof(m_GravityUp);
	mem += m_PathSegments.MemoryUsage();
	mem += StringMemoryUsage(m_Stop);
	for(QMap<QString, AuMatrix4x4>::const_iterator si = m_Stops.cbegin(); si != m_Stops.cend(); si++)
		mem += StringMemoryUsage(si.key()) + sizeof(AuMatrix4x4);
	return mem;
}

AuPoint AuScLift::GravityUp(const AuMatrix4x4& stopM) const
{
	return stopM.ProjectVector(m_GravityUp);
}

AuPoint AuScLift::GravityUp(const AuPoint& gravityN, const AuMatrix4x4& stopM) const
{
	if(m_GravityType == G_Artificial)
		return stopM.ProjectVector(m_GravityUp);
	else
		return stopM.ProjectVector(gravityN);
}

bool AuScLift::CheckVoidFacesPlanar() const
{
	// Locate the room void faces.
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(RoomVoid);
	if(smi == m_StateMeshes.cend())
		return true;	// No room void to have bent faces.
	// Voids require that all faces be planar. Bent faces do not work with the 'is inside' algorithm.
	return (*smi).CheckFacesPlanar();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScScreen::AuScScreen()
{
}

AuScScreen::AuScScreen(const AuScScreen& c) : AuScPart(c)
{
	m_Face = c.m_Face;
}

AuScScreen::AuScScreen(const AuScScreen& c, const AuScSelectInfo& selInfo) : AuScPart(c)
{
	Q_UNUSED(selInfo);
	m_Face = c.m_Face;
}

void AuScScreen::operator>>(QDataStream& out) const
{
	AuScPart::operator>>(out);
	out << m_Face;
}

void AuScScreen::operator<<(QDataStream& in)
{
	AuScPart::operator<<(in);
	in >> m_Face;
}

bool AuScScreen::operator==(const AuScPart& p) const
{
	try
	{
		const AuScScreen& c = dynamic_cast<const AuScScreen&>(p);
		if(!AuScPart::operator==(c))
			return false;

		if(m_Face != c.m_Face)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuScScreen::EdgeIds(QSet<quint32>& edgeIds) const
{
	for(int v=0; v<m_Face.Edges(); v++)
		edgeIds.insert(v);
}

void AuScScreen::FaceIds(QSet<quint32>& faceIds) const
{
	faceIds.insert(0);
}

void AuScScreen::VertexIds(QSet<quint32>& vertexIds) const
{
	for(int v=0; v<m_Face.VertexCount(); v++)
		vertexIds.insert(v);
}

AuScPoint AuScScreen::VertexLoc(quint32 vertexIndex) const
{
	if((int)vertexIndex >= m_Face.VertexCount())
		return AuScPoint();

	return m_Face.Vertex(vertexIndex);
}

bool AuScScreen::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Face.InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces);
}

bool AuScScreen::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyInRect = false;

	if(findVertices)
	{
		// Test all projected vertices to see if they are inside the pixel rectangle.
		int verts = m_Face.VertexCount();
		for(int v=0; v<verts; v++)
		{
			AuPoint vloc = mvp.Project(m_Face.Vertex(v));
			if(vloc.z > 0 && pixelRect.contains(QPointF(vloc.x, vloc.y)))
			{
				selInfo.m_Vertices.insert(v);
				anyInRect = true;
			}
		}
	}

	if(findFaces)
	{
		AuPoint floc = mvp.Project(m_Face.FaceLocation());
		if(floc.z > 0 && pixelRect.contains(QPointF(floc.x, floc.y)))
		{
			selInfo.m_Faces.insert(0);
			anyInRect = true;
		}
	}

	if(findEdges)
	{
		int segs = m_Face.Edges();
		AuSegment seg;
		for(int s=0; s<segs; s++)
		{
			m_Face.EdgeSegment(s, seg);
			seg.Transform(mvp);
			if((seg.m_fLoc1.z > 0 || seg.m_fLoc2.z > 0) &&
					pixelRect.contains(QPointF(seg.m_fLoc1.x, seg.m_fLoc1.y)) &&
					pixelRect.contains(QPointF(seg.m_fLoc2.x, seg.m_fLoc2.y)))
			{
				selInfo.m_Edges.insert(s);
				anyInRect = true;
			}
		}
	}

	return anyInRect;
}

bool AuScScreen::IsValid() const
{
	return !m_Face.Collapsed();
}

void AuScScreen::EdgeSegments(AuPointArray& edges) const
{
	AuSegmentArray fsegs;
	m_Face.EdgeSegments(fsegs);

	for(AuSegmentArray::const_iterator si = fsegs.cbegin(); si != fsegs.cend(); si++)
	{
		edges.append((*si).m_fLoc1);
		edges.append((*si).m_fLoc2);
	}
}

void AuScScreen::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;

	int faceIndex = 0;
	QMap<AuPoint, bool> alreadySel;
	QMap<AuPoint, bool> alreadyUnsel;
	AuPoint v(m_Face.FaceLocation());
	if(selFaces.contains(faceIndex))
	{
		if(!alreadySel.contains(v))
		{
			alreadySel.insert(v, true);
			selLocs.append(AuPointAlpha(v, selSize));
		}
	}
	else
	{
		if(!alreadyUnsel.contains(v))
		{
			alreadyUnsel.insert(v, true);
			unselLocs.append(AuPointAlpha(v, unselSize));
		}
	}
}

void AuScScreen::VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;
	int vertexIndex = 0;
	QMap<AuPoint, bool> alreadySel;
	QMap<AuPoint, bool> alreadyUnsel;
	int verts = m_Face.VertexCount();
	for(int vi=0; vi<verts; vi++)
	{
		AuPoint v(m_Face.Vertex(vi));
		if(selVerts.contains(vertexIndex))
		{
			if(!alreadySel.contains(v))
			{
				alreadySel.insert(v, true);
				selLocs.append(AuPointAlpha(v, selSize));
			}
		}
		else
		{
			if(!alreadyUnsel.contains(v))
			{
				alreadyUnsel.insert(v, true);
				unselLocs.append(AuPointAlpha(v, unselSize));
			}
		}
		vertexIndex++;
	}
}

void AuScScreen::Transform(const AuMatrix4x4& t)
{
	m_Face.Transform(t);
}

void AuScScreen::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	m_Face.Transform(t, 0, selInfo.m_Vertices, 0, selInfo.m_Edges, selInfo.m_Faces.contains(0));
}

void AuScScreen::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	bool faceSelected = false;
	if(selInfo.m_Faces.contains(0))
		faceSelected = true;

	if(!faceSelected)
	{
		for(int v=0; v<m_Face.VertexCount(); v++)
		{
			if(selInfo.m_Vertices.contains(v))
			{
				faceSelected = true;
				break;
			}
		}
	}

	if(!faceSelected)
	{
		for(int v=0; v<m_Face.Edges(); v++)
		{
			if(selInfo.m_Edges.contains(v))
			{
				faceSelected = true;
				break;
			}
		}
	}

	if(faceSelected)
	{
		// Face is affected by the transform.
		m_Face.Transform(t, 0, selInfo.m_Vertices, 0, selInfo.m_Edges, faceSelected);
	}
}

double AuScScreen::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint edgeLoc;
	quint32 faceEdgeIndex;
	double dseg = m_Face.ViewEdgeDistance(mvp, pixel, faceEdgeIndex, edgeLoc);
	if(dseg >= d_SearchTolerance)
		return d_SearchTolerance;

	if(snapLoc)
		*snapLoc = edgeLoc;
	return dseg;
}

double AuScScreen::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint edgeLoc;
	quint32 faceEdgeIndex;
	double dseg = m_Face.ViewEdgeDistance(mvp, pixel, faceEdgeIndex, edgeLoc);
	if(dseg >= d_SearchTolerance)
		return d_SearchTolerance;

	m_Face.EdgeSegment(faceEdgeIndex, edgeSeg);
	return dseg;
}

double AuScScreen::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint v = m_Face.FaceLocation();
	AuPoint floc = mvp.Project(v);
	if(floc.z < 0)
		return d_SearchTolerance;	// Behind eye plane.

	AuPoint fpx(pixel);
	double dist2 = floc.Distance2_2D(fpx);
	if(dist2 < d_SearchTolerance*d_SearchTolerance)
	{
		faceIndex = 0;
		gripLoc = v;
		return sqrt(dist2);
	}
	else
		return d_SearchTolerance;

}

double AuScScreen::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint fpx(pixel);
	double closestDist2 = d_SearchTolerance*d_SearchTolerance;	// We won't find anything that is not closer than this dist.
	bool foundMe = false;

	quint32 vindex = 0;
	QMap<AuPoint, bool> alreadyChecked;
	AuPointArray verts;
	m_Face.VertexLocations(verts);
	for(AuPointArray::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
	{
		if(!alreadyChecked.contains(*vi))
		{
			alreadyChecked.insert(*vi, true);

			AuPoint vloc = mvp.Project(*vi);
			if(vloc.z < 0)
				continue;		// Behind eye plane.

			double dist2 = vloc.Distance2_2D(fpx);
			if(dist2 < closestDist2)
			{
				closestDist2 = dist2;
				vertexIndex = vindex;
				if(snapLoc)
					*snapLoc = *vi;
				foundMe = true;
			}
		}
		vindex++;
	}

	if(!foundMe)
		return d_SearchTolerance;

	return sqrt(closestDist2);
}

bool AuScScreen::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(angleTol);
	Q_UNUSED(selInfo);
	if(IsHidden())
		return false;
	return false;	// There is only one face. It is presumed to already be selected so there could not be a change.
}

bool AuScScreen::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	AuSegmentArray segs;
	m_Face.EdgeSegments(segs);

	quint32 eindex = 0;
	for(AuSegmentArray::const_iterator si = segs.cbegin(); si != segs.cend(); si++)
	{
		if(!selInfo.m_Edges.contains(eindex) && (*si).IsSegment(edgeSeg))
		{
			selInfo.m_Edges.insert(eindex);
			anyChange = true;
		}
		eindex++;
	}

	return anyChange;
}

bool AuScScreen::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	quint32 findex = 0;
	AuPoint v = m_Face.FaceLocation();
	if(v.FuzzyEquals(atLoc) && !selInfo.m_Faces.contains(findex))
	{
		selInfo.m_Faces.insert(findex);
		return true;
	}

	return false;
}

bool AuScScreen::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return SelectFaceAt(atLoc, selInfo);
}

bool AuScScreen::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	quint32 vindex = 0;
	int verts = m_Face.VertexCount();
	for(int vi=0; vi<verts; vi++)
	{
		AuPoint v = m_Face.Vertex(vi);
		if(v.FuzzyEquals(atLoc) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.insert(vindex);
			anyChange = true;
		}
		vindex++;
	}

	return anyChange;
}

bool AuScScreen::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	quint32 vindex = 0;
	int verts = m_Face.VertexCount();
	for(int vi=0; vi<verts; vi++)
	{
		AuScPoint v = m_Face.Vertex(vi);
		if(FuzzyEquals(v.EffectTx(), (float)texLoc.x()) && FuzzyEquals(v.EffectTy(), (float)texLoc.y()) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.remove(vindex);
			anyChange = true;
		}
		vindex++;
	}

	return anyChange;
}

bool AuScScreen::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	quint32 vindex = 0;
	int verts = m_Face.VertexCount();
	for(int vi=0; vi<verts; vi++)
	{
		AuScPoint v = m_Face.Vertex(vi);
		if(FuzzyEquals(v.EffectTx(), (float)texLoc.x()) && FuzzyEquals(v.EffectTy(), (float)texLoc.y()) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.insert(vindex);
			anyChange = true;
		}
		vindex++;
	}

	return anyChange;
}

bool AuScScreen::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	quint32 vindex = 0;
	int verts = m_Face.VertexCount();
	for(int vi=0; vi<verts; vi++)
	{
		AuScPoint v = m_Face.Vertex(vi);
		if(FuzzyEquals(v.m_Tx, (float)texLoc.x()) && FuzzyEquals(v.m_Ty, (float)texLoc.y()) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.remove(vindex);
			anyChange = true;
		}
		vindex++;
	}

	return anyChange;
}

bool AuScScreen::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	quint32 vindex = 0;
	int verts = m_Face.VertexCount();
	for(int vi=0; vi<verts; vi++)
	{
		AuScPoint v = m_Face.Vertex(vi);
		if(FuzzyEquals(v.m_Tx, (float)texLoc.x()) && FuzzyEquals(v.m_Ty, (float)texLoc.y()) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.insert(vindex);
			anyChange = true;
		}
		vindex++;
	}

	return anyChange;
}

bool AuScScreen::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	AuSegmentArray segs;
	m_Face.EdgeSegments(segs);

	quint32 eindex = 0;
	for(AuSegmentArray::const_iterator si = segs.cbegin(); si != segs.cend(); si++)
	{
		if(selInfo.m_Edges.contains(eindex) && (*si).IsSegment(edgeSeg))
		{
			selInfo.m_Edges.remove(eindex);
			anyChange = true;
		}
		eindex++;
	}

	return anyChange;
}

bool AuScScreen::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Remove indexes of all overlapping vertices.
	quint32 findex = 0;
	AuPoint v = m_Face.FaceLocation();
	if(v.FuzzyEquals(atLoc) && selInfo.m_Faces.contains(findex))
	{
		selInfo.m_Faces.remove(findex);
		anyChange = true;
	}
	findex++;

	return anyChange;
}

bool AuScScreen::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	// Remove indexes of all overlapping vertices.
	quint32 vindex = 0;
	int verts = m_Face.VertexCount();
	for(int vi=0; vi<verts; vi++)
	{
		AuPoint v = m_Face.Vertex(vi);
		if(v.FuzzyEquals(atLoc) && selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.remove(vindex);
			anyChange = true;
		}
		vindex++;
	}

	return anyChange;
}

QString AuScScreen::SelectedDesc(const AuScParts& parts) const
{
	return QObject::tr("%1")
		.arg(FullName(parts));
}

void AuScScreen::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	m_Face.RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScScreen::Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
{
	if(selInfo.m_Faces.contains(0))
		faces.AddFace(m_Face);
}

void AuScScreen::Faces(AuScFaceList& faces) const
{
	faces.AddFace(m_Face);
}

void AuScScreen::AddFaces(const AuScFaceList& faces)
{
	Q_UNUSED(faces);
}

void AuScScreen::SetFaces(const AuScFaceList& faces)
{
	const AuScFaceQuad* q = faces.FindQuad();
	if(q)
		m_Face = *q;
}

void AuScScreen::DeleteFaces(const QSet<quint32>& faceIds)
{
	Q_UNUSED(faceIds);	// No.
}

void AuScScreen::ReverseVertexNormals()
{
	m_Face.ReverseVertexNormals();
}

void AuScScreen::ReverseFaces()
{
	m_Face.ReverseTriangleFacing();
	m_Face.ReverseVertexNormals();
}

void AuScScreen::ReverseFaces(const AuScSelectInfo& selInfo)
{
	quint32 findex = 0;
	if(selInfo.m_Faces.contains(findex))
	{
		m_Face.ReverseTriangleFacing();
		m_Face.ReverseVertexNormals();
	}
}

void AuScScreen::FlattenVertices()
{
	m_Face.FlattenVertexNormals();
}

void AuScScreen::FlattenVertices(const AuScSelectInfo& selInfo)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	int vcount = m_Face.VertexCount();
	if(selInfo.m_Faces.contains(findex))
	{
		AuPoint faceN = m_Face.FaceNormal();
		for(int vi=0; vi<vcount; vi++)
			if(selInfo.m_Vertices.contains(vindex + vi))
				m_Face.SetVertexNormal(vi, faceN);
	}
}

void AuScScreen::SmoothVertices()
{
	// A single flat quad cannot be smoothed.
	FlattenVertices();
}

void AuScScreen::SmoothVertices(const AuScSelectInfo& selInfo)
{
	// A single flat quad cannot be smoothed.
	FlattenVertices(selInfo);
}

void AuScScreen::MakeScreen(const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane)
{
	AuPoint cloc1 = gridPlane.UnProject(corner1);
	AuPoint cloc2 = gridPlane.UnProject(corner2);
	cloc1.z = 0;
	cloc2.z = 0;

	AuPoint ll(cloc1.Min(cloc2));
	AuPoint ur(cloc1.Max(cloc2));
	AuPoint ul(ll.x, ur.y, 0);
	AuPoint lr(ur.x, ll.y, 0);

	ll = gridPlane.Project(ll);
	ul = gridPlane.Project(ul);
	ur = gridPlane.Project(ur);
	lr = gridPlane.Project(lr);
	AuPoint n = gridPlane.Project(AuPoint(0,0,1));

	AuScPoint sll(ll, 0,0, n);
	AuScPoint sul(ul, 0,1, n);
	AuScPoint sur(ur, 1,1, n);
	AuScPoint slr(lr, 1,0, n);

	m_Face = AuScFaceQuad(sll, sul, sur, slr);
}

void AuScScreen::SetFaceSelfIllumination(float selfIllum)
{
	m_Face.SetSelfIllumination(selfIllum);
}

void AuScScreen::SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum)
{
	quint32 findex = 0;
	if(selInfo.m_Faces.contains(findex))
		m_Face.SetSelfIllumination(selfIllum);
}

void AuScScreen::SetFaceOpacity(float opacity)
{
	m_Face.SetOpacity(opacity);
}

void AuScScreen::SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	quint32 findex = 0;
	if(selInfo.m_Faces.contains(findex))
		m_Face.SetOpacity(opacity);
}

void AuScScreen::EffectFaces(SURFACE_EFFECT effect)
{
	Q_UNUSED(effect);
}

void AuScScreen::EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
}

void AuScScreen::EffectFacesCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScScreen::EffectFacesProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScScreen::EffectFacesSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(gridPlane);
}

void AuScScreen::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texSize);
	Q_UNUSED(unwrap);
	Q_UNUSED(angleTol);
	Q_UNUSED(gridAlign);
	Q_UNUSED(alignAxis);
	Q_UNUSED(gridPlane);
}

SURFACE_EFFECT AuScScreen::Effect(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return EFF_Count;
}

void AuScScreen::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(vec);
}

void AuScScreen::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(pivot);
	Q_UNUSED(angle);
}

void AuScScreen::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(base);
	Q_UNUSED(scaleX);
	Q_UNUSED(scaleY);
}

void AuScScreen::TextureFaces(SC_TEXTURE_INDEX texIndex)
{
	Q_UNUSED(texIndex);
}

void AuScScreen::TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
}

void AuScScreen::TextureFacesCylindrical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScScreen::TextureFacesProject(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScScreen::TextureFacesSpherical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(gridPlane);
}

void AuScScreen::TextureUnwrap(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texSize);
	Q_UNUSED(unwrap);
	Q_UNUSED(angleTol);
	Q_UNUSED(gridAlign);
	Q_UNUSED(alignAxis);
	Q_UNUSED(gridPlane);
}

void AuScScreen::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(vec);
}

void AuScScreen::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(pivot);
	Q_UNUSED(angle);
}

void AuScScreen::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(base);
	Q_UNUSED(scaleX);
	Q_UNUSED(scaleY);
}

SC_TEXTURE_INDEX AuScScreen::TextureIndex(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return SCTEXI_Count;
}

GLuint AuScScreen::OwnTexture(AuGLEffects& re) const
{
	QString texName(TypeIcon());
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScScreen::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Face.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, SLINE_None);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int plastic = (int)surface / d_HullPlatesPerUnit;
	if(plastic < 1)
		plastic = 1;	// Minimum 1 metal per screen.
	pc.AddRequirement(CID_Plastic, CE_Required, plastic);

	// Add electronic parts.
	int elec = plastic / 10;
	if(!elec)
		elec = 1;
	pc.AddRequirement(CID_ElectronicPart, CE_Required, elec);
}

bool AuScScreen::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	if(!m_Face.Sight(eyeLoc, sightLoc))
		return false;
	sightSurfaceNormal = m_Face.FaceNormal();
	return true;
}

AuSysId AuScScreen::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Hull;
}

AuPoint AuScScreen::Center() const
{
	return m_Face.FaceLocation();
}

bool AuScScreen::ScreenQuad(AuQuad& screen) const
{
	screen = AuQuad(m_Face.Vertex(0), m_Face.Vertex(1), m_Face.Vertex(2), m_Face.Vertex(3));
	return true;
}

bool AuScScreen::ModelExtents(AuPoint& min, AuPoint& max) const
{
	m_Face.Extents(min, max);
	return true;
}

quint64 AuScScreen::MemoryUsage() const
{
	quint64 mem = AuScPart::MemoryUsage();
	mem += m_Face.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStatusAlert::AuScStatusAlert()
{
}

AuScStatusAlert::AuScStatusAlert(const AuScStatusAlert& c) : AuScScreen(c)
{
}

AuScStatusAlert::AuScStatusAlert(const AuScStatusAlert& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScStatusAlert::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScStatusAlert::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScStatusAlert::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStatusAlert& c = dynamic_cast<const AuScStatusAlert&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStatusAlert::Clone() const
{
	return new AuScStatusAlert(*this);
}

AuScPart* AuScStatusAlert::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStatusAlert(*this, selInfo);
}

QString AuScStatusAlert::TypeIcon() const
{
	return QString(":data/ScStatusAlert.png");
}

QString AuScStatusAlert::TypeName() const
{
	return QObject::tr("Alert Status");
}

void AuScStatusAlert::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStatusLifeSupport::AuScStatusLifeSupport()
{
	m_PartId = 0;
}

AuScStatusLifeSupport::AuScStatusLifeSupport(const AuScStatusLifeSupport& c) : AuScScreen(c)
{
	m_PartId = c.m_PartId;
}

AuScStatusLifeSupport::AuScStatusLifeSupport(const AuScStatusLifeSupport& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
	m_PartId = c.m_PartId;
}

void AuScStatusLifeSupport::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
	out << m_PartId;
}

void AuScStatusLifeSupport::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
	if(ReadVersion() >= 0x0a)
		in >> m_PartId;
}

bool AuScStatusLifeSupport::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStatusLifeSupport& c = dynamic_cast<const AuScStatusLifeSupport&>(p);
		if(!AuScScreen::operator==(c))
			return false;
		if(m_PartId != c.m_PartId)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStatusLifeSupport::Clone() const
{
	return new AuScStatusLifeSupport(*this);
}

AuScPart* AuScStatusLifeSupport::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStatusLifeSupport(*this, selInfo);
}

QString AuScStatusLifeSupport::TypeIcon() const
{
	return QString(":data/ScStatusLifeSupport.png");
}

QString AuScStatusLifeSupport::TypeName() const
{
	return QObject::tr("Room Life Support Status");
}

GLuint AuScStatusLifeSupport::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/LifeSupportStatus.png");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScStatusLifeSupport::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
}

quint64 AuScStatusLifeSupport::MemoryUsage() const
{
	quint64 mem = AuScScreen::MemoryUsage();
	mem += sizeof(m_PartId);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStatusMission::AuScStatusMission()
{
}

AuScStatusMission::AuScStatusMission(const AuScStatusMission& c) : AuScScreen(c)
{
}

AuScStatusMission::AuScStatusMission(const AuScStatusMission& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScStatusMission::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScStatusMission::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScStatusMission::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStatusMission& c = dynamic_cast<const AuScStatusMission&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStatusMission::Clone() const
{
	return new AuScStatusMission(*this);
}

AuScPart* AuScStatusMission::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStatusMission(*this, selInfo);
}

QString AuScStatusMission::TypeIcon() const
{
	return QString(":data/ScStatusMission.png");
}

QString AuScStatusMission::TypeName() const
{
	return QObject::tr("Mission Status");
}

void AuScStatusMission::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelCapacitor::AuScPanelCapacitor()
{
}

AuScPanelCapacitor::AuScPanelCapacitor(const AuScPanelCapacitor& c) : AuScScreen(c)
{
}

AuScPanelCapacitor::AuScPanelCapacitor(const AuScPanelCapacitor& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScPanelCapacitor::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScPanelCapacitor::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScPanelCapacitor::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelCapacitor& c = dynamic_cast<const AuScPanelCapacitor&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelCapacitor::Clone() const
{
	return new AuScPanelCapacitor(*this);
}

AuScPart* AuScPanelCapacitor::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelCapacitor(*this, selInfo);
}

QString AuScPanelCapacitor::TypeIcon() const
{
	return QString(":data/AccessCapacitor.png");
}

QString AuScPanelCapacitor::TypeName() const
{
	return QObject::tr("Capacitor Service Panel");
}

AuSysId AuScPanelCapacitor::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Capacitor;
}

void AuScPanelCapacitor::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelFTLDrive::AuScPanelFTLDrive()
{
	m_PartId = 0;
}

AuScPanelFTLDrive::AuScPanelFTLDrive(const AuScPanelFTLDrive& c) : AuScScreen(c)
{
	m_PartId = c.m_PartId;
}

AuScPanelFTLDrive::AuScPanelFTLDrive(const AuScPanelFTLDrive& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
	m_PartId = c.m_PartId;
}

void AuScPanelFTLDrive::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
	out << m_PartId;
}

void AuScPanelFTLDrive::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
	if(ReadVersion() >= 0x05)
		in >> m_PartId;
	else
		m_PartId = 0;
}

bool AuScPanelFTLDrive::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelFTLDrive& c = dynamic_cast<const AuScPanelFTLDrive&>(p);
		if(!AuScScreen::operator==(c))
			return false;
		if(m_PartId != c.m_PartId)
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelFTLDrive::Clone() const
{
	return new AuScPanelFTLDrive(*this);
}

AuScPart* AuScPanelFTLDrive::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelFTLDrive(*this, selInfo);
}

QString AuScPanelFTLDrive::TypeIcon() const
{
	return QString(":data/AccessFTLDrive.png");
}

QString AuScPanelFTLDrive::TypeName() const
{
	return QObject::tr("FTL Drive Service Panel");
}

AuSysId AuScPanelFTLDrive::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return AuSysId(SYS_FTLDrive, m_PartId);
}

void AuScPanelFTLDrive::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

quint64 AuScPanelFTLDrive::MemoryUsage() const
{
	quint64 mem = AuScScreen::MemoryUsage();
	mem += sizeof(m_PartId);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelLifeSupport::AuScPanelLifeSupport()
{
}

AuScPanelLifeSupport::AuScPanelLifeSupport(const AuScPanelLifeSupport& c) : AuScScreen(c)
{
}

AuScPanelLifeSupport::AuScPanelLifeSupport(const AuScPanelLifeSupport& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScPanelLifeSupport::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScPanelLifeSupport::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScPanelLifeSupport::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelLifeSupport& c = dynamic_cast<const AuScPanelLifeSupport&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelLifeSupport::Clone() const
{
	return new AuScPanelLifeSupport(*this);
}

AuScPart* AuScPanelLifeSupport::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelLifeSupport(*this, selInfo);
}

QString AuScPanelLifeSupport::TypeIcon() const
{
	return QString(":data/AccessLifeSupport.png");
}

QString AuScPanelLifeSupport::TypeName() const
{
	return QObject::tr("Life Support Service Panel");
}

AuSysId AuScPanelLifeSupport::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_LifeSupport;
}

void AuScPanelLifeSupport::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelManeuverDrive::AuScPanelManeuverDrive()
{
}

AuScPanelManeuverDrive::AuScPanelManeuverDrive(const AuScPanelManeuverDrive& c) : AuScScreen(c)
{
}

AuScPanelManeuverDrive::AuScPanelManeuverDrive(const AuScPanelManeuverDrive& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScPanelManeuverDrive::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScPanelManeuverDrive::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScPanelManeuverDrive::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelManeuverDrive& c = dynamic_cast<const AuScPanelManeuverDrive&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelManeuverDrive::Clone() const
{
	return new AuScPanelManeuverDrive(*this);
}

AuScPart* AuScPanelManeuverDrive::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelManeuverDrive(*this, selInfo);
}

QString AuScPanelManeuverDrive::TypeIcon() const
{
	return QString(":data/AccessManeuverDrive.png");
}

QString AuScPanelManeuverDrive::TypeName() const
{
	return QObject::tr("Maneuver Drive Service Panel");
}

AuSysId AuScPanelManeuverDrive::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_ManeuverDrive;
}

void AuScPanelManeuverDrive::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelPowerPlant::AuScPanelPowerPlant()
{
}

AuScPanelPowerPlant::AuScPanelPowerPlant(const AuScPanelPowerPlant& c) : AuScScreen(c)
{
}

AuScPanelPowerPlant::AuScPanelPowerPlant(const AuScPanelPowerPlant& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScPanelPowerPlant::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScPanelPowerPlant::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScPanelPowerPlant::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelPowerPlant& c = dynamic_cast<const AuScPanelPowerPlant&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelPowerPlant::Clone() const
{
	return new AuScPanelPowerPlant(*this);
}

AuScPart* AuScPanelPowerPlant::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelPowerPlant(*this, selInfo);
}

QString AuScPanelPowerPlant::TypeIcon() const
{
	return QString(":data/AccessPowerPlant.png");
}

QString AuScPanelPowerPlant::TypeName() const
{
	return QObject::tr("Power Plant Service Panel");
}

AuSysId AuScPanelPowerPlant::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_PowerPlant;
}

void AuScPanelPowerPlant::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelSensor::AuScPanelSensor()
{
}

AuScPanelSensor::AuScPanelSensor(const AuScPanelSensor& c) : AuScScreen(c)
{
}

AuScPanelSensor::AuScPanelSensor(const AuScPanelSensor& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScPanelSensor::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScPanelSensor::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScPanelSensor::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelSensor& c = dynamic_cast<const AuScPanelSensor&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelSensor::Clone() const
{
	return new AuScPanelSensor(*this);
}

AuScPart* AuScPanelSensor::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelSensor(*this, selInfo);
}

QString AuScPanelSensor::TypeIcon() const
{
	return QString(":data/AccessSensor.png");
}

QString AuScPanelSensor::TypeName() const
{
	return QObject::tr("Sensor Service Panel");
}

AuSysId AuScPanelSensor::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Sensor;
}

void AuScPanelSensor::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelShield::AuScPanelShield()
{
}

AuScPanelShield::AuScPanelShield(const AuScPanelShield& c) : AuScScreen(c)
{
}

AuScPanelShield::AuScPanelShield(const AuScPanelShield& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScPanelShield::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScPanelShield::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScPanelShield::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelShield& c = dynamic_cast<const AuScPanelShield&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelShield::Clone() const
{
	return new AuScPanelShield(*this);
}

AuScPart* AuScPanelShield::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelShield(*this, selInfo);
}

QString AuScPanelShield::TypeIcon() const
{
	return QString(":data/AccessShield.png");
}

QString AuScPanelShield::TypeName() const
{
	return QObject::tr("Shield Service Panel");
}

AuSysId AuScPanelShield::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Shield;
}

void AuScPanelShield::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPanelWeapon::AuScPanelWeapon()
{
	m_PartId = 0;
}

AuScPanelWeapon::AuScPanelWeapon(const AuScPanelWeapon& c) : AuScScreen(c)
{
	m_PartId = c.m_PartId;
}

AuScPanelWeapon::AuScPanelWeapon(const AuScPanelWeapon& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
	m_PartId = c.m_PartId;
}

void AuScPanelWeapon::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
	out << m_PartId;
}

void AuScPanelWeapon::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
	if(ReadVersion() >= 0x05)
		in >> m_PartId;
	else
		m_PartId = 0;
}

bool AuScPanelWeapon::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPanelWeapon& c = dynamic_cast<const AuScPanelWeapon&>(p);
		if(!AuScScreen::operator==(c))
			return false;
		if(m_PartId != c.m_PartId)
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPanelWeapon::Clone() const
{
	return new AuScPanelWeapon(*this);
}

AuScPart* AuScPanelWeapon::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPanelWeapon(*this, selInfo);
}

QString AuScPanelWeapon::TypeIcon() const
{
	return QString(":data/AccessWeapon.png");
}

QString AuScPanelWeapon::TypeName() const
{
	return QObject::tr("Weapon Service Panel");
}

AuSysId AuScPanelWeapon::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return AuSysId(SYS_Weapon, m_PartId);
}

void AuScPanelWeapon::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

quint64 AuScPanelWeapon::MemoryUsage() const
{
	quint64 mem = AuScScreen::MemoryUsage();
	mem += sizeof(m_PartId);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStation::AuScStation()
{
	m_Module = CID_Invalid;
	m_Position = Operator_Sit;
}

AuScStation::AuScStation(const AuScStation& c) : AuScScreen(c)
{
	m_Location = c.m_Location;
	m_Module = c.m_Module;
	m_OperatorFacing = c.m_OperatorFacing;
	m_Position = c.m_Position;
}

AuScStation::AuScStation(const AuScStation& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
	m_Location = c.m_Location;
	m_Module = c.m_Module;
	m_OperatorFacing = c.m_OperatorFacing;
	m_Position = c.m_Position;
}

void AuScStation::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);

	quint8 hasWhat = m_Position;
	if(m_OperatorFacing != m_Location)
		hasWhat |= 0x80;

	out << hasWhat;
	out << m_Location;
	out << m_Module;
	if(hasWhat & 0x80)
		out << m_OperatorFacing;
}

void AuScStation::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;

	m_Position = hasWhat & 0x7f;
	in >> m_Location;

	if(ReadVersion() >= 0x01)
		in >> m_Module;
	else
		m_Module = CID_Invalid;

	if(hasWhat & 0x80)
		in >> m_OperatorFacing;
	else
		m_OperatorFacing = m_Location;
}

bool AuScStation::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStation& c = dynamic_cast<const AuScStation&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		if(m_Module != c.m_Module)
			return false;
		if(m_Position != c.m_Position)
			return false;
		if(m_Location != c.m_Location)
			return false;
		if(m_OperatorFacing != c.m_OperatorFacing)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuScStation::Transform(const AuMatrix4x4& t)
{
	AuScScreen::Transform(t);
	m_Location = t.Project(m_Location);
	m_OperatorFacing = t.Project(m_OperatorFacing);
}

void AuScStation::MakeStation(OPERATOR_POSITION pos, const AuPoint& loc, const AuPoint& opFacing, const AuPoint& corner1,
		const AuPoint& corner2, const AuMatrix4x4& gridPlane)
{
	AuScScreen::MakeScreen(corner1, corner2, gridPlane);
	m_Location = loc;
	m_OperatorFacing = opFacing;
	m_Position =  pos;
}

void AuScStation::EdgeSegments(AuPointArray& edges) const
{
	AuScScreen::EdgeSegments(edges);

	// Operator seat circle.
	AuPoint seatAxis(0,0,1);
	AuPoint vx;
	seatAxis.ArbitraryAxis(vx);
	AuPoint vy(vx.Cross(seatAxis));
	double grad = 1;
	AuMatrix4x4 gt(m_Location, vx, vy);
	AuPoint p(gt.Project(grad, 0, 0));
	for(int v=1; v<=8; v++)
	{
		float ang = v*PI*2/8.;
		AuPoint n(gt.Project(AuPoint(grad,0).RotateZ(ang)));
		edges.append(p);
		edges.append(n);
		p = n;
	}

	if(m_Location != m_OperatorFacing)
	{
		edges.append(m_Location);
		edges.append(m_OperatorFacing);
	}
}

QString AuScStation::ReportDesc() const
{
	if(Module() == CID_Invalid)
		return PartName();

	return QObject::tr("%1, %2")
		.arg(PartName())
		.arg(g_Au.CommodityName(Module()));
}

QString AuScStation::SelectedDesc(const AuScParts& parts) const
{
	if(Module() != CID_Invalid)
		return QObject::tr("%1, %2")
			.arg(FullName(parts))
			.arg(g_Au.CommodityName(Module()));
	else
		return QObject::tr("%1")
			.arg(FullName(parts));
}

void AuScStation::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	// Screen adds 1 electronic part and 1 plastic.
	AuScScreen::ListComponents(pc, streamline, armor, hullMaterial);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial == CID_Lumber)
	{
		pc.AddRequirement(CID_Computer, CE_Required, 1);
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 1);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 1);
		pc.AddRequirement(CID_Textiles, CE_Required, 1);
	}
	else
	{
		if(Module() != CID_Invalid)
			pc.AddRequirement(Module(), CE_Required, 1);
	}
}

AuSysId AuScStation::SightedSystem(quint16 partId) const
{
	return AuSysId(SYS_CommandAndControl, partId);
}

AuStation AuScStation::OperatorStation(const AuPoint& gravityUp) const
{
	AuPoint o(OperatorLoc());
	AuPoint y = m_Face.FaceLocation() - o;						// Calculate y to face the screen from the operator loc.
	y.Unit();
	AuPoint x = y.Cross(gravityUp);
	x.Unit();
	y = gravityUp.Cross(x);										// Adjust y to be perpendicular to the gravity up vector.
	y.Unit();
	AuMatrix4x4 opM;
	opM.Translate(o);
	opM.RotateToPlane(x, y);

	if(m_OperatorFacing != o)
	{
		// Calculate facing rotation.
		AuPoint fvec(m_OperatorFacing - o);
		fvec.Unit();
		double fx = fvec.Dot(x);
		double fy = fvec.Dot(y);
		double zang = atan2(fy, fx);
		double zrot = zang - PI/2;
		opM.RotateZ(zrot);
	}

	AuStation::OCCUPANT_POSITION occPos;
	switch(OperatorPos())
	{
#if defined(QT_NO_DEBUG)
	default:
#endif
	case Operator_Sit:
		occPos = AuStation::Occupant_Sit;
		break;
	case Operator_SitSwivel3D:
		occPos = AuStation::Occupant_SitSwivel3D;
		break;
	case Operator_SitStationary:
		occPos = AuStation::Occupant_SitStationary;
		break;
	case Operator_Stand:
		occPos = AuStation::Occupant_Stand;
		break;
	case Operator_StandStationary:
		occPos = AuStation::Occupant_StandStationary;
		break;
	}
	return AuStation(StationType(), opM, occPos, 0);
}

void AuScStation::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

quint64 AuScStation::MemoryUsage() const
{
	quint64 mem = AuScScreen::MemoryUsage();
	mem += sizeof(m_Location);
	mem += sizeof(m_Module);
	mem += sizeof(m_OperatorFacing);
	mem += sizeof(m_Position);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationCommand::AuScStationCommand()
{
}

AuScStationCommand::AuScStationCommand(const AuScStationCommand& c) : AuScStation(c)
{
}

AuScStationCommand::AuScStationCommand(const AuScStationCommand& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
}

void AuScStationCommand::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
}

void AuScStationCommand::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
}

bool AuScStationCommand::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationCommand& c = dynamic_cast<const AuScStationCommand&>(p);
		if(!AuScStation::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationCommand::Clone() const
{
	return new AuScStationCommand(*this);
}

AuScPart* AuScStationCommand::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationCommand(*this, selInfo);
}

QString AuScStationCommand::TypeIcon() const
{
	return QString(":data/dp_CmdConsole.png");
}

QString AuScStationCommand::TypeName() const
{
	return QObject::tr("Command Station");
}

GLuint AuScStationCommand::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txCnCConsole.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationDesigner::AuScStationDesigner()
{
}

AuScStationDesigner::AuScStationDesigner(const AuScStationDesigner& c) : AuScStation(c)
{
}

AuScStationDesigner::AuScStationDesigner(const AuScStationDesigner& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
}

void AuScStationDesigner::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
}

void AuScStationDesigner::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
}

bool AuScStationDesigner::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationDesigner& c = dynamic_cast<const AuScStationDesigner&>(p);
		if(!AuScStation::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationDesigner::Clone() const
{
	return new AuScStationDesigner(*this);
}

AuScPart* AuScStationDesigner::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationDesigner(*this, selInfo);
}

QString AuScStationDesigner::TypeIcon() const
{
	return QString(":data/dp_DesignerConsole.png");
}

QString AuScStationDesigner::TypeName() const
{
	return QObject::tr("Designer Station");
}

GLuint AuScStationDesigner::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txDesignerConsole.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationEngineer::AuScStationEngineer()
{
	m_WhichEffect = 0;
}

AuScStationEngineer::AuScStationEngineer(const AuScStationEngineer& c) : AuScStation(c)
{
	m_WhichEffect = c.m_WhichEffect;
}

AuScStationEngineer::AuScStationEngineer(const AuScStationEngineer& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
	m_WhichEffect = c.m_WhichEffect;
}

void AuScStationEngineer::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
	out << m_WhichEffect;
}

void AuScStationEngineer::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
	if(ReadVersion() >= 0x0b)
		in >> m_WhichEffect;
}

bool AuScStationEngineer::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationEngineer& c = dynamic_cast<const AuScStationEngineer&>(p);
		if(!AuScStation::operator==(c))
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationEngineer::Clone() const
{
	return new AuScStationEngineer(*this);
}

AuScPart* AuScStationEngineer::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationEngineer(*this, selInfo);
}

QString AuScStationEngineer::TypeIcon() const
{
	return QString(":data/dp_EngineerConsole.png");
}

QString AuScStationEngineer::TypeName() const
{
	return QObject::tr("Engineer Station");
}

GLuint AuScStationEngineer::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/ec_Console.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

QString AuScStationEngineer::PartName() const
{
	return QObject::tr("%1 %2").arg(TypeName()).arg(m_WhichEffect+1);
}

quint64 AuScStationEngineer::MemoryUsage() const
{
	quint64 mem = AuScStation::MemoryUsage();
	mem += sizeof(m_WhichEffect);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationFireControl::AuScStationFireControl()
{
	m_WhichEffect = 0;
}

AuScStationFireControl::AuScStationFireControl(const AuScStationFireControl& c) : AuScStation(c)
{
	m_EmitLoc = c.m_EmitLoc;
	m_WhichEffect = c.m_WhichEffect;
}

AuScStationFireControl::AuScStationFireControl(const AuScStationFireControl& c, const AuScSelectInfo& selInfo) :
	AuScStation(c, selInfo)
{
	m_EmitLoc = c.m_EmitLoc;
	m_WhichEffect = c.m_WhichEffect;
}

void AuScStationFireControl::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
	out << m_EmitLoc;
	out << m_WhichEffect;
}

void AuScStationFireControl::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
	in >> m_EmitLoc;
	if(ReadVersion() >= 0x0b)
		in >> m_WhichEffect;
}

bool AuScStationFireControl::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationFireControl& c = dynamic_cast<const AuScStationFireControl&>(p);
		if(!AuScStation::operator==(c))
			return false;
		if(m_EmitLoc != c.m_EmitLoc)
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationFireControl::Clone() const
{
	return new AuScStationFireControl(*this);
}

AuScPart* AuScStationFireControl::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationFireControl(*this, selInfo);
}

QString AuScStationFireControl::TypeIcon() const
{
	return QString(":data/dp_FireControlConsole.png");
}

QString AuScStationFireControl::TypeName() const
{
	return QObject::tr("Fire Control Station");
}

GLuint AuScStationFireControl::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txConsoleWeapon.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScStationFireControl::EdgeSegments(AuPointArray& edges) const
{
	AuScStation::EdgeSegments(edges);

	double irad = FeetPerMeter()*.5;
	double orad = 3 * FeetPerMeter();

	edges.append(m_EmitLoc + AuPoint(irad,irad));
	edges.append(m_EmitLoc + AuPoint(orad,0));

	edges.append(m_EmitLoc + AuPoint(orad,0));
	edges.append(m_EmitLoc + AuPoint(irad,-irad));

	edges.append(m_EmitLoc + AuPoint(irad,-irad));
	edges.append(m_EmitLoc + AuPoint(0,-orad));

	edges.append(m_EmitLoc + AuPoint(0,-orad));
	edges.append(m_EmitLoc + AuPoint(-irad,-irad));

	edges.append(m_EmitLoc + AuPoint(-irad,-irad));
	edges.append(m_EmitLoc + AuPoint(-orad,0));

	edges.append(m_EmitLoc + AuPoint(-orad,0));
	edges.append(m_EmitLoc + AuPoint(-irad,irad));

	edges.append(m_EmitLoc + AuPoint(-irad,irad));
	edges.append(m_EmitLoc + AuPoint(0,orad));

	edges.append(m_EmitLoc + AuPoint(0,orad));
	edges.append(m_EmitLoc + AuPoint(irad,irad));
}

void AuScStationFireControl::Transform(const AuMatrix4x4& t)
{
	AuScStation::Transform(t);
	m_EmitLoc = t.Project(m_EmitLoc);
}

QString AuScStationFireControl::PartName() const
{
	return QObject::tr("%1 %2").arg(TypeName()).arg(m_WhichEffect+1);
}

quint64 AuScStationFireControl::MemoryUsage() const
{
	quint64 mem = AuScStation::MemoryUsage();
	mem += sizeof(m_EmitLoc);
	mem += sizeof(m_WhichEffect);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationHelm::AuScStationHelm()
{
}

AuScStationHelm::AuScStationHelm(const AuScStationHelm& c) : AuScStation(c)
{
}

AuScStationHelm::AuScStationHelm(const AuScStationHelm& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
}

void AuScStationHelm::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
}

void AuScStationHelm::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
}

bool AuScStationHelm::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationHelm& c = dynamic_cast<const AuScStationHelm&>(p);
		if(!AuScStation::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationHelm::Clone() const
{
	return new AuScStationHelm(*this);
}

AuScPart* AuScStationHelm::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationHelm(*this, selInfo);
}

QString AuScStationHelm::TypeIcon() const
{
	return QString(":data/dp_PilotConsole.png");
}

QString AuScStationHelm::TypeName() const
{
	return QObject::tr("Helm Station");
}

GLuint AuScStationHelm::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/pc_Console.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationMedic::AuScStationMedic()
{
	m_PatientPos = Patient_Stand;
}

AuScStationMedic::AuScStationMedic(const AuScStationMedic& c) : AuScStation(c)
{
	m_PatientFacing = c.m_PatientFacing;
	m_PatientLoc = c.m_PatientLoc;
	m_PatientPos = c.m_PatientPos;
}

AuScStationMedic::AuScStationMedic(const AuScStationMedic& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
	m_PatientFacing = c.m_PatientFacing;
	m_PatientLoc = c.m_PatientLoc;
	m_PatientPos = c.m_PatientPos;
}

void AuScStationMedic::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
	quint8 hasWhat = m_PatientPos;
	if(m_PatientFacing != m_PatientLoc)
		hasWhat |= 0x80;
	out << hasWhat;
	out << m_PatientLoc;
	if(hasWhat & 0x80)
		out << m_PatientFacing;
}

void AuScStationMedic::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	m_PatientPos = (PATIENT_POSITION)(hasWhat & 0x7f);
	in >> m_PatientLoc;
	if(hasWhat & 0x80)
		in >> m_PatientFacing;
	else
		m_PatientFacing = m_PatientLoc;
}

bool AuScStationMedic::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationMedic& c = dynamic_cast<const AuScStationMedic&>(p);
		if(!AuScStation::operator==(c))
			return false;

		if(m_PatientFacing != c.m_PatientFacing)
			return false;
		if(m_PatientLoc != c.m_PatientLoc)
			return false;
		if(m_PatientPos != c.m_PatientPos)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationMedic::Clone() const
{
	return new AuScStationMedic(*this);
}

AuScPart* AuScStationMedic::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationMedic(*this, selInfo);
}

QString AuScStationMedic::TypeIcon() const
{
	return QString(":data/dp_SurgeryConsole.png");
}

QString AuScStationMedic::TypeName() const
{
	return QObject::tr("Medic Station");
}

GLuint AuScStationMedic::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txConsoleSurgery.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScStationMedic::Transform(const AuMatrix4x4& t)
{
	AuScStation::Transform(t);
	m_PatientFacing = t.Project(m_PatientFacing);
	m_PatientLoc = t.Project(m_PatientLoc);
}

void AuScStationMedic::EdgeSegments(AuPointArray& edges) const
{
	AuScStation::EdgeSegments(edges);

	double irad = .5;
	double orad = 1;

	edges.append(m_PatientLoc + AuPoint(irad,irad));
	edges.append(m_PatientLoc + AuPoint(orad,irad));

	edges.append(m_PatientLoc + AuPoint(orad,irad));
	edges.append(m_PatientLoc + AuPoint(orad,-irad));

	edges.append(m_PatientLoc + AuPoint(orad,-irad));
	edges.append(m_PatientLoc + AuPoint(irad,-irad));

	edges.append(m_PatientLoc + AuPoint(irad,-irad));
	edges.append(m_PatientLoc + AuPoint(irad,-orad));

	edges.append(m_PatientLoc + AuPoint(irad,-orad));
	edges.append(m_PatientLoc + AuPoint(-irad,-orad));

	edges.append(m_PatientLoc + AuPoint(-irad,-orad));
	edges.append(m_PatientLoc + AuPoint(-irad,-irad));

	edges.append(m_PatientLoc + AuPoint(-irad,-irad));
	edges.append(m_PatientLoc + AuPoint(-orad,-irad));

	edges.append(m_PatientLoc + AuPoint(-orad,-irad));
	edges.append(m_PatientLoc + AuPoint(-orad,irad));

	edges.append(m_PatientLoc + AuPoint(-orad,irad));
	edges.append(m_PatientLoc + AuPoint(-irad,irad));

	edges.append(m_PatientLoc + AuPoint(-irad,irad));
	edges.append(m_PatientLoc + AuPoint(-irad,orad));

	edges.append(m_PatientLoc + AuPoint(-irad,orad));
	edges.append(m_PatientLoc + AuPoint(irad,orad));

	edges.append(m_PatientLoc + AuPoint(irad,orad));
	edges.append(m_PatientLoc + AuPoint(irad,irad));

	if(m_PatientLoc != m_PatientFacing)
	{
		edges.append(m_PatientLoc);
		edges.append(m_PatientFacing);
	}
}

void AuScStationMedic::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		AuScStation::ListComponents(pc, streamline, armor, hullMaterial);
		if(Module() != CID_Invalid)
			pc.AddRequirement(Module(), CE_Required, 1);
	}
	else
	{
		pc.AddRequirement(CID_Metal, CE_Required, 1);
		pc.AddRequirement(CID_Textiles, CE_Required, 4);
	}
}

AuStation AuScStationMedic::ClosestStation(const AuPoint& opLoc, const AuPoint& gravityUp) const
{
	double patdist2 = PatientLoc().Distance2(opLoc);
	double opdist2 = OperatorLoc().Distance2(opLoc);
	if(opdist2 < patdist2)
		return OperatorStation(gravityUp);						// Return medic's station position.

	// Return patient's station position.
	return PatientStation(gravityUp);
}

AuStation AuScStationMedic::PatientStation(const AuPoint& gravityUp) const
{
	AuPoint o(PatientLoc());
	AuPoint y = m_Face.FaceLocation() - o;						// Calculate y to face the screen from the operator loc.
	y.Unit();
	AuPoint x = y.Cross(gravityUp);
	x.Unit();
	y = gravityUp.Cross(x);										// Adjust y to be perpendicular to the gravity up vector.
	y.Unit();
	AuMatrix4x4 opM;
	opM.Translate(o);
	opM.RotateToPlane(x, y);

	if(m_PatientFacing != o)
	{
		// Calculate facing rotation.
		AuPoint fvec(m_PatientFacing - o);
		fvec.Unit();
		double fx = fvec.Dot(x);
		double fy = fvec.Dot(y);
		double zang = atan2(fy, fx);
		double zrot = zang - PI/2;
		opM.RotateZ(zrot);
	}

	AuStation::OCCUPANT_POSITION occPos;
	switch(PatientPos())
	{
#if defined(QT_NO_DEBUG)
	default:
#endif
	case Patient_Sit:
		occPos = AuStation::Occupant_Sit;
		break;
	case Patient_SitStationary:
		occPos = AuStation::Occupant_SitStationary;
		break;
	case Patient_Stand:
		occPos = AuStation::Occupant_Stand;
		break;
	case Patient_StandStationary:
		occPos = AuStation::Occupant_StandStationary;
		break;
	case Patient_Prone:
		occPos = AuStation::Occupant_Prone;
		break;
	}
	return AuStation(AuStation::Integrated_Patient, opM, occPos, 1);
}

quint64 AuScStationMedic::MemoryUsage() const
{
	quint64 mem = AuScStation::MemoryUsage();
	mem += sizeof(m_PatientFacing);
	mem += sizeof(m_PatientLoc);
	mem += sizeof(m_PatientPos);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationNavigator::AuScStationNavigator()
{
}

AuScStationNavigator::AuScStationNavigator(const AuScStationNavigator& c) : AuScStation(c)
{
}

AuScStationNavigator::AuScStationNavigator(const AuScStationNavigator& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
}

void AuScStationNavigator::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
}

void AuScStationNavigator::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
}

bool AuScStationNavigator::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationNavigator& c = dynamic_cast<const AuScStationNavigator&>(p);
		if(!AuScStation::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationNavigator::Clone() const
{
	return new AuScStationNavigator(*this);
}

AuScPart* AuScStationNavigator::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationNavigator(*this, selInfo);
}

QString AuScStationNavigator::TypeIcon() const
{
	return QString(":data/dp_NavConsole.png");
}

QString AuScStationNavigator::TypeName() const
{
	return QObject::tr("Navigator Station");
}

GLuint AuScStationNavigator::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txConsoleNav.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationPowerRelay::AuScStationPowerRelay()
{
}

AuScStationPowerRelay::AuScStationPowerRelay(const AuScStationPowerRelay& c) : AuScStation(c)
{
}

AuScStationPowerRelay::AuScStationPowerRelay(const AuScStationPowerRelay& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
}

void AuScStationPowerRelay::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
}

void AuScStationPowerRelay::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
}

bool AuScStationPowerRelay::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationPowerRelay& c = dynamic_cast<const AuScStationPowerRelay&>(p);
		if(!AuScStation::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationPowerRelay::Clone() const
{
	return new AuScStationPowerRelay(*this);
}

AuScPart* AuScStationPowerRelay::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationPowerRelay(*this, selInfo);
}

QString AuScStationPowerRelay::TypeIcon() const
{
	return QString(":data/dp_PowerConsole.png");
}

QString AuScStationPowerRelay::TypeName() const
{
	return QObject::tr("Power Relay Station");
}

GLuint AuScStationPowerRelay::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/dc_Console.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationSensor::AuScStationSensor()
{
}

AuScStationSensor::AuScStationSensor(const AuScStationSensor& c) : AuScStation(c)
{
}

AuScStationSensor::AuScStationSensor(const AuScStationSensor& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
}

void AuScStationSensor::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
}

void AuScStationSensor::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
}

bool AuScStationSensor::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationSensor& c = dynamic_cast<const AuScStationSensor&>(p);
		if(!AuScStation::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationSensor::Clone() const
{
	return new AuScStationSensor(*this);
}

AuScPart* AuScStationSensor::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationSensor(*this, selInfo);
}

QString AuScStationSensor::TypeIcon() const
{
	return QString(":data/dp_SensorConsole.png");
}

QString AuScStationSensor::TypeName() const
{
	return QObject::tr("Sensor Station");
}

GLuint AuScStationSensor::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/sc_Console.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

AuPoint AuScStationSensor::SoundEmitLoc() const
{
	return Center();					// Sound emits from the screen.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationShield::AuScStationShield()
{
}

AuScStationShield::AuScStationShield(const AuScStationShield& c) : AuScStation(c)
{
}

AuScStationShield::AuScStationShield(const AuScStationShield& c, const AuScSelectInfo& selInfo) : AuScStation(c, selInfo)
{
}

void AuScStationShield::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
}

void AuScStationShield::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
}

bool AuScStationShield::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationShield& c = dynamic_cast<const AuScStationShield&>(p);
		if(!AuScStation::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationShield::Clone() const
{
	return new AuScStationShield(*this);
}

AuScPart* AuScStationShield::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationShield(*this, selInfo);
}

QString AuScStationShield::TypeIcon() const
{
	return QString(":data/dp_ShieldConsole.png");
}

QString AuScStationShield::TypeName() const
{
	return QObject::tr("Shield Station");
}

GLuint AuScStationShield::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txConsoleShield.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStationTransporter::AuScStationTransporter()
{
	m_WhichEffect = 0;
}

AuScStationTransporter::AuScStationTransporter(const AuScStationTransporter& c) : AuScStation(c)
{
	m_PadLocs = c.m_PadLocs;
	m_WhichEffect = c.m_WhichEffect;
}

AuScStationTransporter::AuScStationTransporter(const AuScStationTransporter& c, const AuScSelectInfo& selInfo) :
	AuScStation(c, selInfo)
{
	m_PadLocs = c.m_PadLocs;
	m_WhichEffect = c.m_WhichEffect;
}

void AuScStationTransporter::operator>>(QDataStream& out) const
{
	AuScStation::operator>>(out);
	out << (quint8)m_PadLocs.size();
	for(AuPointArray::const_iterator pi = m_PadLocs.cbegin(); pi != m_PadLocs.cend(); pi++)
		out << (*pi);
	out << m_WhichEffect;
}

void AuScStationTransporter::operator<<(QDataStream& in)
{
	AuScStation::operator<<(in);
	m_PadLocs.clear();
	quint8 c;
	in >> c;
	while(c--)
	{
		AuPoint padLoc;
		in >> padLoc;
		m_PadLocs.append(padLoc);
	}
	if(ReadVersion() >= 0x0b)
		in >> m_WhichEffect;
}

bool AuScStationTransporter::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStationTransporter& c = dynamic_cast<const AuScStationTransporter&>(p);
		if(!AuScStation::operator==(c))
			return false;
		if(m_PadLocs != c.m_PadLocs)
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStationTransporter::Clone() const
{
	return new AuScStationTransporter(*this);
}

AuScPart* AuScStationTransporter::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStationTransporter(*this, selInfo);
}

QString AuScStationTransporter::TypeIcon() const
{
	return QString(":data/dp_TransporterConsole.png");
}

QString AuScStationTransporter::TypeName() const
{
	return QObject::tr("Transporter Station");
}

GLuint AuScStationTransporter::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txConsoleTransporter.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScStationTransporter::Transform(const AuMatrix4x4& t)
{
	AuScStation::Transform(t);
	m_PadLocs.Transform(t);
}

void AuScStationTransporter::EdgeSegments(AuPointArray& edges) const
{
	AuScStation::EdgeSegments(edges);

	AuMatrix4x4 t(PositionMatrix());
	AuPoint vx = t.ProjectVector(1,0,0);
	AuPoint vy = t.ProjectVector(0,1,0);
	vx.Unit();
	vy.Unit();
	double prad = .5;

	for(AuPointArray::const_iterator pi = m_PadLocs.cbegin(); pi != m_PadLocs.cend(); pi++)
	{
		AuMatrix4x4 t(*pi, vx, vy);
		AuPoint p(t.Project(prad, 0, 0));
		for(int v=1; v<=6; v++)
		{
			float ang = v*PI*2/6.;
			AuPoint n(t.Project(AuPoint(prad,0).RotateZ(ang)));
			edges.append(p);
			edges.append(n);
			p = n;
		}
	}
}

void AuScStationTransporter::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	AuScStation::ListComponents(pc, streamline, armor, hullMaterial);

	// Transporter pads.
	pc.AddRequirement(CID_ElectronicPart, CE_Required, 3 * m_PadLocs.size());
	pc.AddRequirement(CID_IoTube, CE_Required, 1 * m_PadLocs.size());
	pc.AddRequirement(CID_Plastic, CE_Required, 1 * m_PadLocs.size());
}

QString AuScStationTransporter::PartName() const
{
	return QObject::tr("%1 %2").arg(TypeName()).arg(m_WhichEffect+1);
}

quint64 AuScStationTransporter::MemoryUsage() const
{
	quint64 mem = AuScStation::MemoryUsage();
	mem += m_PadLocs.MemoryUsage();
	mem += sizeof(m_WhichEffect);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScIcon::AuScIcon()
{
}

AuScIcon::AuScIcon(const AuScIcon& c) : AuScScreen(c)
{
}

AuScIcon::AuScIcon(const AuScIcon& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScIcon::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScIcon::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScIcon::operator==(const AuScPart& p) const
{
	try
	{
		const AuScIcon& c = dynamic_cast<const AuScIcon&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuScIcon::MakeIcon(const AuPoint& loc, float rot, const AuMatrix4x4& gridPlane)
{
	double r = rot;
	while(r < -PI)
		r += PI*2;
	while(r > PI)
		r -= PI*2;

	AuMatrix4x4 iconPlane;
	iconPlane.Translate(loc);
	iconPlane.RotateZ(r);
	iconPlane.Rotate(gridPlane.GetRotation());
	MakeIcon(iconPlane);
}

void AuScIcon::MakeIcon(const AuMatrix4x4& atPlane)
{
	AuPoint ll(-IconLeft(), -IconBack());
	AuPoint ur(IconRight(), IconFore());
	MakeScreen(ll, ur, AuMatrix4x4());
	Transform(atPlane);

	// If icons are slightly transparent they get blended, which tends to look a little nicer.
	SetFaceOpacity(.95f);
}

void AuScIcon::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsole::AuScConsole()
{
	m_Module = CID_Invalid;
}

AuScConsole::AuScConsole(const AuScConsole& c) : AuScIcon(c)
{
	m_Module = c.m_Module;
}

AuScConsole::AuScConsole(const AuScConsole& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
	m_Module = c.m_Module;
}

void AuScConsole::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);

	out << m_Module;
}

void AuScConsole::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);

	if(ReadVersion() >= 0x01)
		in >> m_Module;
	else
		m_Module = CID_Invalid;
}

bool AuScConsole::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsole& c = dynamic_cast<const AuScConsole&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

double AuScConsole::IconBack() const
{
	return FeetPerMeter() * .6;
}

double AuScConsole::IconFore() const
{
	return FeetPerMeter() * 1.1;
}

double AuScConsole::IconLeft() const
{
	return FeetPerMeter() * .5;
}

double AuScConsole::IconRight() const
{
	return FeetPerMeter() * .5;
}

QString AuScConsole::ReportDesc() const
{
	if(Module() == CID_Invalid)
		return PartName();

	return QObject::tr("%1, %2")
		.arg(PartName())
		.arg(g_Au.CommodityName(Module()));
}

QString AuScConsole::SelectedDesc(const AuScParts& parts) const
{
	if(Module() != CID_Invalid)
		return QObject::tr("%1, %2")
			.arg(FullName(parts))
			.arg(g_Au.CommodityName(Module()));
	else
		return QObject::tr("%1")
			.arg(FullName(parts));
}

AuPoint AuScConsole::OperatorLoc() const
{
	return m_Face.FaceLocation();
}

void AuScConsole::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	// Screen adds 1 electronic part and 1 plastic.
	AuScScreen::ListComponents(pc, streamline, armor, hullMaterial);
	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial == CID_Lumber)
	{
		pc.AddRequirement(CID_Computer, CE_Required, 1);
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 1);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 1);
		pc.AddRequirement(CID_Textiles, CE_Required, 1);
	}
	else
	{
		if(Module() != CID_Invalid)
			pc.AddRequirement(Module(), CE_Required, 1);
	}
}

AuMatrix4x4 AuScConsole::PositionMatrix() const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(o, x, y);
}

bool AuScConsole::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	// Lift the face slightly. That helps to sight the base of the console vs the coplanar floor, where it most likely resides.
	AuScFaceQuad f(m_Face);
	AuMatrix4x4 t;
	AuPoint fN(f.FaceNormal());
	t.Translate(fN * .1);
	f.Transform(t);
	if(!f.Sight(eyeLoc, sightLoc))
		return false;
	sightSurfaceNormal = fN;
	return true;
}

AuSysId AuScConsole::SightedSystem(quint16 partId) const
{
	return AuSysId(SYS_CommandAndControl, partId);
}

AuStation AuScConsole::OperatorStation(const AuPoint& gravityUp) const
{
	Q_UNUSED(gravityUp);
	AuMatrix4x4 opM(PositionMatrix());
	opM.Translate(0,0,AuStation::SeatHeight());
	return AuStation(StationType(), opM, AuStation::Occupant_Sit, 0);
}

quint64 AuScConsole::MemoryUsage() const
{
	quint64 mem = AuScIcon::MemoryUsage();
	mem += sizeof(m_Module);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleCommand::AuScConsoleCommand()
{
}

AuScConsoleCommand::AuScConsoleCommand(const AuScConsoleCommand& c) : AuScConsole(c)
{
}

AuScConsoleCommand::AuScConsoleCommand(const AuScConsoleCommand& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
}

void AuScConsoleCommand::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
}

void AuScConsoleCommand::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
}

bool AuScConsoleCommand::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleCommand& c = dynamic_cast<const AuScConsoleCommand&>(p);
		if(!AuScConsole::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleCommand::Clone() const
{
	return new AuScConsoleCommand(*this);
}

AuScPart* AuScConsoleCommand::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleCommand(*this, selInfo);
}

QString AuScConsoleCommand::TypeIcon() const
{
	return QString(":data/dp_CmdConsole.png");
}

QString AuScConsoleCommand::TypeName() const
{
	return QObject::tr("Command Console");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleEngineer::AuScConsoleEngineer()
{
	m_WhichEffect = 0;
}

AuScConsoleEngineer::AuScConsoleEngineer(const AuScConsoleEngineer& c) : AuScConsole(c)
{
	m_WhichEffect = c.m_WhichEffect;
}

AuScConsoleEngineer::AuScConsoleEngineer(const AuScConsoleEngineer& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
	m_WhichEffect = c.m_WhichEffect;
}

void AuScConsoleEngineer::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
	out << m_WhichEffect;
}

void AuScConsoleEngineer::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
	if(ReadVersion() >= 0x0b)
		in >> m_WhichEffect;
}

bool AuScConsoleEngineer::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleEngineer& c = dynamic_cast<const AuScConsoleEngineer&>(p);
		if(!AuScConsole::operator==(c))
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleEngineer::Clone() const
{
	return new AuScConsoleEngineer(*this);
}

AuScPart* AuScConsoleEngineer::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleEngineer(*this, selInfo);
}

QString AuScConsoleEngineer::TypeIcon() const
{
	return QString(":data/dp_EngineerConsole.png");
}

QString AuScConsoleEngineer::TypeName() const
{
	return QObject::tr("Engineer Console");
}

QString AuScConsoleEngineer::PartName() const
{
	return QObject::tr("%1 %2").arg(TypeName()).arg(m_WhichEffect+1);
}

quint64 AuScConsoleEngineer::MemoryUsage() const
{
	quint64 mem = AuScConsole::MemoryUsage();
	mem += sizeof(m_WhichEffect);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleFireControl::AuScConsoleFireControl()
{
	m_WhichEffect = 0;
}

AuScConsoleFireControl::AuScConsoleFireControl(const AuScConsoleFireControl& c) : AuScConsole(c)
{
	m_EmitLoc = c.m_EmitLoc;
	m_WhichEffect = c.m_WhichEffect;
}

AuScConsoleFireControl::AuScConsoleFireControl(const AuScConsoleFireControl& c, const AuScSelectInfo& selInfo) :
	AuScConsole(c, selInfo)
{
	m_EmitLoc = c.m_EmitLoc;
	m_WhichEffect = c.m_WhichEffect;
}

void AuScConsoleFireControl::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
	out << m_EmitLoc;
	out << m_WhichEffect;
}

void AuScConsoleFireControl::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
	in >> m_EmitLoc;
	if(ReadVersion() >= 0x0b)
		in >> m_WhichEffect;
}

bool AuScConsoleFireControl::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleFireControl& c = dynamic_cast<const AuScConsoleFireControl&>(p);
		if(!AuScConsole::operator==(c))
			return false;
		if(m_EmitLoc != c.m_EmitLoc)
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleFireControl::Clone() const
{
	return new AuScConsoleFireControl(*this);
}

AuScPart* AuScConsoleFireControl::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleFireControl(*this, selInfo);
}

QString AuScConsoleFireControl::TypeIcon() const
{
	return QString(":data/dp_FireControlConsole.png");
}

QString AuScConsoleFireControl::TypeName() const
{
	return QObject::tr("Fire Control Console");
}

void AuScConsoleFireControl::EdgeSegments(AuPointArray& edges) const
{
	AuScConsole::EdgeSegments(edges);

	double irad = FeetPerMeter();
	double orad = 3 * FeetPerMeter();

	edges.append(m_EmitLoc + AuPoint(irad,irad));
	edges.append(m_EmitLoc + AuPoint(orad,0));

	edges.append(m_EmitLoc + AuPoint(orad,0));
	edges.append(m_EmitLoc + AuPoint(irad,-irad));

	edges.append(m_EmitLoc + AuPoint(irad,-irad));
	edges.append(m_EmitLoc + AuPoint(0,-orad));

	edges.append(m_EmitLoc + AuPoint(0,-orad));
	edges.append(m_EmitLoc + AuPoint(-irad,-irad));

	edges.append(m_EmitLoc + AuPoint(-irad,-irad));
	edges.append(m_EmitLoc + AuPoint(-orad,0));

	edges.append(m_EmitLoc + AuPoint(-orad,0));
	edges.append(m_EmitLoc + AuPoint(-irad,irad));

	edges.append(m_EmitLoc + AuPoint(-irad,irad));
	edges.append(m_EmitLoc + AuPoint(0,orad));

	edges.append(m_EmitLoc + AuPoint(0,orad));
	edges.append(m_EmitLoc + AuPoint(irad,irad));
}

void AuScConsoleFireControl::Transform(const AuMatrix4x4& t)
{
	AuScConsole::Transform(t);
	m_EmitLoc = t.Project(m_EmitLoc);
}

QString AuScConsoleFireControl::PartName() const
{
	return QObject::tr("%1 %2").arg(TypeName()).arg(m_WhichEffect+1);
}

quint64 AuScConsoleFireControl::MemoryUsage() const
{
	quint64 mem = AuScConsole::MemoryUsage();
	mem += sizeof(m_EmitLoc);
	mem += sizeof(m_WhichEffect);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleHelm::AuScConsoleHelm()
{
}

AuScConsoleHelm::AuScConsoleHelm(const AuScConsoleHelm& c) : AuScConsole(c)
{
}

AuScConsoleHelm::AuScConsoleHelm(const AuScConsoleHelm& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
}

void AuScConsoleHelm::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
}

void AuScConsoleHelm::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
}

bool AuScConsoleHelm::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleHelm& c = dynamic_cast<const AuScConsoleHelm&>(p);
		if(!AuScConsole::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleHelm::Clone() const
{
	return new AuScConsoleHelm(*this);
}

AuScPart* AuScConsoleHelm::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleHelm(*this, selInfo);
}

QString AuScConsoleHelm::TypeIcon() const
{
	return QString(":data/dp_PilotConsole.png");
}

QString AuScConsoleHelm::TypeName() const
{
	return QObject::tr("Helm Console");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleMedic::AuScConsoleMedic()
{
	m_PatientPos = Patient_Stand;
}

AuScConsoleMedic::AuScConsoleMedic(const AuScConsoleMedic& c) : AuScConsole(c)
{
	m_PatientFacing = c.m_PatientFacing;
	m_PatientLoc = c.m_PatientLoc;
	m_PatientPos = c.m_PatientPos;
}

AuScConsoleMedic::AuScConsoleMedic(const AuScConsoleMedic& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
	m_PatientFacing = c.m_PatientFacing;
	m_PatientLoc = c.m_PatientLoc;
	m_PatientPos = c.m_PatientPos;
}

void AuScConsoleMedic::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
	quint8 hasWhat = m_PatientPos;
	if(m_PatientFacing != m_PatientLoc)
		hasWhat |= 0x80;
	out << hasWhat;
	out << m_PatientLoc;
	if(hasWhat & 0x80)
		out << m_PatientFacing;
}

void AuScConsoleMedic::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	m_PatientPos = (PATIENT_POSITION)(hasWhat & 0x7f);
	in >> m_PatientLoc;
	if(hasWhat & 0x80)
		in >> m_PatientFacing;
	else
		m_PatientFacing = m_PatientLoc;
}

bool AuScConsoleMedic::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleMedic& c = dynamic_cast<const AuScConsoleMedic&>(p);
		if(!AuScConsole::operator==(c))
			return false;

		if(m_PatientFacing != c.m_PatientFacing)
			return false;
		if(m_PatientLoc != c.m_PatientLoc)
			return false;
		if(m_PatientPos != c.m_PatientPos)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleMedic::Clone() const
{
	return new AuScConsoleMedic(*this);
}

AuScPart* AuScConsoleMedic::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleMedic(*this, selInfo);
}

QString AuScConsoleMedic::TypeIcon() const
{
	return QString(":data/dp_SurgeryConsole.png");
}

QString AuScConsoleMedic::TypeName() const
{
	return QObject::tr("Medic Console");
}

void AuScConsoleMedic::Transform(const AuMatrix4x4& t)
{
	AuScConsole::Transform(t);
	m_PatientFacing = t.Project(m_PatientFacing);
	m_PatientLoc = t.Project(m_PatientLoc);
}

void AuScConsoleMedic::EdgeSegments(AuPointArray& edges) const
{
	AuScConsole::EdgeSegments(edges);
	//qDebug() << "AuScConsoleMedic::EdgeSegments patientLoc" << m_PatientLoc;

	double irad = .5;
	double orad = 1;

	edges.append(m_PatientLoc + AuPoint(irad,irad));
	edges.append(m_PatientLoc + AuPoint(orad,irad));

	edges.append(m_PatientLoc + AuPoint(orad,irad));
	edges.append(m_PatientLoc + AuPoint(orad,-irad));

	edges.append(m_PatientLoc + AuPoint(orad,-irad));
	edges.append(m_PatientLoc + AuPoint(irad,-irad));

	edges.append(m_PatientLoc + AuPoint(irad,-irad));
	edges.append(m_PatientLoc + AuPoint(irad,-orad));

	edges.append(m_PatientLoc + AuPoint(irad,-orad));
	edges.append(m_PatientLoc + AuPoint(-irad,-orad));

	edges.append(m_PatientLoc + AuPoint(-irad,-orad));
	edges.append(m_PatientLoc + AuPoint(-irad,-irad));

	edges.append(m_PatientLoc + AuPoint(-irad,-irad));
	edges.append(m_PatientLoc + AuPoint(-orad,-irad));

	edges.append(m_PatientLoc + AuPoint(-orad,-irad));
	edges.append(m_PatientLoc + AuPoint(-orad,irad));

	edges.append(m_PatientLoc + AuPoint(-orad,irad));
	edges.append(m_PatientLoc + AuPoint(-irad,irad));

	edges.append(m_PatientLoc + AuPoint(-irad,irad));
	edges.append(m_PatientLoc + AuPoint(-irad,orad));

	edges.append(m_PatientLoc + AuPoint(-irad,orad));
	edges.append(m_PatientLoc + AuPoint(irad,orad));

	edges.append(m_PatientLoc + AuPoint(irad,orad));
	edges.append(m_PatientLoc + AuPoint(irad,irad));

	edges.append(m_PatientLoc);
	edges.append(m_PatientFacing);
}

void AuScConsoleMedic::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		AuScConsole::ListComponents(pc, streamline, armor, hullMaterial);
		if(Module() != CID_Invalid)
			pc.AddRequirement(Module(), CE_Required, 1);
	}
	else
	{
		pc.AddRequirement(CID_Metal, CE_Required, 1);
		pc.AddRequirement(CID_Textiles, CE_Required, 4);
	}
}

AuStation AuScConsoleMedic::ClosestStation(const AuPoint& opLoc, const AuPoint& gravityUp) const
{
	double patdist2 = PatientLoc().Distance2(opLoc);
	double opdist2 = OperatorLoc().Distance2(opLoc);
	if(opdist2 < patdist2)
		return OperatorStation(gravityUp);						// Return medic's station position.

	// Return patient's station position.
	return PatientStation(gravityUp);
}

AuStation AuScConsoleMedic::PatientStation(const AuPoint& gravityUp) const
{
	Q_UNUSED(gravityUp);
	AuPoint o(PatientLoc());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	AuMatrix4x4 opM;
	opM.Translate(o);
	opM.RotateToPlane(x, y);

	if(m_PatientFacing != o)
	{
		// Calculate facing rotation.
		AuPoint fvec(m_PatientFacing - o);
		fvec.Unit();
		double fx = fvec.Dot(x);
		double fy = fvec.Dot(y);
		double zang = atan2(fy, fx);
		double zrot = zang - PI/2;
		opM.RotateZ(zrot);
	}

	AuStation::OCCUPANT_POSITION occPos;
	switch(PatientPos())
	{
#if defined(QT_NO_DEBUG)
	default:
#endif
	case Patient_Sit:
		occPos = AuStation::Occupant_Sit;
		break;
	case Patient_SitStationary:
		occPos = AuStation::Occupant_SitStationary;
		break;
	case Patient_Stand:
		occPos = AuStation::Occupant_Stand;
		break;
	case Patient_StandStationary:
		occPos = AuStation::Occupant_StandStationary;
		break;
	case Patient_Prone:
		occPos = AuStation::Occupant_Prone;
		break;
	}
	// Console_Patient is not passed here because that implies the old style surgery unit, which was always Occupant_Stand.
	return AuStation(AuStation::Integrated_Patient, opM, occPos, 1);
}

quint64 AuScConsoleMedic::MemoryUsage() const
{
	quint64 mem = AuScConsole::MemoryUsage();
	mem += sizeof(m_PatientFacing);
	mem += sizeof(m_PatientLoc);
	mem += sizeof(m_PatientPos);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleNavigator::AuScConsoleNavigator()
{
}

AuScConsoleNavigator::AuScConsoleNavigator(const AuScConsoleNavigator& c) : AuScConsole(c)
{
}

AuScConsoleNavigator::AuScConsoleNavigator(const AuScConsoleNavigator& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
}

void AuScConsoleNavigator::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
}

void AuScConsoleNavigator::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
}

bool AuScConsoleNavigator::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleNavigator& c = dynamic_cast<const AuScConsoleNavigator&>(p);
		if(!AuScConsole::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleNavigator::Clone() const
{
	return new AuScConsoleNavigator(*this);
}

AuScPart* AuScConsoleNavigator::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleNavigator(*this, selInfo);
}

QString AuScConsoleNavigator::TypeIcon() const
{
	return QString(":data/dp_NavConsole.png");
}

QString AuScConsoleNavigator::TypeName() const
{
	return QObject::tr("Navigator Console");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsolePowerRelay::AuScConsolePowerRelay()
{
}

AuScConsolePowerRelay::AuScConsolePowerRelay(const AuScConsolePowerRelay& c) : AuScConsole(c)
{
}

AuScConsolePowerRelay::AuScConsolePowerRelay(const AuScConsolePowerRelay& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
}

void AuScConsolePowerRelay::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
}

void AuScConsolePowerRelay::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
}

bool AuScConsolePowerRelay::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsolePowerRelay& c = dynamic_cast<const AuScConsolePowerRelay&>(p);
		if(!AuScConsole::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsolePowerRelay::Clone() const
{
	return new AuScConsolePowerRelay(*this);
}

AuScPart* AuScConsolePowerRelay::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsolePowerRelay(*this, selInfo);
}

QString AuScConsolePowerRelay::TypeIcon() const
{
	return QString(":data/dp_PowerConsole.png");
}

QString AuScConsolePowerRelay::TypeName() const
{
	return QObject::tr("Power Relay Console");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleSensor::AuScConsoleSensor()
{
}

AuScConsoleSensor::AuScConsoleSensor(const AuScConsoleSensor& c) : AuScConsole(c)
{
}

AuScConsoleSensor::AuScConsoleSensor(const AuScConsoleSensor& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
}

void AuScConsoleSensor::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
}

void AuScConsoleSensor::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
}

bool AuScConsoleSensor::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleSensor& c = dynamic_cast<const AuScConsoleSensor&>(p);
		if(!AuScConsole::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleSensor::Clone() const
{
	return new AuScConsoleSensor(*this);
}

AuScPart* AuScConsoleSensor::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleSensor(*this, selInfo);
}

QString AuScConsoleSensor::TypeIcon() const
{
	return QString(":data/dp_SensorConsole.png");
}

QString AuScConsoleSensor::TypeName() const
{
	return QObject::tr("Sensor Console");
}

AuPoint AuScConsoleSensor::SoundEmitLoc() const
{
	return OperatorLoc();				// Sound emits from the base of the swivel console chair.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleShield::AuScConsoleShield()
{
}

AuScConsoleShield::AuScConsoleShield(const AuScConsoleShield& c) : AuScConsole(c)
{
}

AuScConsoleShield::AuScConsoleShield(const AuScConsoleShield& c, const AuScSelectInfo& selInfo) : AuScConsole(c, selInfo)
{
}

void AuScConsoleShield::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
}

void AuScConsoleShield::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
}

bool AuScConsoleShield::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleShield& c = dynamic_cast<const AuScConsoleShield&>(p);
		if(!AuScConsole::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleShield::Clone() const
{
	return new AuScConsoleShield(*this);
}

AuScPart* AuScConsoleShield::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleShield(*this, selInfo);
}

QString AuScConsoleShield::TypeIcon() const
{
	return QString(":data/dp_ShieldConsole.png");
}

QString AuScConsoleShield::TypeName() const
{
	return QObject::tr("Shield Console");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScConsoleTransporter::AuScConsoleTransporter()
{
	m_WhichEffect = 0;
}

AuScConsoleTransporter::AuScConsoleTransporter(const AuScConsoleTransporter& c) : AuScConsole(c)
{
	m_PadLocs = c.m_PadLocs;
	m_WhichEffect = c.m_WhichEffect;
}

AuScConsoleTransporter::AuScConsoleTransporter(const AuScConsoleTransporter& c, const AuScSelectInfo& selInfo) :
	AuScConsole(c, selInfo)
{
	m_PadLocs = c.m_PadLocs;
	m_WhichEffect = c.m_WhichEffect;
}

void AuScConsoleTransporter::operator>>(QDataStream& out) const
{
	AuScConsole::operator>>(out);
	out << (quint8)m_PadLocs.size();
	for(AuPointArray::const_iterator pi = m_PadLocs.cbegin(); pi != m_PadLocs.cend(); pi++)
		out << (*pi);
	out << m_WhichEffect;
}

void AuScConsoleTransporter::operator<<(QDataStream& in)
{
	AuScConsole::operator<<(in);
	m_PadLocs.clear();
	quint8 c;
	in >> c;
	while(c--)
	{
		AuPoint padLoc;
		in >> padLoc;
		m_PadLocs.append(padLoc);
	}
	if(ReadVersion() >= 0x0b)
		in >> m_WhichEffect;
}

bool AuScConsoleTransporter::operator==(const AuScPart& p) const
{
	try
	{
		const AuScConsoleTransporter& c = dynamic_cast<const AuScConsoleTransporter&>(p);
		if(!AuScConsole::operator==(c))
			return false;
		if(m_PadLocs != c.m_PadLocs)
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScConsoleTransporter::Clone() const
{
	return new AuScConsoleTransporter(*this);
}

AuScPart* AuScConsoleTransporter::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScConsoleTransporter(*this, selInfo);
}

QString AuScConsoleTransporter::TypeIcon() const
{
	return QString(":data/dp_TransporterConsole.png");
}

QString AuScConsoleTransporter::TypeName() const
{
	return QObject::tr("Transporter Console");
}

void AuScConsoleTransporter::Transform(const AuMatrix4x4& t)
{
	AuScConsole::Transform(t);
	m_PadLocs.Transform(t);
}

void AuScConsoleTransporter::EdgeSegments(AuPointArray& edges) const
{
	AuScConsole::EdgeSegments(edges);

	AuMatrix4x4 t(PositionMatrix());
	AuPoint vx = t.ProjectVector(1,0,0);
	AuPoint vy = t.ProjectVector(0,1,0);
	vx.Unit();
	vy.Unit();
	double prad = .5;

	for(AuPointArray::const_iterator pi = m_PadLocs.cbegin(); pi != m_PadLocs.cend(); pi++)
	{
		AuMatrix4x4 t(*pi, vx, vy);
		AuPoint p(t.Project(prad, 0, 0));
		for(int v=1; v<=6; v++)
		{
			float ang = v*PI*2/6.;
			AuPoint n(t.Project(AuPoint(prad,0).RotateZ(ang)));
			edges.append(p);
			edges.append(n);
			p = n;
		}
	}
}

void AuScConsoleTransporter::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	AuScConsole::ListComponents(pc, streamline, armor, hullMaterial);

	// Transporter pads.
	pc.AddRequirement(CID_ElectronicPart, CE_Required, 3 * m_PadLocs.size());
	pc.AddRequirement(CID_IoTube, CE_Required, 1 * m_PadLocs.size());
	pc.AddRequirement(CID_Plastic, CE_Required, 1 * m_PadLocs.size());
}

QString AuScConsoleTransporter::PartName() const
{
	return QObject::tr("%1 %2").arg(TypeName()).arg(m_WhichEffect+1);
}

quint64 AuScConsoleTransporter::MemoryUsage() const
{
	quint64 mem = AuScConsole::MemoryUsage();
	mem += m_PadLocs.MemoryUsage();
	mem += sizeof(m_WhichEffect);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPost::AuScPost()
{
	m_Position = Occupant_Stand;
}

AuScPost::AuScPost(const AuScPost& c) : AuScIcon(c)
{
	m_Position = c.m_Position;
}

AuScPost::AuScPost(const AuScPost& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
	m_Position = c.m_Position;
}

void AuScPost::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
	out << (quint8)m_Position;
}

void AuScPost::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
	quint8 ui8;
	in >> ui8;
	m_Position = (OCCUPANT_POSITION)ui8;
}

bool AuScPost::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPost& c = dynamic_cast<const AuScPost&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		if(m_Position != c.m_Position)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

double AuScPost::IconBack() const
{
	if(m_Position == Occupant_Stand || m_Position == Occupant_StandStationary)
		return FeetPerMeter()/2;
	return FeetPerMeter() * .6;
}

double AuScPost::IconFore() const
{
	if(m_Position == Occupant_Stand || m_Position == Occupant_StandStationary)
		return FeetPerMeter()/2;
	return FeetPerMeter() * .6;
}

double AuScPost::IconLeft() const
{
	if(m_Position == Occupant_Stand || m_Position == Occupant_StandStationary)
		return FeetPerMeter()/2;
	return FeetPerMeter() * .6;
}

double AuScPost::IconRight() const
{
	if(m_Position == Occupant_Stand || m_Position == Occupant_StandStationary)
		return FeetPerMeter()/2;
	return FeetPerMeter() * .6;
}

AuPoint AuScPost::OperatorLoc() const
{
	return m_Face.FaceLocation();
}

AuStation AuScPost::OperatorStation(const AuPoint& gravityUp) const
{
	Q_UNUSED(gravityUp);
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	AuMatrix4x4 opM(o, x, y);
	AuStation::OCCUPANT_POSITION occPos;
	switch(Position())
	{
#if defined(QT_NO_DEBUG)
	default:
#endif
	case Occupant_Sit:
		occPos = AuStation::Occupant_Sit;
		break;
	case Occupant_Stand:
		occPos = AuStation::Occupant_Stand;
		break;
	case Occupant_SitStationary:
		occPos = AuStation::Occupant_SitStationary;
		break;
	case Occupant_StandStationary:
		occPos = AuStation::Occupant_StandStationary;
		break;
	case Occupant_Prone:
		occPos = AuStation::Occupant_Prone;
		break;
	}
	return AuStation(StationType(), opM, occPos, 0);
}

quint64 AuScPost::MemoryUsage() const
{
	quint64 mem = AuScIcon::MemoryUsage();
	mem += sizeof(m_Position);
	return mem;
}

bool AuScPost::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
/*
AuScSurgeryUnit::AuScSurgeryUnit()
{
}

AuScSurgeryUnit::AuScSurgeryUnit(const AuScSurgeryUnit& c) : AuScPost(c)
{
}

AuScSurgeryUnit::AuScSurgeryUnit(const AuScSurgeryUnit& c, const AuScSelectInfo& selInfo) : AuScPost(c, selInfo)
{
}

void AuScSurgeryUnit::operator>>(QDataStream& out) const
{
	AuScPost::operator>>(out);
}

void AuScSurgeryUnit::operator<<(QDataStream& in)
{
	AuScPost::operator<<(in);
}

AuScPart* AuScSurgeryUnit::Clone() const
{
	return new AuScSurgeryUnit(*this);
}

AuScPart* AuScSurgeryUnit::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScSurgeryUnit(*this, selInfo);
}

QString AuScSurgeryUnit::TypeIcon() const
{
	return QString(":data/dp_OperatingTable.png");
}

QString AuScSurgeryUnit::TypeName() const
{
	return QObject::tr("Surgery Occupant Position");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScTransporterPad::AuScTransporterPad()
{
}

AuScTransporterPad::AuScTransporterPad(const AuScTransporterPad& c) : AuScPost(c)
{
}

AuScTransporterPad::AuScTransporterPad(const AuScTransporterPad& c, const AuScSelectInfo& selInfo) : AuScPost(c, selInfo)
{
}

void AuScTransporterPad::operator>>(QDataStream& out) const
{
	AuScPost::operator>>(out);
}

void AuScTransporterPad::operator<<(QDataStream& in)
{
	AuScPost::operator<<(in);
}

AuScPart* AuScTransporterPad::Clone() const
{
	return new AuScTransporterPad(*this);
}

AuScPart* AuScTransporterPad::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScTransporterPad(*this, selInfo);
}

QString AuScTransporterPad::TypeIcon() const
{
	return QString(":data/dp_TransporterPad.png");
}

QString AuScTransporterPad::TypeName() const
{
	return QObject::tr("Transporter Pad");
}
*/
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScTroopPost::AuScTroopPost()
{
}

AuScTroopPost::AuScTroopPost(const AuScTroopPost& c) : AuScPost(c)
{
}

AuScTroopPost::AuScTroopPost(const AuScTroopPost& c, const AuScSelectInfo& selInfo) : AuScPost(c, selInfo)
{
}

void AuScTroopPost::operator>>(QDataStream& out) const
{
	AuScPost::operator>>(out);
}

void AuScTroopPost::operator<<(QDataStream& in)
{
	AuScPost::operator<<(in);
}

bool AuScTroopPost::operator==(const AuScPart& p) const
{
	try
	{
		const AuScTroopPost& c = dynamic_cast<const AuScTroopPost&>(p);
		if(!AuScPost::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScTroopPost::Clone() const
{
	return new AuScTroopPost(*this);
}

AuScPart* AuScTroopPost::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScTroopPost(*this, selInfo);
}

QString AuScTroopPost::TypeIcon() const
{
	return QString(":data/ScTroop.png");
}

QString AuScTroopPost::TypeName() const
{
	return QObject::tr("Troop Post");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScCitizenPost::AuScCitizenPost()
{
}

AuScCitizenPost::AuScCitizenPost(const AuScCitizenPost& c) : AuScPost(c)
{
}

AuScCitizenPost::AuScCitizenPost(const AuScCitizenPost& c, const AuScSelectInfo& selInfo) : AuScPost(c, selInfo)
{
}

void AuScCitizenPost::operator>>(QDataStream& out) const
{
	AuScPost::operator>>(out);
}

void AuScCitizenPost::operator<<(QDataStream& in)
{
	AuScPost::operator<<(in);
}

bool AuScCitizenPost::operator==(const AuScPart& p) const
{
	try
	{
		const AuScCitizenPost& c = dynamic_cast<const AuScCitizenPost&>(p);
		if(!AuScPost::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScCitizenPost::Clone() const
{
	return new AuScCitizenPost(*this);
}

AuScPart* AuScCitizenPost::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScCitizenPost(*this, selInfo);
}

QString AuScCitizenPost::TypeIcon() const
{
	return QString(":data/ScCitizen.png");
}

QString AuScCitizenPost::TypeName() const
{
	return QObject::tr("Citizen Post");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScLivestockPost::AuScLivestockPost()
{
}

AuScLivestockPost::AuScLivestockPost(const AuScLivestockPost& c) : AuScPost(c)
{
}

AuScLivestockPost::AuScLivestockPost(const AuScLivestockPost& c, const AuScSelectInfo& selInfo) : AuScPost(c, selInfo)
{
}

void AuScLivestockPost::operator>>(QDataStream& out) const
{
	AuScPost::operator>>(out);
}

void AuScLivestockPost::operator<<(QDataStream& in)
{
	AuScPost::operator<<(in);
}

bool AuScLivestockPost::operator==(const AuScPart& p) const
{
	try
	{
		const AuScLivestockPost& c = dynamic_cast<const AuScLivestockPost&>(p);
		if(!AuScPost::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScLivestockPost::Clone() const
{
	return new AuScLivestockPost(*this);
}

AuScPart* AuScLivestockPost::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScLivestockPost(*this, selInfo);
}

QString AuScLivestockPost::TypeIcon() const
{
	return QString(":data/ScLivestock.png");
}

QString AuScLivestockPost::TypeName() const
{
	return QObject::tr("Livestock Post");
}

double AuScLivestockPost::IconBack() const
{
	return FeetPerMeter() * 1.5;
}

double AuScLivestockPost::IconFore() const
{
	return FeetPerMeter() * 1.5;
}

double AuScLivestockPost::IconLeft() const
{
	return FeetPerMeter();
}

double AuScLivestockPost::IconRight() const
{
	return FeetPerMeter();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScGroundVehicleParkingSpot::AuScGroundVehicleParkingSpot()
{
}

AuScGroundVehicleParkingSpot::AuScGroundVehicleParkingSpot(const AuScGroundVehicleParkingSpot& c) : AuScIcon(c)
{
}

AuScGroundVehicleParkingSpot::AuScGroundVehicleParkingSpot(const AuScGroundVehicleParkingSpot& c, const AuScSelectInfo& selInfo) :
	AuScIcon(c, selInfo)
{
}

void AuScGroundVehicleParkingSpot::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScGroundVehicleParkingSpot::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScGroundVehicleParkingSpot::operator==(const AuScPart& p) const
{
	try
	{
		const AuScGroundVehicleParkingSpot& c = dynamic_cast<const AuScGroundVehicleParkingSpot&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScGroundVehicleParkingSpot::Clone() const
{
	return new AuScGroundVehicleParkingSpot(*this);
}

AuScPart* AuScGroundVehicleParkingSpot::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScGroundVehicleParkingSpot(*this, selInfo);
}

QString AuScGroundVehicleParkingSpot::TypeIcon() const
{
	return QString(":data/ScParkGroundVehicle.png");
}

QString AuScGroundVehicleParkingSpot::TypeName() const
{
	return QObject::tr("Ground Vehicle Parking Spot");
}

// An F-16 is 10m wide by 15m long.
double AuScGroundVehicleParkingSpot::IconBack() const
{
	return FeetPerMeter()*6;
}

double AuScGroundVehicleParkingSpot::IconFore() const
{
	return FeetPerMeter()*6;
}

double AuScGroundVehicleParkingSpot::IconLeft() const
{
	return FeetPerMeter()*3;
}

double AuScGroundVehicleParkingSpot::IconRight() const
{
	return FeetPerMeter()*3;
}

AuMatrix4x4 AuScGroundVehicleParkingSpot::PositionMatrix() const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(o, x, y);
}

void AuScGroundVehicleParkingSpot::BuildingCursorSegments(AuPointArray& edges) const
{
	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(1));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(2));
	edges.append(m_Face.Vertex(3));

	edges.append(m_Face.Vertex(3));
	edges.append(m_Face.Vertex(0));

	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(3));
}

bool AuScGroundVehicleParkingSpot::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops,
		AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScSpaceVehicleParkingSpot::AuScSpaceVehicleParkingSpot()
{
}

AuScSpaceVehicleParkingSpot::AuScSpaceVehicleParkingSpot(const AuScSpaceVehicleParkingSpot& c) : AuScIcon(c)
{
}

AuScSpaceVehicleParkingSpot::AuScSpaceVehicleParkingSpot(const AuScSpaceVehicleParkingSpot& c, const AuScSelectInfo& selInfo) :
	AuScIcon(c, selInfo)
{
}

void AuScSpaceVehicleParkingSpot::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScSpaceVehicleParkingSpot::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScSpaceVehicleParkingSpot::operator==(const AuScPart& p) const
{
	try
	{
		const AuScSpaceVehicleParkingSpot& c = dynamic_cast<const AuScSpaceVehicleParkingSpot&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScSpaceVehicleParkingSpot::Clone() const
{
	return new AuScSpaceVehicleParkingSpot(*this);
}

AuScPart* AuScSpaceVehicleParkingSpot::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScSpaceVehicleParkingSpot(*this, selInfo);
}

QString AuScSpaceVehicleParkingSpot::TypeIcon() const
{
	return QString(":data/ScParkSpaceVehicle.png");
}

QString AuScSpaceVehicleParkingSpot::TypeName() const
{
	return QObject::tr("Space Vehicle Parking Spot");
}

// An F-16 is 10m wide by 15m long.
// Sizes match AuScSpaceVehicleLauncher.
double AuScSpaceVehicleParkingSpot::IconBack() const
{
	return FeetPerMeter()*8.5;
}

double AuScSpaceVehicleParkingSpot::IconFore() const
{
	return FeetPerMeter()*8.5;
}

double AuScSpaceVehicleParkingSpot::IconLeft() const
{
	return FeetPerMeter()*6;
}

double AuScSpaceVehicleParkingSpot::IconRight() const
{
	return FeetPerMeter()*6;
}

AuMatrix4x4 AuScSpaceVehicleParkingSpot::PositionMatrix() const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(o, x, y);
}

void AuScSpaceVehicleParkingSpot::BuildingCursorSegments(AuPointArray& edges) const
{
	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(1));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(2));
	edges.append(m_Face.Vertex(3));

	edges.append(m_Face.Vertex(3));
	edges.append(m_Face.Vertex(0));

	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(3));
}

AuSysId AuScSpaceVehicleParkingSpot::SightedSystem(quint16 partId) const
{
	// Part id is not included in the system id because it's just a parking spot. The actual landing system has no representation
	// that can be spotted and repaired in the final scene. Instead, repairs to the landing system with a part id of 0 result in
	// repairs to any/all damaged landing systems.
	Q_UNUSED(partId);
	return SYS_VehicleLanding;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScSpaceVehicleLauncher::AuScSpaceVehicleLauncher()
{
}

AuScSpaceVehicleLauncher::AuScSpaceVehicleLauncher(const AuScSpaceVehicleLauncher& c) : AuScIcon(c)
{
	m_ExitLoc = c.m_ExitLoc;
}

AuScSpaceVehicleLauncher::AuScSpaceVehicleLauncher(const AuScSpaceVehicleLauncher& c, const AuScSelectInfo& selInfo) :
	AuScIcon(c, selInfo)
{
	m_ExitLoc = c.m_ExitLoc;
}

void AuScSpaceVehicleLauncher::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
	out << m_ExitLoc;
}

void AuScSpaceVehicleLauncher::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
	in >> m_ExitLoc;
}

bool AuScSpaceVehicleLauncher::operator==(const AuScPart& p) const
{
	try
	{
		const AuScSpaceVehicleLauncher& c = dynamic_cast<const AuScSpaceVehicleLauncher&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		if(m_ExitLoc != c.m_ExitLoc)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScSpaceVehicleLauncher::Clone() const
{
	return new AuScSpaceVehicleLauncher(*this);
}

AuScPart* AuScSpaceVehicleLauncher::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScSpaceVehicleLauncher(*this, selInfo);
}

void AuScSpaceVehicleLauncher::Transform(const AuMatrix4x4& t)
{
	AuScIcon::Transform(t);
	m_ExitLoc = t.Project(m_ExitLoc);
}

QString AuScSpaceVehicleLauncher::TypeIcon() const
{
	return QString(":data/ScLaunchSpaceVehicle.png");
}

QString AuScSpaceVehicleLauncher::TypeName() const
{
	return QObject::tr("Space Vehicle Launcher");
}

// Sizes match AuScSpaceVehicleParkingSpot.
double AuScSpaceVehicleLauncher::IconBack() const
{
	return FeetPerMeter() * 8.5;
}

double AuScSpaceVehicleLauncher::IconFore() const
{
	return FeetPerMeter() * 8.5;
}

double AuScSpaceVehicleLauncher::IconLeft() const
{
	return FeetPerMeter() * 6;
}

double AuScSpaceVehicleLauncher::IconRight() const
{
	return FeetPerMeter() * 6;
}

void AuScSpaceVehicleLauncher::EdgeSegments(AuPointArray& edges) const
{
	AuScIcon::EdgeSegments(edges);

	AuPoint v(m_ExitLoc - m_Face.FaceLocation());
	v.Unit();
	AuPoint vx;
	v.ArbitraryAxis(vx);
	AuPoint vp = m_ExitLoc + v*FeetPerMeter();
	AuPoint vs1 = m_ExitLoc - v*(FeetPerMeter()*2) + vx * FeetPerMeter();
	AuPoint vs2 = m_ExitLoc - v*(FeetPerMeter()*2) - vx * FeetPerMeter();

	edges.append(m_Face.FaceLocation());
	edges.append(m_ExitLoc);

	edges.append(vp);
	edges.append(vs1);

	edges.append(vs1);
	edges.append(vs2);

	edges.append(vs2);
	edges.append(vp);
}

void AuScSpaceVehicleLauncher::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);

	pc.AddRequirement(CID_MechanicalPart, CE_Required, 2);
	pc.AddRequirement(CID_Plastic, CE_Required, 1);
	pc.AddRequirement(CID_ElectronicPart, CE_Required, 1);
}

bool AuScSpaceVehicleLauncher::LaunchPosition(AuMatrix4x4& launchPadM, AuPoint& launchExit) const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	launchPadM = AuMatrix4x4(o, x, y);
	launchExit = m_ExitLoc;
	return true;
}

quint64 AuScSpaceVehicleLauncher::MemoryUsage() const
{
	quint64 mem = AuScIcon::MemoryUsage();
	mem += sizeof(m_ExitLoc);
	return mem;
}

AuSysId AuScSpaceVehicleLauncher::SightedSystem(quint16 partId) const
{
	return AuSysId(SYS_VehicleLaunch, partId);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---| 
AuScBerth::AuScBerth()
{
}

AuScBerth::AuScBerth(const AuScBerth& c) : AuScIcon(c)
{
}

AuScBerth::AuScBerth(const AuScBerth& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
}

bool AuScBerth::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBerth& c = dynamic_cast<const AuScBerth&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuPoint AuScBerth::BerthLocation() const
{
	return m_Face.FaceLocation();
}

double AuScBerth::IconBack() const
{
	return FeetPerMeter();
}

double AuScBerth::IconFore() const
{
	return FeetPerMeter();
}

double AuScBerth::IconLeft() const
{
	return FeetPerMeter()/2;
}

double AuScBerth::IconRight() const
{
	return FeetPerMeter()/2;
}

void AuScBerth::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);

	pc.AddRequirement(hullMaterial, CE_Required, 1);
	pc.AddRequirement(CID_Textiles, CE_Required, 2);
}

AuStation AuScBerth::OperatorStation(const AuPoint& gravityUp) const
{
	Q_UNUSED(gravityUp);
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	AuMatrix4x4 opM(o, x, y);
	return AuStation(StationType(), opM, AuStation::Occupant_Prone, 0);
}

AuSysId AuScBerth::SightedSystem(quint16 partId) const
{
	return AuSysId(SYS_LifeSupport, partId);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBerthCapt::AuScBerthCapt()
{
}

AuScBerthCapt::AuScBerthCapt(const AuScBerthCapt& c) : AuScBerth(c)
{
}

AuScBerthCapt::AuScBerthCapt(const AuScBerthCapt& c, const AuScSelectInfo& selInfo) : AuScBerth(c, selInfo)
{
}

void AuScBerthCapt::operator>>(QDataStream& out) const
{
	AuScBerth::operator>>(out);
}

void AuScBerthCapt::operator<<(QDataStream& in)
{
	AuScBerth::operator<<(in);
}

bool AuScBerthCapt::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBerthCapt& c = dynamic_cast<const AuScBerthCapt&>(p);
		if(!AuScBerth::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScBerthCapt::Clone() const
{
	return new AuScBerthCapt(*this);
}

AuScPart* AuScBerthCapt::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScBerthCapt(*this, selInfo);
}

QString AuScBerthCapt::TypeIcon() const
{
	return QString(":data/Captain.png");
}

QString AuScBerthCapt::TypeName() const
{
	return QObject::tr("Captain Berth");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBerthCrew::AuScBerthCrew()
{
}

AuScBerthCrew::AuScBerthCrew(const AuScBerthCrew& c) : AuScBerth(c)
{
}

AuScBerthCrew::AuScBerthCrew(const AuScBerthCrew& c, const AuScSelectInfo& selInfo) : AuScBerth(c, selInfo)
{
}

void AuScBerthCrew::operator>>(QDataStream& out) const
{
	AuScBerth::operator>>(out);
}

void AuScBerthCrew::operator<<(QDataStream& in)
{
	AuScBerth::operator<<(in);
}

bool AuScBerthCrew::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBerthCrew& c = dynamic_cast<const AuScBerthCrew&>(p);
		if(!AuScBerth::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScBerthCrew::Clone() const
{
	return new AuScBerthCrew(*this);
}

AuScPart* AuScBerthCrew::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScBerthCrew(*this, selInfo);
}

QString AuScBerthCrew::TypeIcon() const
{
	return QString(":data/Crewman.png");
}

QString AuScBerthCrew::TypeName() const
{
	return QObject::tr("Crew Berth");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBerthOfficer::AuScBerthOfficer()
{
}

AuScBerthOfficer::AuScBerthOfficer(const AuScBerthOfficer& c) : AuScBerth(c)
{
}

AuScBerthOfficer::AuScBerthOfficer(const AuScBerthOfficer& c, const AuScSelectInfo& selInfo) : AuScBerth(c, selInfo)
{
}

void AuScBerthOfficer::operator>>(QDataStream& out) const
{
	AuScBerth::operator>>(out);
}

void AuScBerthOfficer::operator<<(QDataStream& in)
{
	AuScBerth::operator<<(in);
}

bool AuScBerthOfficer::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBerthOfficer& c = dynamic_cast<const AuScBerthOfficer&>(p);
		if(!AuScBerth::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScBerthOfficer::Clone() const
{
	return new AuScBerthOfficer(*this);
}

AuScPart* AuScBerthOfficer::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScBerthOfficer(*this, selInfo);
}

QString AuScBerthOfficer::TypeIcon() const
{
	return QString(":data/Officer.png");
}

QString AuScBerthOfficer::TypeName() const
{
	return QObject::tr("Officer Berth");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBerthPassenger::AuScBerthPassenger()
{
}

AuScBerthPassenger::AuScBerthPassenger(const AuScBerthPassenger& c) : AuScBerth(c)
{
}

AuScBerthPassenger::AuScBerthPassenger(const AuScBerthPassenger& c, const AuScSelectInfo& selInfo) : AuScBerth(c, selInfo)
{
}

void AuScBerthPassenger::operator>>(QDataStream& out) const
{
	AuScBerth::operator>>(out);
}

void AuScBerthPassenger::operator<<(QDataStream& in)
{
	AuScBerth::operator<<(in);
}

bool AuScBerthPassenger::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBerthPassenger& c = dynamic_cast<const AuScBerthPassenger&>(p);
		if(!AuScBerth::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScBerthPassenger::Clone() const
{
	return new AuScBerthPassenger(*this);
}

AuScPart* AuScBerthPassenger::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScBerthPassenger(*this, selInfo);
}

QString AuScBerthPassenger::TypeIcon() const
{
	return QString(":data/Passenger.png");
}

QString AuScBerthPassenger::TypeName() const
{
	return QObject::tr("Passenger Berth");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBerthTroop::AuScBerthTroop()
{
}

AuScBerthTroop::AuScBerthTroop(const AuScBerthTroop& c) : AuScBerth(c)
{
}

AuScBerthTroop::AuScBerthTroop(const AuScBerthTroop& c, const AuScSelectInfo& selInfo) : AuScBerth(c, selInfo)
{
}

void AuScBerthTroop::operator>>(QDataStream& out) const
{
	AuScBerth::operator>>(out);
}

void AuScBerthTroop::operator<<(QDataStream& in)
{
	AuScBerth::operator<<(in);
}

bool AuScBerthTroop::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBerthTroop& c = dynamic_cast<const AuScBerthTroop&>(p);
		if(!AuScBerth::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScBerthTroop::Clone() const
{
	return new AuScBerthTroop(*this);
}

AuScPart* AuScBerthTroop::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScBerthTroop(*this, selInfo);
}

QString AuScBerthTroop::TypeIcon() const
{
	return QString(":data/Troop.png");
}

QString AuScBerthTroop::TypeName() const
{
	return QObject::tr("Troop Berth");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScGameTable::AuScGameTable()
{
}

AuScGameTable::AuScGameTable(const AuScGameTable& c) : AuScIcon(c)
{
}

AuScGameTable::AuScGameTable(const AuScGameTable& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
}

void AuScGameTable::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScGameTable::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScGameTable::operator==(const AuScPart& p) const
{
	try
	{
		const AuScGameTable& c = dynamic_cast<const AuScGameTable&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScGameTable::Clone() const
{
	return new AuScGameTable(*this);
}

AuScPart* AuScGameTable::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScGameTable(*this, selInfo);
}

QString AuScGameTable::TypeIcon() const
{
	return QString(":data/dp_GameTable.png");
}

QString AuScGameTable::TypeName() const
{
	return QObject::tr("Game Table");
}

double AuScGameTable::IconBack() const
{
	return FeetPerMeter()*2;
}

double AuScGameTable::IconFore() const
{
	return FeetPerMeter()*2;
}

double AuScGameTable::IconLeft() const
{
	return FeetPerMeter()*2;
}

double AuScGameTable::IconRight() const
{
	return FeetPerMeter()*2;
}

void AuScGameTable::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);

	pc.AddRequirement(CID_Computer, CE_Required, 1);
	pc.AddRequirement(CID_ElectronicPart, CE_Required, 1);
	pc.AddRequirement(CID_MechanicalPart, CE_Required, 2);
	pc.AddRequirement(CID_Plastic, CE_Required, 2);
	pc.AddRequirement(CID_Textiles, CE_Required, 2);
}

AuMatrix4x4 AuScGameTable::PositionMatrix() const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(o, x, y);
}

AuStation AuScGameTable::ClosestStation(const AuPoint& opLoc, const AuPoint& gravityUp) const
{
	Q_UNUSED(gravityUp);
	// Seat vectors from center of game table to seating position.
	AuPoint seat0Vec = AuPoint(4,4,AuStation::SeatHeight());
	AuPoint seat1Vec = AuPoint(4,-4,AuStation::SeatHeight());
	AuPoint seat2Vec = AuPoint(-4,-4,AuStation::SeatHeight());
	AuPoint seat3Vec = AuPoint(-4,4,AuStation::SeatHeight());

	// Seat locations relative to the design.
	AuMatrix4x4 posM(PositionMatrix());
	AuPoint seat0 = posM.Project(seat0Vec);
	AuPoint seat1 = posM.Project(seat1Vec);
	AuPoint seat2 = posM.Project(seat2Vec);
	AuPoint seat3 = posM.Project(seat3Vec);

	// Find the closest seat to 'opLoc'.
	double dist0 = seat0.Distance2(opLoc);
	double dist1 = seat1.Distance2(opLoc);
	double dist2 = seat2.Distance2(opLoc);
	double dist3 = seat3.Distance2(opLoc);
	AuMatrix4x4 opM(posM);
	AuStation::OCCUPANT_POSITION occPos = AuStation::Occupant_Sit;
	unsigned int seatIndex;
	if(dist0 < dist1 && dist0 < dist2 && dist0 < dist3)
	{
		// 'opLoc' is closest to seat 0.
		opM.Translate(seat0Vec);
		opM.RotateZ(135*DEG2RAD);
		seatIndex = 0;
	}
	else if(dist1 < dist2 && dist1 < dist3)
	{
		// 'opLoc' is closest to seat 1.
		opM.Translate(seat1Vec);
		opM.RotateZ(45*DEG2RAD);
		seatIndex = 1;
	}
	else if(dist2 < dist3)
	{
		// 'opLoc' is closest to seat 2.
		opM.Translate(seat2Vec);
		opM.RotateZ(-45*DEG2RAD);
		seatIndex = 2;
	}
	else
	{
		// 'opLoc' is closest to seat 3.
		opM.Translate(seat3Vec);
		opM.RotateZ(-135*DEG2RAD);
		seatIndex = 3;
	}
	return AuStation(StationType(), opM, occPos, seatIndex);
}

AuStation AuScGameTable::OperatorStation(const AuPoint& gravityUp) const
{
	Q_UNUSED(gravityUp);
	AuMatrix4x4 opM(PositionMatrix());
	opM.Translate(AuPoint(4,4,AuStation::SeatHeight()));
	opM.RotateZ(135*DEG2RAD);
	return AuStation(StationType(), opM, AuStation::Occupant_Sit, 0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScContrailEmitter::AuScContrailEmitter()
{
}

AuScContrailEmitter::AuScContrailEmitter(const AuScContrailEmitter& c) : AuScIcon(c)
{
}

AuScContrailEmitter::AuScContrailEmitter(const AuScContrailEmitter& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
}

void AuScContrailEmitter::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScContrailEmitter::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScContrailEmitter::operator==(const AuScPart& p) const
{
	try
	{
		const AuScContrailEmitter& c = dynamic_cast<const AuScContrailEmitter&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScContrailEmitter::Clone() const
{
	return new AuScContrailEmitter(*this);
}

AuScPart* AuScContrailEmitter::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScContrailEmitter(*this, selInfo);
}

QString AuScContrailEmitter::TypeIcon() const
{
	return QString(":data/dp_ContrailEmitter.png");
}

QString AuScContrailEmitter::TypeName() const
{
	return QObject::tr("Contrail Emitter");
}

double AuScContrailEmitter::IconBack() const
{
	return FeetPerMeter();
}

double AuScContrailEmitter::IconFore() const
{
	return FeetPerMeter();
}

double AuScContrailEmitter::IconLeft() const
{
	return FeetPerMeter();
}

double AuScContrailEmitter::IconRight() const
{
	return FeetPerMeter();
}

GLuint AuScContrailEmitter::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/dp_ContrailEmitter.png");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

float AuScContrailEmitter::EmitterDiameter() const
{
	float d = m_Face.Vertex(0).Distance(m_Face.Vertex(3));
	if(d < FeetPerMeter())
		return (float)FeetPerMeter();
	return d;
}

bool AuScContrailEmitter::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScHeadlightLens::AuScHeadlightLens()
{
}

AuScHeadlightLens::AuScHeadlightLens(const AuScHeadlightLens& c) : AuScMeshBase(c)
{
}

AuScHeadlightLens::AuScHeadlightLens(const AuScHeadlightLens& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScHeadlightLens::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScHeadlightLens::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScHeadlightLens::operator==(const AuScPart& p) const
{
	try
	{
		const AuScHeadlightLens& c = dynamic_cast<const AuScHeadlightLens&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScHeadlightLens::Clone() const
{
	return new AuScHeadlightLens(*this);
}

AuScPart* AuScHeadlightLens::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScHeadlightLens(*this, selInfo);
}

void AuScHeadlightLens::Transform(const AuMatrix4x4& t)
{
	AuScMeshBase::Transform(t);
}

QString AuScHeadlightLens::TypeIcon() const
{
	return QString(":data/ScHeadlight.png");
}

QString AuScHeadlightLens::TypeName() const
{
	return QObject::tr("Headlight Lens");
}

void AuScHeadlightLens::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int plastic = (int)surface / d_HullPlatesPerUnit;
	if(plastic < 1)
		plastic = 1;	// Minimum 1 metal per screen.
	if(hullMaterial != CID_Lumber)
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	else
		pc.AddRequirement(CID_Metal, CE_Required, plastic);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		// Add electronic parts when not a building.
		int elec = plastic / 20;
		if(!elec)
			elec = 1;
		pc.AddRequirement(CID_ElectronicPart, CE_Required, elec);
	}
}

QColor AuScHeadlightLens::LightColor() const
{
	return AuGLLight::ColorCarbonArc();
}

AuPoint AuScHeadlightLens::LightLoc() const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	return (min + max)/2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScLightLens::AuScLightLens()
{
	m_Color = AuGLLight::ColorSodiumVapor();
}

AuScLightLens::AuScLightLens(const AuScLightLens& c) : AuScMeshBase(c)
{
	m_Color = c.m_Color;
}

AuScLightLens::AuScLightLens(const AuScLightLens& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
	m_Color = c.m_Color;
}

void AuScLightLens::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
	out << m_Color;
}

void AuScLightLens::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
	in >> m_Color;
}

bool AuScLightLens::operator==(const AuScPart& p) const
{
	try
	{
		const AuScLightLens& c = dynamic_cast<const AuScLightLens&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		if(m_Color != c.m_Color)
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScLightLens::Clone() const
{
	return new AuScLightLens(*this);
}

AuScPart* AuScLightLens::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScLightLens(*this, selInfo);
}

void AuScLightLens::Transform(const AuMatrix4x4& t)
{
	AuScMeshBase::Transform(t);
}

QString AuScLightLens::TypeIcon() const
{
	return QString(":data/ScHeadlight.png");
}

QString AuScLightLens::TypeName() const
{
	return QObject::tr("Light Lens");
}

void AuScLightLens::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int plastic = (int)surface / d_HullPlatesPerUnit;
	if(plastic < 1)
		plastic = 1;	// Minimum 1 metal per screen.
	if(hullMaterial != CID_Lumber)
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	else
		pc.AddRequirement(CID_Metal, CE_Required, plastic);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		// Add electronic parts when not a building.
		int elec = plastic / 20;
		if(!elec)
			elec = 1;
		pc.AddRequirement(CID_ElectronicPart, CE_Required, elec);
	}
}

QColor AuScLightLens::LightColor() const
{
	return m_Color;
}

AuPoint AuScLightLens::LightLoc() const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	return (min + max)/2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScNavLightLens::AuScNavLightLens()
{
	m_Color = QColor(255,0,0);
}

AuScNavLightLens::AuScNavLightLens(const AuScNavLightLens& c) : AuScMeshBase(c)
{
	m_Color = c.m_Color;
}

AuScNavLightLens::AuScNavLightLens(const AuScNavLightLens& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
	m_Color = c.m_Color;
}

void AuScNavLightLens::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
	out << m_Color;
}

void AuScNavLightLens::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
	in >> m_Color;
}

bool AuScNavLightLens::operator==(const AuScPart& p) const
{
	try
	{
		const AuScNavLightLens& c = dynamic_cast<const AuScNavLightLens&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		if(m_Color != c.m_Color)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScNavLightLens::Clone() const
{
	return new AuScNavLightLens(*this);
}

AuScPart* AuScNavLightLens::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScNavLightLens(*this, selInfo);
}

QString AuScNavLightLens::TypeIcon() const
{
	return QString(":data/ScNavLight.png");
}

QString AuScNavLightLens::TypeName() const
{
	return QObject::tr("Nav Light Lens");
}

void AuScNavLightLens::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int plastic = (int)surface / d_HullPlatesPerUnit;
	if(plastic < 1)
		plastic = 1;	// Minimum 1 metal per screen.
	if(hullMaterial != CID_Lumber)
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	else
		pc.AddRequirement(CID_Metal, CE_Required, plastic);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
	{
		// Add electronic parts when not a building.
		int elec = plastic / 20;
		if(!elec)
			elec = 1;
		pc.AddRequirement(CID_ElectronicPart, CE_Required, elec);
	}
}

AuPoint AuScNavLightLens::LightLoc() const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	return (min + max)/2;
}

quint64 AuScNavLightLens::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	mem += sizeof(m_Color);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScRotatingBeaconLens::AuScRotatingBeaconLens()
{
	m_Color = QColor(255,0,0);
	m_RotationAxis = AuPoint(0,0,1);
}

AuScRotatingBeaconLens::AuScRotatingBeaconLens(const AuScRotatingBeaconLens& c) : AuScMeshBase(c)
{
	m_Color = c.m_Color;
	m_RotationAxis = c.m_RotationAxis;
}

AuScRotatingBeaconLens::AuScRotatingBeaconLens(const AuScRotatingBeaconLens& c, const AuScSelectInfo& selInfo) :
	AuScMeshBase(c, selInfo)
{
	m_Color = c.m_Color;
	m_RotationAxis = c.m_RotationAxis;
}

void AuScRotatingBeaconLens::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
	out << m_Color;
	out << m_RotationAxis;
}

void AuScRotatingBeaconLens::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
	if(ReadVersion() >= 0x0e)
		in >> m_Color;
	if(ReadVersion() >= 0x0a)
		in >> m_RotationAxis;
}

bool AuScRotatingBeaconLens::operator==(const AuScPart& p) const
{
	try
	{
		const AuScRotatingBeaconLens& c = dynamic_cast<const AuScRotatingBeaconLens&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		if(m_Color != c.m_Color)
			return false;
		if(m_RotationAxis != c.m_RotationAxis)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScRotatingBeaconLens::Clone() const
{
	return new AuScRotatingBeaconLens(*this);
}

AuScPart* AuScRotatingBeaconLens::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScRotatingBeaconLens(*this, selInfo);
}

void AuScRotatingBeaconLens::Transform(const AuMatrix4x4& t)
{
	AuScMeshBase::Transform(t);
	m_RotationAxis = t.ProjectVector(m_RotationAxis);
	m_RotationAxis.Unit();
}

QString AuScRotatingBeaconLens::TypeIcon() const
{
	return QString(":data/MsgSOS.png");
}

QString AuScRotatingBeaconLens::TypeName() const
{
	return QObject::tr("Rotating Beacon Lens");
}

void AuScRotatingBeaconLens::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int plastic = (int)surface / d_HullPlatesPerUnit;
	if(plastic < 1)
		plastic = 1;	// Minimum 1 metal per screen.
	if(hullMaterial != CID_Lumber)
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	else
		pc.AddRequirement(CID_Metal, CE_Required, plastic);

	// Add electronic parts.
	int elec = plastic / 20;
	if(!elec)
		elec = 1;
	pc.AddRequirement(CID_ElectronicPart, CE_Required, elec);
}

AuPoint AuScRotatingBeaconLens::LightLoc() const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	return (min + max)/2;
}

quint64 AuScRotatingBeaconLens::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	mem += sizeof(m_Color);
	mem += sizeof(m_RotationAxis);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStrobeLens::AuScStrobeLens()
{
	m_Color = QColor(AuGLLight::ColorXenonStrobe());
}

AuScStrobeLens::AuScStrobeLens(const AuScStrobeLens& c) : AuScMeshBase(c)
{
	m_Color = c.m_Color;
}

AuScStrobeLens::AuScStrobeLens(const AuScStrobeLens& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
	m_Color = c.m_Color;
}

void AuScStrobeLens::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
	out << m_Color;
}

void AuScStrobeLens::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
	if(ReadVersion() >= 0x0e)
		in >> m_Color;
}

bool AuScStrobeLens::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStrobeLens& c = dynamic_cast<const AuScStrobeLens&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		if(m_Color != c.m_Color)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStrobeLens::Clone() const
{
	return new AuScStrobeLens(*this);
}

AuScPart* AuScStrobeLens::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStrobeLens(*this, selInfo);
}

QString AuScStrobeLens::TypeIcon() const
{
	return QString(":data/SpotNeutral.png");
}

QString AuScStrobeLens::TypeName() const
{
	return QObject::tr("Strobe Lens");
}

void AuScStrobeLens::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double surface = m_Faces.Area();

	// Increase surface value due to hull density.
	surface *= AuScDesign::HullDensityFactor(0, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int plastic = (int)surface / d_HullPlatesPerUnit;
	if(plastic < 1)
		plastic = 1;	// Minimum 1 metal per screen.
	if(hullMaterial != CID_Lumber)
		pc.AddRequirement(CID_Plastic, CE_Required, plastic);
	else
		pc.AddRequirement(CID_Metal, CE_Required, plastic);

	// Add electronic parts.
	int elec = plastic / 20;
	if(!elec)
		elec = 1;
	pc.AddRequirement(CID_ElectronicPart, CE_Required, elec);
}

AuPoint AuScStrobeLens::LightLoc() const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	return (min + max)/2;
}

quint64 AuScStrobeLens::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	mem += sizeof(m_Color);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScTurret::AuScTurret()
{
	m_BasePivotMax = 0;
	m_BasePivotMin = 0;
	m_GunnerPosition = Gunner_Sit;
	m_GunPivotMax = 0;
	m_GunPivotMin = 0;
	m_Module = CID_Invalid;
}

AuScTurret::AuScTurret(const AuScTurret& c) : AuScStatePart(c)
{
	m_BaseAxis = c.m_BaseAxis;
	m_BasePivot = c.m_BasePivot;
	m_BasePivotMax = c.m_BasePivotMax;
	m_BasePivotMin = c.m_BasePivotMin;
	m_GunAxis = c.m_GunAxis;
	m_GunEmitLoc = c.m_GunEmitLoc;
	m_GunEmitVec = c.m_GunEmitVec;
	m_GunnerFacing = c.m_GunnerFacing;
	m_GunnerLocation = c.m_GunnerLocation;
	m_GunnerPosition = c.m_GunnerPosition;
	m_GunPivot = c.m_GunPivot;
	m_GunPivotMax = c.m_GunPivotMax;
	m_GunPivotMin = c.m_GunPivotMin;
	m_GunSightLoc = c.m_GunSightLoc;
	m_Module = c.m_Module;
}

AuScTurret::AuScTurret(const AuScTurret& c, const AuScSelectInfo& selInfo) : AuScStatePart(c, selInfo)
{
	m_BaseAxis = c.m_BaseAxis;
	m_BasePivot = c.m_BasePivot;
	m_BasePivotMax = c.m_BasePivotMax;
	m_BasePivotMin = c.m_BasePivotMin;
	m_GunAxis = c.m_GunAxis;
	m_GunEmitLoc = c.m_GunEmitLoc;
	m_GunEmitVec = c.m_GunEmitVec;
	m_GunnerFacing = c.m_GunnerFacing;
	m_GunnerLocation = c.m_GunnerLocation;
	m_GunnerPosition = c.m_GunnerPosition;
	m_GunPivot = c.m_GunPivot;
	m_GunPivotMax = c.m_GunPivotMax;
	m_GunPivotMin = c.m_GunPivotMin;
	m_GunSightLoc = c.m_GunSightLoc;
	m_Module = c.m_Module;
}

void AuScTurret::operator>>(QDataStream& out) const
{
	AuScStatePart::operator>>(out);
	out << m_BaseAxis;
	out << m_BasePivot;
	out << m_BasePivotMax;
	out << m_BasePivotMin;
	out << m_GunAxis;
	out << m_GunEmitLoc;
	out << m_GunEmitVec;
	out << m_GunnerLocation;

	quint8 hasWhat = m_GunnerPosition;
	if(m_GunnerFacing != m_GunnerLocation)
		hasWhat |= 0x80;
	if(m_GunSightLoc != m_GunEmitLoc)
		hasWhat |= 0x40;

	out << hasWhat;
	out << m_GunPivot;
	out << m_GunPivotMax;
	out << m_GunPivotMin;
	out << m_Module;
	if(hasWhat & 0x80)
		out << m_GunnerFacing;
	if(hasWhat & 0x40)
		out << m_GunSightLoc;
}

void AuScTurret::operator<<(QDataStream& in)
{
	AuScStatePart::operator<<(in);
	in >> m_BaseAxis;
	in >> m_BasePivot;
	in >> m_BasePivotMax;
	in >> m_BasePivotMin;
	in >> m_GunAxis;
	in >> m_GunEmitLoc;
	in >> m_GunEmitVec;
	in >> m_GunnerLocation;
	quint8 hasWhat;
	in >> hasWhat;
	m_GunnerPosition = hasWhat & 0x07;
	in >> m_GunPivot;
	in >> m_GunPivotMax;
	in >> m_GunPivotMin;

	if(ReadVersion() >= 0x01)
		in >> m_Module;
	else
		m_Module = CID_Invalid;

	if(hasWhat & 0x80)
		in >> m_GunnerFacing;
	else
		m_GunnerFacing = m_GunnerLocation;

	if(hasWhat & 0x40)
		in >> m_GunSightLoc;
	else
		m_GunSightLoc = m_GunEmitLoc;
}

bool AuScTurret::operator==(const AuScPart& p) const
{
	try
	{
		const AuScTurret& c = dynamic_cast<const AuScTurret&>(p);
		if(!AuScStatePart::operator==(c))
			return false;

		if(m_BaseAxis != c.m_BaseAxis)
			return false;
		if(m_BasePivot != c.m_BasePivot)
			return false;
		if(m_BasePivotMax != c.m_BasePivotMax)
			return false;
		if(m_BasePivotMin != c.m_BasePivotMin)
			return false;
		if(m_GunAxis != c.m_GunAxis)
			return false;
		if(m_GunEmitLoc != c.m_GunEmitLoc)
			return false;
		if(m_GunEmitVec != c.m_GunEmitVec)
			return false;
		if(m_GunnerPosition != c.m_GunnerPosition)
			return false;
		if(m_GunnerFacing != c.m_GunnerFacing)
			return false;
		if(m_GunnerLocation != c.m_GunnerLocation)
			return false;
		if(m_GunPivot != c.m_GunPivot)
			return false;
		if(m_GunPivotMax != c.m_GunPivotMax)
			return false;
		if(m_GunPivotMin != c.m_GunPivotMin)
			return false;
		if(m_GunSightLoc != c.m_GunSightLoc)
			return false;
		if(m_Module != c.m_Module)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScTurret::Clone() const
{
	return new AuScTurret(*this);
}

AuScPart* AuScTurret::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScTurret(*this, selInfo);
}

void AuScTurret::Transform(const AuMatrix4x4& t)
{
	AuScStatePart::Transform(t);
	m_BaseAxis = t.ProjectVector(m_BaseAxis);
	m_BaseAxis.Unit();
	m_BasePivot = t.Project(m_BasePivot);
	m_GunAxis = t.ProjectVector(m_GunAxis);
	m_GunAxis.Unit();
	m_GunEmitLoc = t.Project(m_GunEmitLoc);
	m_GunEmitVec = t.ProjectVector(m_GunEmitVec);
	m_GunEmitVec.Unit();
	m_GunnerFacing = t.Project(m_GunnerFacing);
	m_GunnerLocation = t.Project(m_GunnerLocation);
	m_GunPivot = t.Project(m_GunPivot);
	m_GunSightLoc = t.Project(m_GunSightLoc);
}

QString AuScTurret::TypeIcon() const
{
	return QString(":data/dp_TurretTopBottom.png");
}

QString AuScTurret::TypeName() const
{
	return QObject::tr("Turret");
}

QString AuScTurret::StateName() const
{
	return AuScStatePart::StateName();
}

QString AuScTurret::StateName(quint8 state) const
{
	if(state >= STATES_Count)
		return QObject::tr("Invalid State %1").arg(state);

	QString stateNames[] =
	{
		QObject::tr("Base"),
		QObject::tr("Gun"),
		QObject::tr("Gunner Screen"),
		QObject::tr("Gun Firing 1"),
		QObject::tr("Gun Firing 2"),
		QObject::tr("Gun Firing 3"),
		QObject::tr("Gun Firing 4"),
		QObject::tr("Gun Firing 5"),
		QObject::tr("Gun Firing 6"),
		QObject::tr("Gun Firing 7"),
		QObject::tr("Gun Firing 8"),
		QObject::tr("Gun Firing 9"),
		QObject::tr("Gun Firing 10"),
		QObject::tr("Gun Reloading 1"),
		QObject::tr("Gun Reloading 2"),
		QObject::tr("Gun Reloading 3"),
		QObject::tr("Gun Reloading 4"),
		QObject::tr("Gun Reloading 5"),
		QObject::tr("Gun Reloading 6"),
		QObject::tr("Gun Reloading 7"),
		QObject::tr("Gun Reloading 8"),
		QObject::tr("Gun Reloading 9"),
		QObject::tr("Gun Reloading 10"),
	};
	return stateNames[state];
}

QString AuScTurret::ReportDesc() const
{
	return QObject::tr("%1, %2")
		.arg(PartName())
		.arg(g_Au.CommodityName(Module()));
}

QString AuScTurret::SelectedDesc(const AuScParts& parts) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(m_State);
	if(smi != m_StateMeshes.cend())
		return QObject::tr("%1 %2, %3, %L4 faces")
			.arg(FullName(parts))
			.arg(StateName())
			.arg(g_Au.CommodityName(Module()))
			.arg((*smi).size());
	else
		return QObject::tr("%1 %2, %3, No faces")
			.arg(FullName(parts))
			.arg(StateName())
			.arg(g_Au.CommodityName(Module()));
}

QMap<quint8, QString> AuScTurret::StateNames() const
{
	QMap<quint8, QString> stateNames;
	for(quint8 state = Base; state < STATES_Count; state++)
		if(HasState(state))
			stateNames.insert(state, StateName(state));
	return stateNames;
}

QMap<quint8, QString> AuScTurret::AddableStates() const
{
	QMap<quint8, QString> stateNames;
	if(!HasState(Base))
	{
		// Only the base state can be added.
		stateNames.insert(Base, StateName(Base));
	}
	else if(!HasState(Gun))
	{
		// Only the gun state can be added.
		stateNames.insert(Gun, StateName(Gun));
	}
	else
	{
		// All other gun states can be added, except GunnerScreen.
		for(quint8 state = GunFire1; state < STATES_Count; state++)
			if(!HasState(state))
				stateNames.insert(state, StateName(state));
	}
	return stateNames;
}

QMap<quint8, QString> AuScTurret::DeletableStates() const
{
	QMap<quint8, QString> stateNames = StateNames();
	stateNames.remove(Base);			// Cannot remove the 'base' state of turrets.
	return stateNames;
}

bool AuScTurret::AddState(quint8 state)
{
	if(state >= STATES_Count)
		return false;
	if(m_StateMeshes.contains(state))
		return false;

	// Insert a new empty face list.
	m_StateMeshes.insert(state, AuScFaceList());

	// The designer usually wants to start editing this new state right away.
	SetState(state);
	return true;
}

bool AuScTurret::DeleteState(quint8 state)
{
	// Can not delete the base state of the turret.
	if(state == Base)
		return false;

	if(!m_StateMeshes.contains(state))
		return false;

	// Remove the face list.
	m_StateMeshes.remove(state);
	return true;
}

void AuScTurret::MakeScreen(const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane)
{
	AuPoint cloc1 = gridPlane.UnProject(corner1);
	AuPoint cloc2 = gridPlane.UnProject(corner2);
	cloc1.z = 0;
	cloc2.z = 0;

	AuPoint ll(cloc1.Min(cloc2));
	AuPoint ur(cloc1.Max(cloc2));
	AuPoint ul(ll.x, ur.y, 0);
	AuPoint lr(ur.x, ll.y, 0);

	ll = gridPlane.Project(ll);
	ul = gridPlane.Project(ul);
	ur = gridPlane.Project(ur);
	lr = gridPlane.Project(lr);
	AuPoint n = gridPlane.Project(AuPoint(0,0,1));

	AuScPoint sll(ll, 0,0, n);
	AuScPoint sul(ul, 0,1, n);
	AuScPoint sur(ur, 1,1, n);
	AuScPoint slr(lr, 1,0, n);

	AuScFaceList screen;
	screen.AddFace(AuScFaceQuad(sll, sul, sur, slr));

	m_StateMeshes.insert(GunnerScreen, screen);
}

bool AuScTurret::CanDeleteState() const
{
	return State() != Base;
}

void AuScTurret::EdgeSegments(AuPointArray& edges) const
{
	if(State() == GunnerScreen)
	{
		// Show edges for all states.
		for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
			(*smi).EdgeSegments(edges);

		// Gunner seat circle.
		AuPoint seatAxis(0,0,1);
		AuPoint vx;
		seatAxis.ArbitraryAxis(vx);
		AuPoint vy = vx.Cross(seatAxis);
		double grad = 1;
		AuMatrix4x4 gt(m_GunnerLocation, vx, vy);
		AuPoint p(gt.Project(grad, 0, 0));
		for(int v=1; v<=8; v++)
		{
			float ang = v*PI*2/8.;
			AuPoint n(gt.Project(AuPoint(grad,0).RotateZ(ang)));
			edges.append(p);
			edges.append(n);
			p = n;
		}

		if(m_GunnerLocation != m_GunnerFacing)
		{
			edges.append(m_GunnerLocation);
			edges.append(m_GunnerFacing);
		}
	}
	else
		AuScStatePart::EdgeSegments(edges);				// Show edges for current state only.

	if(State() == Base || (State() == GunnerScreen && HasState(Base)))
	{
		// Base circle.
		AuPoint vx;
		m_BaseAxis.ArbitraryAxis(vx);
		AuPoint vy = vx.Cross(m_BaseAxis);
		double brad = FeetPerMeter();
		AuMatrix4x4 bt(m_BasePivot, vx, vy);
		AuPoint p(bt.Project(brad, 0, 0));
		for(int v=1; v<=12; v++)
		{
			float ang = v*PI*2/12.;
			AuPoint n(bt.Project(AuPoint(brad,0).RotateZ(ang)));
			edges.append(p);
			edges.append(n);
			p = n;
		}

		// Base axis.
		edges.append(m_BasePivot);
		edges.append(m_BasePivot + m_BaseAxis * (FeetPerMeter()*2));
	}

	if(State() == Gun || (State() == GunnerScreen && HasState(Gun)))
	{
		// Gun axis.
		edges.append(m_GunPivot - m_GunAxis * FeetPerMeter());
		edges.append(m_GunPivot + m_GunAxis * FeetPerMeter());

		// Gun fire emission
		{
			// Gun emit V.
			AuPoint vecl = m_GunEmitVec.RotateZ(PI/6);
			edges.append(m_GunEmitLoc);
			edges.append(m_GunEmitLoc + vecl * FeetPerMeter());
			AuPoint vecr = m_GunEmitVec.RotateZ(-PI/6);
			edges.append(m_GunEmitLoc);
			edges.append(m_GunEmitLoc + vecr * FeetPerMeter());
			edges.append(m_GunEmitLoc);
			edges.append(m_GunEmitLoc + m_GunEmitVec * FeetPerMeter());

			// Gun emit dashed line.
			double dd=1.5*FeetPerMeter();
			for(int b=0; b<5; b++)
			{
				double n = dd + .5*FeetPerMeter();
				edges.append(m_GunEmitLoc + m_GunEmitVec * dd);
				edges.append(m_GunEmitLoc + m_GunEmitVec * n);
				dd = n + .5*FeetPerMeter();
			}
		}

		// Gun sight
		{
			// Gun sight V.
			AuPoint vecl = m_GunEmitVec.RotateZ(PI/6);
			edges.append(m_GunSightLoc);
			edges.append(m_GunSightLoc + vecl * FeetPerMeter());
			AuPoint vecr = m_GunEmitVec.RotateZ(-PI/6);
			edges.append(m_GunSightLoc);
			edges.append(m_GunSightLoc + vecr * FeetPerMeter());
			edges.append(m_GunSightLoc);
			edges.append(m_GunSightLoc + m_GunEmitVec * FeetPerMeter());

			// Gun sight line.
			edges.append(m_GunSightLoc);
			edges.append(m_GunSightLoc + m_GunEmitVec * FeetPerMeter()*2);
		}
	}
}

void AuScTurret::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	if(State() == GunnerScreen)
	{
		// Get faces for all states.
		RenderTrianglesState(opaqueTris, transparentTris, themeColors, GunnerScreen);	// Get gunner screen with self illumination.
		for(QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.cbegin(); smi != m_StateMeshes.cend(); smi++)
			if(smi.key() != GunnerScreen)
				(*smi).RenderTriangles(opaqueTris, transparentTris, themeColors);
	}
	else
		AuScStatePart::RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScTurret::RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count], quint8 state) const
{
	QHash<quint8, AuScFaceList>::const_iterator smi = m_StateMeshes.find(state);
	if(smi == m_StateMeshes.cend())
		return;
	(*smi).RenderTriangles(opaqueTris, transparentTris, themeColors);
	if(state == AuScTurret::GunnerScreen)
	{
		opaqueTris.SetSelfIllumination(1);
		transparentTris.SetSelfIllumination(1);
	}
}

bool AuScTurret::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	Q_UNUSED(shipHull);
	if(IsOmitted())
		return false;
	return true;
}

void AuScTurret::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	// Turret and Gun
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double gunSurface = 0;
	QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.find(Base);
	if(mi != m_StateMeshes.cend())
		gunSurface += (*mi).Area();
	mi = m_StateMeshes.find(Gun);
	if(mi != m_StateMeshes.cend())
		gunSurface += (*mi).Area();

	// Increase surface value due to hull density.
	gunSurface *= AuScDesign::HullDensityFactor(armor, streamline);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int metal = (int)gunSurface / d_HullPlatesPerUnit;
	if(metal < 1)
		metal = 1;	// Minimum 1 metal per room.
	pc.AddRequirement(hullMaterial, CE_Required, metal);

	// Add parts for the mechanism.
	int mech = metal / 20;
	if(mech < 2)
		mech = 2;
	pc.AddRequirement(CID_MechanicalPart, CE_Required, mech);

	// Screen
	// Calculate metal units needed for the room structure. Each metal unit is 1 cubic foot.
	double screenSurface = 0;
	mi = m_StateMeshes.find(GunnerScreen);
	if(mi != m_StateMeshes.cend())
		screenSurface += (*mi).Area();

	// Increase surface value due to hull density.
	screenSurface *= AuScDesign::HullDensityFactor(0, SLINE_None);

	// Calculate metal requirement from surface value, adjusted for armor and streamlining.
	int plastic = (int)screenSurface / d_HullPlatesPerUnit;
	if(plastic < 1)
		plastic = 1;	// Minimum 1 metal per screen.
	pc.AddRequirement(CID_Plastic, CE_Required, plastic);

	// Add electronic parts.
	int elec = plastic / 10;
	if(!elec)
		elec = 1;
	pc.AddRequirement(CID_ElectronicPart, CE_Required, elec);

	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial == CID_Lumber)
	{
		pc.AddRequirement(CID_HeavyWeapon, CE_Required, 1);
		pc.AddRequirement(CID_Textiles, CE_Required, 1);
	}
	else
	{
		if(Module() != CID_Invalid)
			pc.AddRequirement(Module(), CE_Required, 1);
	}
}

bool AuScTurret::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	bool hitMe = false;
	QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.find(Base);
	if(mi != m_StateMeshes.cend() && (*mi).Sight(eyeLoc, sightLoc, sightSurfaceNormal))
		hitMe = true;

	mi = m_StateMeshes.find(Gun);
	if(mi != m_StateMeshes.cend() && (*mi).Sight(eyeLoc, sightLoc, sightSurfaceNormal))
		hitMe = true;

	mi = m_StateMeshes.find(GunnerScreen);
	if(mi != m_StateMeshes.cend() && (*mi).Sight(eyeLoc, sightLoc, sightSurfaceNormal))
		hitMe = true;

	return hitMe;
}

AuSysId AuScTurret::SightedSystem(quint16 partId) const
{
	return AuSysId(SYS_Turret, partId);
}

AuStation AuScTurret::OperatorStation(const AuPoint& gravityUp) const
{
	QHash<quint8, AuScFaceList>::const_iterator mi = m_StateMeshes.find(GunnerScreen);
	if(mi == m_StateMeshes.cend())
		return AuStation();
	AuPoint o(OperatorLoc());
	AuPoint min,max;
	(*mi).Extents(min,max);
	AuPoint c((min + max)/2);
	AuPoint y = c - o;											// Calculate y to face the screen from the operator loc.
	y.Unit();
	AuPoint x = y.Cross(gravityUp);
	x.Unit();
	y = gravityUp.Cross(x);										// Adjust y to be perpendicular to the gravity up vector.
	y.Unit();
	AuMatrix4x4 opM;
	opM.Translate(o);
	opM.RotateToPlane(x, y);

	if(m_GunnerFacing != o)
	{
		// Calculate facing rotation.
		AuPoint fvec(m_GunnerFacing - o);
		fvec.Unit();
		double fx = fvec.Dot(x);
		double fy = fvec.Dot(y);
		double zang = atan2(fy, fx);
		double zrot = zang - PI/2;
		opM.RotateZ(zrot);
	}

	AuStation::OCCUPANT_POSITION occPos;
	switch(GunnerPos())
	{
#if defined(QT_NO_DEBUG)
	default:
#endif
	case Gunner_Sit:
		occPos = AuStation::Occupant_Sit;
		break;
	case Gunner_SitSwivel3D:
		occPos = AuStation::Occupant_SitSwivel3D;
		break;
	case Gunner_SitStationary:
		occPos = AuStation::Occupant_SitStationary;
		break;
	case Gunner_Stand:
		occPos = AuStation::Occupant_Stand;
		break;
	case Gunner_StandStationary:
		occPos = AuStation::Occupant_StandStationary;
		break;
	};
	return AuStation(StationType(), opM, occPos, 0);
}

GLuint AuScTurret::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txTurretConsole.bmp");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScTurret::TurretBaseMatrix(float baseRot, AuMatrix4x4& m) const
{
	m.LoadIdentity();
	m.Translate(m_BasePivot);
	m.RotateAxis(baseRot, m_BaseAxis);
	m.Translate(-m_BasePivot);
}

void AuScTurret::TurretGunMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const
{
	m.LoadIdentity();
	m.Translate(m_BasePivot);
	m.RotateAxis(baseRot, m_BaseAxis);
	m.Translate(m_GunPivot - m_BasePivot);
	m.RotateAxis(gunRot, m_GunAxis);
	m.Translate(-m_GunPivot);
}

void AuScTurret::TurretSightMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const
{
	AuMatrix4x4 sightM;
	sightM.Translate(m_BasePivot);
	sightM.RotateAxis(baseRot, m_BaseAxis);
	sightM.Translate(m_GunPivot - m_BasePivot);
	sightM.RotateAxis(gunRot, m_GunAxis);
	sightM.Translate(m_GunSightLoc - m_GunPivot);
	AuPoint sightLoc = sightM.Translation();
	AuPoint sightVec = sightM.ProjectVector(m_GunEmitVec);
	AuPoint gunAxis = sightM.ProjectVector(m_GunAxis);

	AuPoint z(-sightVec);
	z.Unit();
	AuPoint x(gunAxis);
	x.Unit();
	AuPoint y(z.Cross(x));
	y.Unit();
	m.LoadIdentity();
	m.Translate(sightLoc);
	m.RotateToPlane(x,y);
}

void AuScTurret::TurretAimMatrix(float baseRot, AuMatrix4x4& m) const
{
	AuMatrix4x4 aimM;
	aimM.Translate(m_BasePivot);
	aimM.RotateAxis(baseRot, m_BaseAxis);
	aimM.Translate(m_GunPivot - m_BasePivot);
	AuPoint gunPivot = aimM.Translation();
	AuPoint emitVec = aimM.ProjectVector(m_GunEmitVec);
	AuPoint gunAxis = aimM.ProjectVector(m_GunAxis);

	AuPoint z(-emitVec);
	z.Unit();
	AuPoint x(gunAxis);
	x.Unit();
	AuPoint y(z.Cross(x));
	y.Unit();
	m.LoadIdentity();
	m.Translate(gunPivot);
	m.RotateToPlane(x,y);
}

void AuScTurret::TurretMuzzleMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const
{
	AuMatrix4x4 emitM;
	emitM.Translate(m_BasePivot);
	emitM.RotateAxis(baseRot, m_BaseAxis);
	emitM.Translate(m_GunPivot - m_BasePivot);
	emitM.RotateAxis(gunRot, m_GunAxis);
	emitM.Translate(m_GunEmitLoc - m_GunPivot);
	AuPoint emitLoc = emitM.Translation();
	AuPoint emitVec = emitM.ProjectVector(m_GunEmitVec);
	AuPoint gunAxis = emitM.ProjectVector(m_GunAxis);

	AuPoint z(emitVec);
	z.Unit();
	AuPoint x(gunAxis);
	x.Unit();
	AuPoint y(z.Cross(x));
	y.Unit();
	m.LoadIdentity();
	m.Translate(emitLoc);
	m.RotateToPlane(x,y);
}

quint64 AuScTurret::MemoryUsage() const
{
	quint64 mem = AuScStatePart::MemoryUsage();
	mem += sizeof(m_BaseAxis);
	mem += sizeof(m_BasePivot);
	mem += sizeof(m_BasePivotMax);
	mem += sizeof(m_BasePivotMin);
	mem += sizeof(m_GunAxis);
	mem += sizeof(m_GunEmitLoc);
	mem += sizeof(m_GunEmitVec);
	mem += sizeof(m_GunnerPosition);
	mem += sizeof(m_GunnerFacing);
	mem += sizeof(m_GunnerLocation);
	mem += sizeof(m_GunPivot);
	mem += sizeof(m_GunPivotMax);
	mem += sizeof(m_GunPivotMin);
	mem += sizeof(m_GunSightLoc);
	mem += sizeof(m_Module);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPodBay::AuScPodBay()
{
}

AuScPodBay::AuScPodBay(const AuScPodBay& c) : AuScIcon(c)
{
}

AuScPodBay::AuScPodBay(const AuScPodBay& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
}

void AuScPodBay::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScPodBay::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScPodBay::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPodBay& c = dynamic_cast<const AuScPodBay&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPodBay::Clone() const
{
	return new AuScPodBay(*this);
}

AuScPart* AuScPodBay::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPodBay(*this, selInfo);
}

QString AuScPodBay::TypeIcon() const
{
	return QString(":data/dp_DropPod.png");
}

QString AuScPodBay::TypeName() const
{
	return QObject::tr("Pod Bay");
}

double AuScPodBay::IconBack() const
{
	return FeetPerMeter()*2.5;
}

double AuScPodBay::IconFore() const
{
	return FeetPerMeter()*2.5;
}

double AuScPodBay::IconLeft() const
{
	return FeetPerMeter()*2.5;
}

double AuScPodBay::IconRight() const
{
	return FeetPerMeter()*2.5;
}

void AuScPodBay::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);

	pc.AddRequirement(CID_Plastic, CE_Required, 1);
	pc.AddRequirement(CID_MechanicalPart, CE_Required, 2);
	pc.AddRequirement(CID_ElectronicPart, CE_Required, 1);
}

bool AuScPodBay::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScSketch::AuScSketch()
{
}

AuScSketch::AuScSketch(const AuScSketch& c) : AuScScreen(c)
{
	m_FilePath = c.m_FilePath;
}

AuScSketch::AuScSketch(const AuScSketch& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
	m_FilePath = c.m_FilePath;
}

void AuScSketch::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
	out << m_FilePath;
}

void AuScSketch::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
	in >> m_FilePath;
}

bool AuScSketch::operator==(const AuScPart& p) const
{
	try
	{
		const AuScSketch& c = dynamic_cast<const AuScSketch&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		if(m_FilePath != c.m_FilePath)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScSketch::Clone() const
{
	return new AuScSketch(*this);
}

AuScPart* AuScSketch::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScSketch(*this, selInfo);
}

QString AuScSketch::TypeIcon() const
{
	return QString(":data/ScSketch.png");
}

QString AuScSketch::TypeName() const
{
	return QObject::tr("Sketch");
}

float AuScSketch::Opacity() const
{
	return m_Face.Opacity();
}

void AuScSketch::SetOpacity(float opacity)
{
	return m_Face.SetOpacity(opacity);
}

GLuint AuScSketch::OwnTexture(AuGLEffects& re) const
{
	QString texName(FilePath());
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScSketch::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

quint64 AuScSketch::MemoryUsage() const
{
	quint64 mem = AuScScreen::MemoryUsage();
	mem += StringMemoryUsage(m_FilePath);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScLaunchTubeJig::AuScLaunchTubeJig()
{
}

AuScLaunchTubeJig::AuScLaunchTubeJig(const AuScLaunchTubeJig& c) : AuScMeshBase(c)
{
}

AuScLaunchTubeJig::AuScLaunchTubeJig(const AuScLaunchTubeJig& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScLaunchTubeJig::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScLaunchTubeJig::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScLaunchTubeJig::operator==(const AuScPart& p) const
{
	try
	{
		const AuScLaunchTubeJig& c = dynamic_cast<const AuScLaunchTubeJig&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScLaunchTubeJig::Clone() const
{
	return new AuScLaunchTubeJig(*this);
}

AuScPart* AuScLaunchTubeJig::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScLaunchTubeJig(*this, selInfo);
}

QString AuScLaunchTubeJig::TypeIcon() const
{
	return QString(":data/ScJig.png");
}

QString AuScLaunchTubeJig::TypeName() const
{
	return QObject::tr("Jig, Launch Tube");
}

void AuScLaunchTubeJig::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScCuttingJig::AuScCuttingJig()
{
}

AuScCuttingJig::AuScCuttingJig(const AuScCuttingJig& c) : AuScMeshBase(c)
{
}

AuScCuttingJig::AuScCuttingJig(const AuScCuttingJig& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScCuttingJig::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScCuttingJig::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScCuttingJig::operator==(const AuScPart& p) const
{
	try
	{
		const AuScCuttingJig& c = dynamic_cast<const AuScCuttingJig&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScCuttingJig::Clone() const
{
	return new AuScCuttingJig(*this);
}

AuScPart* AuScCuttingJig::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScCuttingJig(*this, selInfo);
}

QString AuScCuttingJig::TypeIcon() const
{
	return QString(":data/ScJig.png");
}

QString AuScCuttingJig::TypeName() const
{
	return QObject::tr("Jig, Cutting");
}

void AuScCuttingJig::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScSpaceVehicleLandingSystem::AuScSpaceVehicleLandingSystem()
{
}

AuScSpaceVehicleLandingSystem::AuScSpaceVehicleLandingSystem(const AuScSpaceVehicleLandingSystem& c) : AuScIcon(c)
{
}

AuScSpaceVehicleLandingSystem::AuScSpaceVehicleLandingSystem(const AuScSpaceVehicleLandingSystem& c, const AuScSelectInfo& selInfo) :
	AuScIcon(c, selInfo)
{
}

void AuScSpaceVehicleLandingSystem::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScSpaceVehicleLandingSystem::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScSpaceVehicleLandingSystem::operator==(const AuScPart& p) const
{
	try
	{
		const AuScSpaceVehicleLandingSystem& c = dynamic_cast<const AuScSpaceVehicleLandingSystem&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScSpaceVehicleLandingSystem::Clone() const
{
	return new AuScSpaceVehicleLandingSystem(*this);
}

AuScPart* AuScSpaceVehicleLandingSystem::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScSpaceVehicleLandingSystem(*this, selInfo);
}

void AuScSpaceVehicleLandingSystem::Transform(const AuMatrix4x4& t)
{
	AuScIcon::Transform(t);
}

QString AuScSpaceVehicleLandingSystem::TypeIcon() const
{
	return QString(":data/ScLandingSystem.png");
}

QString AuScSpaceVehicleLandingSystem::TypeName() const
{
	return QObject::tr("Space Vehicle Recovery System");
}

// Sizes match AuScSpaceVehicleParkingSpot.
double AuScSpaceVehicleLandingSystem::IconBack() const
{
	return 0;
}

double AuScSpaceVehicleLandingSystem::IconFore() const
{
	return SpacecraftRecoveryLength();
}

double AuScSpaceVehicleLandingSystem::IconLeft() const
{
	return SpacecraftRecoveryWidth()/2;
}

double AuScSpaceVehicleLandingSystem::IconRight() const
{
	return SpacecraftRecoveryWidth()/2;
}

GLuint AuScSpaceVehicleLandingSystem::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/txLandingSystem.png");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

AuPoint AuScSpaceVehicleLandingSystem::VLSEmitVec() const
{
	//DHH This may not be right.
	AuPoint vec((m_Face.Vertex(0) + m_Face.Vertex(3))/2 - VLSEmitLoc());
	vec.Unit();
	return vec;
}

void AuScSpaceVehicleLandingSystem::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);

	pc.AddRequirement(CID_Plastic, CE_Required, 1);
	pc.AddRequirement(CID_ElectronicPart, CE_Required, 2);
}

bool AuScSpaceVehicleLandingSystem::LandingSystemPosition(AuMatrix4x4& landerPosM) const
{
	AuPoint o((m_Face.Vertex(0) + m_Face.Vertex(3))/2);	// Origin is mid point along the bottom edge.
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	landerPosM = AuMatrix4x4(o, x, y);
	return true;
}

bool AuScSpaceVehicleLandingSystem::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops,
		AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScLightBulb::AuScLightBulb()
{
	m_Color = AuGLLight::ColorCarbonArc();
	m_kL = .005f;
	m_kQ = 1.f/(float)(d_SpacecraftLight * d_SpacecraftLight);
	m_CosInnerCone = -1;
	m_CosOuterCone = -1;
}

AuScLightBulb::AuScLightBulb(const AuScLightBulb& c) : AuScIcon(c)
{
	m_Color = c.m_Color;
	m_kL = c.m_kL;
	m_kQ = c.m_kQ;
	m_CosInnerCone = c.m_CosInnerCone;
	m_CosOuterCone = c.m_CosOuterCone;
}

AuScLightBulb::AuScLightBulb(const AuScLightBulb& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
	m_Color = c.m_Color;
	m_kL = c.m_kL;
	m_kQ = c.m_kQ;
	m_CosInnerCone = c.m_CosInnerCone;
	m_CosOuterCone = c.m_CosOuterCone;
}

void AuScLightBulb::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
	out << m_Color;
	out << m_kL;
	out << m_kQ;
	out << m_CosInnerCone;
	out << m_CosOuterCone;
}

void AuScLightBulb::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
	in >> m_Color;
	in >> m_kL;
	in >> m_kQ;
	in >> m_CosInnerCone;
	in >> m_CosOuterCone;
	if(ReadVersion() <= 0x09)
	{
		// Due to a bug, these were broken. There was no way for the user to change the defaults so we can safely fix them.
		m_CosInnerCone = .939692621f;   // 20 degrees
		m_CosOuterCone = .5f;           // 60 degrees
		m_kL = .005f;
	}
}

bool AuScLightBulb::operator==(const AuScPart& p) const
{
	try
	{
		const AuScLightBulb& c = dynamic_cast<const AuScLightBulb&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		if(m_Color != c.m_Color)
			return false;
		if(m_kL != c.m_kL)
			return false;
		if(m_kQ != c.m_kQ)
			return false;
		if(m_CosInnerCone != c.m_CosInnerCone)
			return false;
		if(m_CosOuterCone != c.m_CosOuterCone)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScLightBulb::Clone() const
{
	return new AuScLightBulb(*this);
}

AuScPart* AuScLightBulb::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScLightBulb(*this, selInfo);
}

QString AuScLightBulb::TypeIcon() const
{
	return QString(":data/Light.png");
}

QString AuScLightBulb::TypeName() const
{
	return QObject::tr("Light Bulb");
}

QString AuScLightBulb::SelectedDesc(const AuScParts& parts) const
{
	float atten;
	if(m_kQ > 0)
		atten = sqrtf(1/m_kQ);
	else
		atten = d_SpacecraftLight;
	atten = round(atten/FeetPerMeter());
	if(IsSpotLight())
	{
		float angInner = round(acosf(m_CosInnerCone) * RAD2DEGf);
		float angOuter = round(acosf(m_CosOuterCone) * RAD2DEGf);
		return QString("%1: Spot a%L5m Cone i%2%3 o%4%3")
			.arg(FullName(parts))
			.arg(angInner)
			.arg(g_Au.DegreeSymbol())
			.arg(angOuter)
			.arg(atten);
	}
	else
		return QString("%1: Omni a%L2m")
			.arg(FullName(parts))
			.arg(atten);
}

double AuScLightBulb::IconBack() const
{
	if(IsSpotLight())
		return 0;
	else
	{
		double atten;
		if(m_kQ > 0)
			atten = sqrt(1/m_kQ);
		else
			atten = d_SpacecraftLight;
		return atten;
	}
}

double AuScLightBulb::IconFore() const
{
	double atten;
	if(m_kQ > 0)
		atten = sqrt(1/m_kQ);
	else
		atten = d_SpacecraftLight;
	return atten;
}

double AuScLightBulb::IconLeft() const
{
	double atten;
	if(m_kQ > 0)
		atten = sqrt(1/m_kQ);
	else
		atten = d_SpacecraftLight;
	return atten;
}

double AuScLightBulb::IconRight() const
{
	double atten;
	if(m_kQ > 0)
		atten = sqrt(1/m_kQ);
	else
		atten = d_SpacecraftLight;
	return atten;
}

GLuint AuScLightBulb::OwnTexture(AuGLEffects& re) const
{
	QString texName;
	if(IsSpotLight())
	{
		if(m_CosInnerCone > .906307787f)	// cos 25 degrees
			texName = ":data/txLightHeadlight.png";		// 20 degrees
		else
			texName = ":data/txLightStreetLight.png";	// 30 degrees
	}
	else
		texName = ":data/txLightOmni.png";
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScLightBulb::SetLight(const QColor& color, float kL, float kQ, float cosOuterCone, float cosInnerCone)
{
	m_Color = color;
	m_kL = kL;
	m_kQ = kQ;
	m_CosInnerCone = cosInnerCone;
	m_CosOuterCone = cosOuterCone;
}

void AuScLightBulb::EdgeSegments(AuPointArray& edges) const
{
	AuScIcon::EdgeSegments(edges);
}

AuPoint AuScLightBulb::LightLoc() const
{
	if(IsSpotLight())
		return (m_Face.Vertex(0) + m_Face.Vertex(3))/2;
	else
		return (m_Face.Vertex(0) + m_Face.Vertex(2))/2;
}

AuPoint AuScLightBulb::LightDir() const
{
	AuPoint lb = (m_Face.Vertex(0) + m_Face.Vertex(3))/2;
	AuPoint lt = (m_Face.Vertex(1) + m_Face.Vertex(2))/2;
	AuPoint vec(lt - lb);
	vec.Unit();
	return vec;
}

quint64 AuScLightBulb::MemoryUsage() const
{
	quint64 mem = AuScIcon::MemoryUsage();
	mem += sizeof(m_Color);
	mem += sizeof(m_kL);
	mem += sizeof(m_kQ);
	mem += sizeof(m_CosInnerCone);
	mem += sizeof(m_CosOuterCone);
	return mem;
}

bool AuScLightBulb::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScStatusWorldMap::AuScStatusWorldMap()
{
}

AuScStatusWorldMap::AuScStatusWorldMap(const AuScStatusWorldMap& c) : AuScScreen(c)
{
}

AuScStatusWorldMap::AuScStatusWorldMap(const AuScStatusWorldMap& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScStatusWorldMap::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScStatusWorldMap::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScStatusWorldMap::operator==(const AuScPart& p) const
{
	try
	{
		const AuScStatusWorldMap& c = dynamic_cast<const AuScStatusWorldMap&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScStatusWorldMap::Clone() const
{
	return new AuScStatusWorldMap(*this);
}

AuScPart* AuScStatusWorldMap::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScStatusWorldMap(*this, selInfo);
}

QString AuScStatusWorldMap::TypeIcon() const
{
	return QString(":data/ScStatusWorldMap.png");
}

QString AuScStatusWorldMap::TypeName() const
{
	return QObject::tr("World Map");
}

void AuScStatusWorldMap::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	opaqueTris.SetSelfIllumination(1);
	transparentTris.SetSelfIllumination(1);
}

void AuScStatusWorldMap::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
		COMMODITYID hullMaterial) const
{
	// World map is static in a building, so it doesn't require display screen components.
	// Buildings pass CID_Lumber as their hull material.
	if(hullMaterial != CID_Lumber)
		AuScScreen::ListComponents(pc, streamline, armor, hullMaterial);		// Aboard spacecraft, map is a display screen.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScDecal::AuScDecal()
{
}

AuScDecal::AuScDecal(const AuScDecal& c) : AuScMeshBase(c)
{
}

AuScDecal::AuScDecal(const AuScDecal& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScDecal::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScDecal::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScDecal::operator==(const AuScPart& p) const
{
	try
	{
		const AuScDecal& c = dynamic_cast<const AuScDecal&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScDecal::Clone() const
{
	return new AuScDecal(*this);
}

AuScPart* AuScDecal::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScDecal(*this, selInfo);
}

QString AuScDecal::TypeIcon() const
{
	return QString(":data/ScDecal.png");
}

QString AuScDecal::TypeName() const
{
	return QObject::tr("Decal");
}

float AuScDecal::Opacity() const
{
	if(m_Faces.empty())
		return 1;
	return m_Faces.first()->Opacity();
}

void AuScDecal::SetOpacity(float opacity)
{
	return m_Faces.SetOpacity(opacity);
}

bool AuScDecal::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

AuSysId AuScDecal::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Count;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScEmpireFlag::AuScEmpireFlag()
{
}

AuScEmpireFlag::AuScEmpireFlag(const AuScEmpireFlag& c) : AuScScreen(c)
{
}

AuScEmpireFlag::AuScEmpireFlag(const AuScEmpireFlag& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScEmpireFlag::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScEmpireFlag::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScEmpireFlag::operator==(const AuScPart& p) const
{
	try
	{
		const AuScEmpireFlag& c = dynamic_cast<const AuScEmpireFlag&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScEmpireFlag::Clone() const
{
	return new AuScEmpireFlag(*this);
}

AuScPart* AuScEmpireFlag::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScEmpireFlag(*this, selInfo);
}

QString AuScEmpireFlag::TypeIcon() const
{
	return QString(":data/ImperiumFlag.png");
}

QString AuScEmpireFlag::TypeName() const
{
	return QObject::tr("Empire Flag");
}

float AuScEmpireFlag::Opacity() const
{
	return m_Face.Opacity();
}

void AuScEmpireFlag::SetOpacity(float opacity)
{
	return m_Face.SetOpacity(opacity);
}

void AuScEmpireFlag::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	if(selInfo.m_Faces.contains(0))
	{
		m_Face.SetShine(shine);
		m_Face.SetReflectivity(reflectivity);
	}
}

void AuScEmpireFlag::SetFaceSelfIllumination(float selfIllum)
{
	m_Face.SetSelfIllumination(selfIllum);
}

GLuint AuScEmpireFlag::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/ImperiumFlag.png");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

void AuScEmpireFlag::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

void AuScEmpireFlag::RenderTrianglesScaled(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count], float scaleX, float scaleY) const
{
	if((scaleX == 1 && scaleY == 1) || scaleX <= 0 || scaleY <= 0)
		AuScScreen::RenderTriangles(opaqueTris, transparentTris, themeColors);
	else
	{
		// Scale the face to maintain the aspect ratio of the flag decal, in the plane of the face.
		AuScFaceQuad flatFace(m_Face);
		AuMatrix4x4 t = flatFace.FacePlane();
		flatFace.Transform(t.Inverse());	// Flatten the flag to it's own space, so ll is at 0,0.

		AuPoint c(flatFace.FaceLocation());
		AuScPoint ll = flatFace.Vertex(0);
		AuScPoint ul = flatFace.Vertex(1);
		AuScPoint ur = flatFace.Vertex(2);
		AuScPoint lr = flatFace.Vertex(3);
		ll -= c;
		ul -= c;
		ur -= c;
		lr -= c;
		AuPoint scl(scaleX, scaleY);
		ll.Scale(scl);
		ul.Scale(scl);
		ur.Scale(scl);
		lr.Scale(scl);
		ll += c;
		ul += c;
		ur += c;
		lr += c;

		// Make the resulting face.
		AuScFaceQuad face(ll, ul, ur, lr);
		face.Transform(t);
		face.RenderTriangles(opaqueTris, transparentTris, themeColors);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScEmpireName::AuScEmpireName()
{
}

AuScEmpireName::AuScEmpireName(const AuScEmpireName& c) : AuScScreen(c)
{
}

AuScEmpireName::AuScEmpireName(const AuScEmpireName& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScEmpireName::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScEmpireName::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScEmpireName::operator==(const AuScPart& p) const
{
	try
	{
		const AuScEmpireName& c = dynamic_cast<const AuScEmpireName&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScEmpireName::Clone() const
{
	return new AuScEmpireName(*this);
}

AuScPart* AuScEmpireName::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScEmpireName(*this, selInfo);
}

QString AuScEmpireName::TypeIcon() const
{
	return QString(":data/ScName.png");
}

QString AuScEmpireName::TypeName() const
{
	return QObject::tr("Empire Name");
}

float AuScEmpireName::Opacity() const
{
	return m_Face.Opacity();
}

void AuScEmpireName::SetOpacity(float opacity)
{
	return m_Face.SetOpacity(opacity);
}

void AuScEmpireName::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexEffectIntensity(iv, intensity);
}

void AuScEmpireName::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexGlow(iv, glow);
}

void AuScEmpireName::PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(paint);
}

void AuScEmpireName::PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(theme, opacity);
}

void AuScEmpireName::PaintInUse(QList<QColor>& paintInUse) const
{
	if(m_Face.IsPainted() && !paintInUse.contains(m_Face.Paint()))
		paintInUse.append(m_Face.Paint());
}

void AuScEmpireName::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	if(selInfo.m_Faces.contains(0))
	{
		m_Face.SetShine(shine);
		m_Face.SetReflectivity(reflectivity);
	}
}

void AuScEmpireName::SetFaceSelfIllumination(float selfIllum)
{
	m_Face.SetSelfIllumination(selfIllum);
}

GLuint AuScEmpireName::OwnTexture(AuGLEffects& re) const
{
	QString text = QObject::tr("Empire Name");

	AuGLTextItem textItem;
	re.GetRenderTextItem(textItem, text, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
	return textItem.m_Texture;
}

void AuScEmpireName::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScShipName::AuScShipName()
{
}

AuScShipName::AuScShipName(const AuScShipName& c) : AuScScreen(c)
{
}

AuScShipName::AuScShipName(const AuScShipName& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScShipName::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScShipName::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScShipName::operator==(const AuScPart& p) const
{
	try
	{
		const AuScShipName& c = dynamic_cast<const AuScShipName&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScShipName::Clone() const
{
	return new AuScShipName(*this);
}

AuScPart* AuScShipName::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScShipName(*this, selInfo);
}

QString AuScShipName::TypeIcon() const
{
	return QString(":data/ScName.png");
}

QString AuScShipName::TypeName() const
{
	return QObject::tr("Spacecraft Name");
}

float AuScShipName::Opacity() const
{
	return m_Face.Opacity();
}

void AuScShipName::SetOpacity(float opacity)
{
	return m_Face.SetOpacity(opacity);
}

void AuScShipName::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexEffectIntensity(iv, intensity);
}

void AuScShipName::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexGlow(iv, glow);
}

void AuScShipName::PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(paint);
}

void AuScShipName::PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(theme, opacity);
}

void AuScShipName::PaintInUse(QList<QColor>& paintInUse) const
{
	if(m_Face.IsPainted() && !paintInUse.contains(m_Face.Paint()))
		paintInUse.append(m_Face.Paint());
}

void AuScShipName::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	if(selInfo.m_Faces.contains(0))
	{
		m_Face.SetShine(shine);
		m_Face.SetReflectivity(reflectivity);
	}
}

void AuScShipName::SetFaceSelfIllumination(float selfIllum)
{
	m_Face.SetSelfIllumination(selfIllum);
}

GLuint AuScShipName::OwnTexture(AuGLEffects& re) const
{
	QString text = QObject::tr("Spacecraft Name");

	AuGLTextItem textItem;
	re.GetRenderTextItem(textItem, text, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
	return textItem.m_Texture;
}

void AuScShipName::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScCaptName::AuScCaptName()
{
}

AuScCaptName::AuScCaptName(const AuScCaptName& c) : AuScScreen(c)
{
}

AuScCaptName::AuScCaptName(const AuScCaptName& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
}

void AuScCaptName::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
}

void AuScCaptName::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
}

bool AuScCaptName::operator==(const AuScPart& p) const
{
	try
	{
		const AuScCaptName& c = dynamic_cast<const AuScCaptName&>(p);
		if(!AuScScreen::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScCaptName::Clone() const
{
	return new AuScCaptName(*this);
}

AuScPart* AuScCaptName::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScCaptName(*this, selInfo);
}

QString AuScCaptName::TypeIcon() const
{
	return QString(":data/ScName.png");
}

QString AuScCaptName::TypeName() const
{
	return QObject::tr("Captain's Name");
}

float AuScCaptName::Opacity() const
{
	return m_Face.Opacity();
}

void AuScCaptName::SetOpacity(float opacity)
{
	return m_Face.SetOpacity(opacity);
}

void AuScCaptName::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexEffectIntensity(iv, intensity);
}

void AuScCaptName::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexGlow(iv, glow);
}

void AuScCaptName::PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(paint);
}

void AuScCaptName::PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(theme, opacity);
}

void AuScCaptName::PaintInUse(QList<QColor>& paintInUse) const
{
	if(m_Face.IsPainted() && !paintInUse.contains(m_Face.Paint()))
		paintInUse.append(m_Face.Paint());
}

void AuScCaptName::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	if(selInfo.m_Faces.contains(0))
	{
		m_Face.SetShine(shine);
		m_Face.SetReflectivity(reflectivity);
	}
}

void AuScCaptName::SetFaceSelfIllumination(float selfIllum)
{
	m_Face.SetSelfIllumination(selfIllum);
}

GLuint AuScCaptName::OwnTexture(AuGLEffects& re) const
{
	QString text = QObject::tr("Captain's Name");

	AuGLTextItem textItem;
	re.GetRenderTextItem(textItem, text, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
	return textItem.m_Texture;
}

void AuScCaptName::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScDecalJig::AuScDecalJig()
{
}

AuScDecalJig::AuScDecalJig(const AuScDecalJig& c) : AuScMeshBase(c)
{
}

AuScDecalJig::AuScDecalJig(const AuScDecalJig& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScDecalJig::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScDecalJig::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScDecalJig::operator==(const AuScPart& p) const
{
	try
	{
		const AuScDecalJig& c = dynamic_cast<const AuScDecalJig&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScDecalJig::Clone() const
{
	return new AuScDecalJig(*this);
}

AuScPart* AuScDecalJig::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScDecalJig(*this, selInfo);
}

QString AuScDecalJig::TypeIcon() const
{
	return QString(":data/ScJig.png");
}

QString AuScDecalJig::TypeName() const
{
	return QObject::tr("Jig, Decal");
}

void AuScDecalJig::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScVoid::AuScVoid()
{
	m_PartId = 0;
	m_PartState = 0;
}

AuScVoid::AuScVoid(const AuScVoid& c) : AuScPart(c), m_Faces(c.m_Faces)
{
	m_PartId = c.m_PartId;
	m_PartState = c.m_PartState;
}

AuScVoid::AuScVoid(const AuScVoid& c, const AuScSelectInfo& selInfo) : AuScPart(c), m_Faces(c.m_Faces, selInfo)
{
	m_PartId = c.m_PartId;
	m_PartState = c.m_PartState;
}

void AuScVoid::operator>>(QDataStream& out) const
{
	AuScPart::operator>>(out);

	quint8 hasWhat = m_PartState;
	if(m_PartId)
		hasWhat |= 0x80;

	out << m_Faces;
	out << hasWhat;

	if(hasWhat & 0x80)
		out << m_PartId;
}

void AuScVoid::operator<<(QDataStream& in)
{
	AuScPart::operator<<(in);

	quint8 hasWhat;

	in >> m_Faces;
	in >> hasWhat;

	m_PartState = hasWhat & 0x7f;

	if(hasWhat & 0x80)
		in >> m_PartId;
	else
		m_PartId = 0;
}

bool AuScVoid::operator==(const AuScPart& p) const
{
	try
	{
		const AuScVoid& c = dynamic_cast<const AuScVoid&>(p);
		if(!AuScPart::operator==(c))
			return false;

		if(m_Faces != c.m_Faces)
			return false;
		if(m_PartId != c.m_PartId)
			return false;
		if(m_PartState != c.m_PartState)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuScVoid::EdgeIds(QSet<quint32>& edgeIds) const
{
	m_Faces.EdgeIds(edgeIds);
}

void AuScVoid::FaceIds(QSet<quint32>& faceIds) const
{
	m_Faces.FaceIds(faceIds);
}

void AuScVoid::VertexIds(QSet<quint32>& vertexIds) const
{
	m_Faces.VertexIds(vertexIds);
}

AuScPoint AuScVoid::VertexLoc(quint32 vertexIndex) const
{
	return m_Faces.VertexLocation(vertexIndex);
}

bool AuScVoid::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces);
}

bool AuScVoid::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.InViewRectangle(mvp, pixelRect, selInfo, findVertices, findEdges, findFaces);
}

bool AuScVoid::IsValid() const
{
	return !m_Faces.empty();
}

void AuScVoid::EdgeSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

void AuScVoid::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;
	m_Faces.FaceSpriteLocs(selFaces, selSize, selLocs, unselSize, unselLocs);
}

void AuScVoid::VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;
	m_Faces.VertexSpriteLocs(selVerts, selSize, selLocs, unselSize, unselLocs);
}

void AuScVoid::Transform(const AuMatrix4x4& t)
{
	m_Faces.Transform(t);
}

void AuScVoid::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	m_Faces.Transform(t, selInfo.m_Vertices, selInfo.m_Edges, selInfo.m_Faces);
}

void AuScVoid::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	m_Faces.TransformCopy(t, selInfo);
}

double AuScVoid::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.EdgeDistance(mvp, pixel, snapLoc);
}

double AuScVoid::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.EdgeDistance(mvp, pixel, edgeSeg);
}

double AuScVoid::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.FaceDistance(mvp, pixel, faceIndex, gripLoc);
}

double AuScVoid::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.VertexDistance(mvp, pixel, vertexIndex, snapLoc);
}

bool AuScVoid::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;
	return m_Faces.SelectContiguous(angleTol, selInfo);
}

bool AuScVoid::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectEdgesAt(edgeSeg, selInfo);
}

bool AuScVoid::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectFaceAt(atLoc, selInfo);
}

bool AuScVoid::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectFacesAt(atLoc, selInfo);
}

bool AuScVoid::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAt(atLoc, selInfo);
}

bool AuScVoid::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScVoid::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScVoid::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAtTextureCoord(texLoc, selInfo);
}

bool AuScVoid::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAtTextureCoord(texLoc, selInfo);
}

bool AuScVoid::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectEdgesAt(edgeSeg, selInfo);
}

bool AuScVoid::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectFacesAt(atLoc, selInfo);
}

bool AuScVoid::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAt(atLoc, selInfo);
}

void AuScVoid::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	m_Faces.RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScVoid::MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
		bool closeEnds)
{
	m_Faces.Clear();
	m_Faces.MakeExtrusion(bnd, extVec, gridPlane, steps, closeEnds);
}

void AuScVoid::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSpindle(profile, steps, gridPlane);
}

void AuScVoid::MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSphereLongLat(center, radius, lonSteps, latSteps, gridPlane);
}

void AuScVoid::MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSphereUniform(center, radius, equatorSteps, textureRepetitions, gridPlane);
}

void AuScVoid::MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides)
{
	m_Faces.Clear();
	m_Faces.MakeTube(path, diameter, sides, smoothSides);
}

void AuScVoid::MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides)
{
	m_Faces.Clear();
	m_Faces.MakeTube(path, shape, smoothSides);
}

void AuScVoid::Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
{
	m_Faces.Faces(faces, selInfo);
}

void AuScVoid::Faces(AuScFaceList& faces) const
{
	faces = m_Faces;
}

void AuScVoid::AddFaces(const AuScFaceList& faces)
{
	m_Faces.AddFaces(faces);
}

void AuScVoid::DeleteFaces(const QSet<quint32>& faceIds)
{
	m_Faces.DeleteFaces(faceIds);
}

void AuScVoid::ReverseVertexNormals()
{
	m_Faces.ReverseVertexNormals();
}

void AuScVoid::ReverseFaces()
{
	m_Faces.ReverseTriangleFacing();
	m_Faces.ReverseVertexNormals();
}

void AuScVoid::ReverseFaces(const AuScSelectInfo& selInfo)
{
	m_Faces.ReverseTriangleFacing(selInfo);
	m_Faces.ReverseVertexNormals(selInfo);
}

void AuScVoid::DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions)
{
	m_Faces.DivideFaces(selInfo, divisions);
}

void AuScVoid::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	m_Faces.EffectIntensityVertices(selInfo, intensity);
}

void AuScVoid::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	m_Faces.GlowVertices(selInfo, glow);
}

void AuScVoid::FlattenVertices()
{
	m_Faces.FlattenVertexNormals();
}

void AuScVoid::FlattenVertices(const AuScSelectInfo& selInfo)
{
	m_Faces.FlattenVertexNormals(selInfo);
}

bool AuScVoid::CutFaces(const AuPointArray& cutLine)
{
	return m_Faces.CutFaces(cutLine);
}

bool AuScVoid::PunchFaces(const AuPointArray& cutBnd)
{
	return m_Faces.PunchFaces(cutBnd);
}

void AuScVoid::SetFaceSelfIllumination(float selfIllum)
{
	m_Faces.SetSelfIllumination(selfIllum);
}

void AuScVoid::SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum)
{
	m_Faces.SetSelfIllumination(selInfo, selfIllum);
}

void AuScVoid::SetFaceOpacity(float opacity)
{
	m_Faces.SetOpacity(opacity);
}

void AuScVoid::SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	m_Faces.SetOpacity(selInfo, opacity);
}

void AuScVoid::EffectFaces(SURFACE_EFFECT effect)
{
	m_Faces.SetEffect(effect);
}

void AuScVoid::EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	m_Faces.SetEffect(selInfo, effect);
}

void AuScVoid::EffectFacesCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectCylindrical(selInfo, effect, texRepsAtEquator, texSize, gridPlane);
}

void AuScVoid::EffectFacesProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectProject(selInfo, effect, texSize, gridPlane);
}

void AuScVoid::EffectFacesSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectSpherical(selInfo, effect, texRepsAtEquator, gridPlane);
}

void AuScVoid::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectUnwrap(selInfo, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

SURFACE_EFFECT AuScVoid::Effect(const AuScSelectInfo& selInfo) const
{
	qint8 texI = m_Faces.Effect(selInfo);
	if(texI == -2)
		texI = EFF_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = EFF_Count;	// Face texture indexes differ.
	return (SURFACE_EFFECT)texI;
}

void AuScVoid::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	m_Faces.EffectMove(selInfo, vec);
}

void AuScVoid::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	m_Faces.EffectRotate(selInfo, pivot, angle);
}

void AuScVoid::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	m_Faces.EffectStretch(selInfo, base, scaleX, scaleY);
}

void AuScVoid::TextureFaces(SC_TEXTURE_INDEX texIndex)
{
	m_Faces.SetTextureIndex(texIndex);
}

void AuScVoid::TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex)
{
	m_Faces.SetTextureIndex(selInfo, texIndex);
}

void AuScVoid::TextureFacesCylindrical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureCylindrical(selInfo, texIndex, texRepsAtEquator, texSize, gridPlane);
}

void AuScVoid::TextureFacesProject(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureProject(selInfo, texIndex, texSize, gridPlane);
}

void AuScVoid::TextureFacesSpherical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureSpherical(selInfo, texIndex, texRepsAtEquator, gridPlane);
}

void AuScVoid::TextureUnwrap(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureUnwrap(selInfo, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

void AuScVoid::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	m_Faces.TextureMove(selInfo, vec);
}

void AuScVoid::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	m_Faces.TextureRotate(selInfo, pivot, angle);
}

void AuScVoid::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	m_Faces.TextureStretch(selInfo, base, scaleX, scaleY);
}

SC_TEXTURE_INDEX AuScVoid::TextureIndex(const AuScSelectInfo& selInfo) const
{
	qint8 texI = m_Faces.TextureIndex(selInfo);
	if(texI == -2)
		texI = SCTEXI_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = SCTEXI_Count;	// Face texture indexes differ.
	return (SC_TEXTURE_INDEX)texI;
}

bool AuScVoid::IsInsideVoid(const AuPoint& loc) const
{
	// Surface loc counts as being inside, to allow passage from one void to another, in case someone abuts two voids right 
	// against each other.
	return m_Faces.TestInside(loc) != 1;
}

bool AuScVoid::IsInsideVoid(const AuPoint& loc, const AuScParts& parts) const
{
	if(m_PartId)
	{
		// I am associated with a part.
		AuScParts::const_iterator pi = parts.find(m_PartId);
		if(pi != parts.cend())
		{
			// Found the associated part. It must be in the proper state for me to be a void.
			if((*pi)->State() != m_PartState)
				return false;			// Part is not in the state I care about.
		}
	}

	// Surface loc counts as being inside, to allow passage from one void to another, in case someone abuts two voids right 
	// against each other.
	return m_Faces.TestInside(loc) != 1;
}

bool AuScVoid::IsInsideVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	Q_UNUSED(partId);
	Q_UNUSED(liftStops);
	if(m_PartId)
	{
		// This void is associated with a state of a part.
		AuDoorStateHash::const_iterator di = doorStates.find(m_PartId);
		if(di != doorStates.cend())
		{
			// Associated with a door so nothing is inside this void unless the door is in the desired state.
			if(!CompareAssociatedDoorState((*di).State()))
				return false;				// Door is not in the associated state.
		}
		else
		{
			QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(m_PartId);
			if(gi != landingGearStates.m_States.cend())
			{
				// Associated with a landing gear so nothing is inside this void unless the gear is in the desired state.
				if(!CompareAssociatedLandingGearState(*gi))
					return false;				// Door is not in the associated state.
			}
		}
	}

	// Surface loc counts as being inside, to allow passage from one void to another, in case someone abuts two voids right 
	// against each other.
	return m_Faces.TestInside(loc) != 1;
}

AuPoint AuScVoid::FaceLoc(const AuPoint& loc, const AuPoint& vec, AuPoint& terN, const AuMatrix4x4& stopM) const
{
	Q_UNUSED(stopM);
	AuPoint hitLoc;
	m_Faces.FindNearestFace(loc, vec, hitLoc, terN);
	return hitLoc;
}

void AuScVoid::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

bool AuScVoid::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

AuSysId AuScVoid::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Count;
}

AuPoint AuScVoid::Center() const
{
	AuPoint min,max;
	m_Faces.Extents(min,max);
	return (min + max)/2;
}

bool AuScVoid::ModelExtents(AuPoint& min, AuPoint& max) const
{
	if(m_Faces.empty())
		return false;
	m_Faces.Extents(min, max);
	return true;
}

quint64 AuScVoid::MemoryUsage() const
{
	quint64 mem = AuScPart::MemoryUsage();
	mem += m_Faces.MemoryUsage();
	mem += sizeof(m_PartId);
	mem += sizeof(m_PartState);
	return mem;
}

bool AuScVoid::CheckVoidFacesPlanar() const
{
	// Voids require that all faces be planar. Bent faces do not work with the 'is inside' algorithm.
	return m_Faces.CheckFacesPlanar();
}

bool AuScVoid::FractureFaces(const AuScSelectInfo& selInfo)
{
	return m_Faces.Fracture(selInfo);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScHullVoid::AuScHullVoid()
{
}

AuScHullVoid::AuScHullVoid(const AuScVoid& c) : AuScVoid(c)
{
}

AuScHullVoid::AuScHullVoid(const AuScVoid& c, const AuScSelectInfo& selInfo) : AuScVoid(c)
{
	Q_UNUSED(selInfo);
}

bool AuScHullVoid::operator==(const AuScPart& p) const
{
	try
	{
		const AuScHullVoid& c = dynamic_cast<const AuScHullVoid&>(p);
		if(!AuScVoid::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScHullVoid::Clone() const
{
	return new AuScHullVoid(*this);
}

AuScPart* AuScHullVoid::CloneParts(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return new AuScHullVoid(*this);
}

QString AuScHullVoid::SelectedDesc(const AuScParts& parts) const
{
	return QObject::tr("%1, %L2 faces")
		.arg(FullName(parts))
		.arg(m_Faces.size());
}

QString AuScHullVoid::TypeIcon() const
{
	return QString(":data/ScHullVoid.png");
}

QString AuScHullVoid::TypeName() const
{
	return QObject::tr("Hull Void");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScRoomVoid::AuScRoomVoid()
{
	m_Access = 0;
	m_GravityType = G_Natural;
}

AuScRoomVoid::AuScRoomVoid(const AuScRoomVoid& c) : AuScVoid(c)
{
	m_Access = c.m_Access;
	m_GravityType = c.m_GravityType;
	m_GravityUp = c.m_GravityUp;
}

AuScRoomVoid::AuScRoomVoid(const AuScRoomVoid& c, const AuScSelectInfo& selInfo) : AuScVoid(c)
{
	Q_UNUSED(selInfo);
	m_Access = c.m_Access;
	m_GravityType = c.m_GravityType;
	m_GravityUp = c.m_GravityUp;
}

void AuScRoomVoid::operator>>(QDataStream& out) const
{
	AuScVoid::operator>>(out);
	quint8 hasWhat = m_GravityType;		// Bottom nibble.
	if(m_GravityUp != AuPoint(0,0,1))
		hasWhat |= 0x10;				// Gravity up vector.
	if(m_Access)
		hasWhat |= 0x40;

	out << hasWhat;

	if(hasWhat & 0x10)
		out << m_GravityUp;

	if(hasWhat & 0x40)
		out << m_Access;
}

void AuScRoomVoid::operator<<(QDataStream& in)
{
	AuScVoid::operator<<(in);

	quint8 hasWhat;
	in >> hasWhat;

	m_GravityType = hasWhat & 0x0f;		// Bottom nibble.

	if(hasWhat & 0x10)
	{
		in >> m_GravityUp;
		if(m_GravityUp.IsZero())
			m_GravityUp = AuPoint(0,0,1);
	}
	else
		m_GravityUp = AuPoint(0,0,1);

	if(ReadVersion() < 0x10 && (hasWhat & 0x20))
	{
		QString oldName;
		in >> oldName;
		SetGroupName(oldName);
	}

	if(hasWhat & 0x40)
		in >> m_Access;
	else
		m_Access = 0;
}

bool AuScRoomVoid::operator==(const AuScPart& p) const
{
	try
	{
		const AuScRoomVoid& c = dynamic_cast<const AuScRoomVoid&>(p);
		if(!AuScVoid::operator==(c))
			return false;

		if(m_Access != c.m_Access)
			return false;
		if(m_GravityType != c.m_GravityType)
			return false;
		if(m_GravityUp != c.m_GravityUp)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScRoomVoid::Clone() const
{
	return new AuScRoomVoid(*this);
}

AuScPart* AuScRoomVoid::CloneParts(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return new AuScRoomVoid(*this);
}

QString AuScRoomVoid::SelectedDesc(const AuScParts& parts) const
{
	QString g = GravityName(GravityType());

	QString text = QObject::tr("%1, %L2 faces, G=%3")
		.arg(FullName(parts))
		.arg(m_Faces.size())
		.arg(g);

	HOLD_ACCESS holdAccess = HoldAccess();
	if(holdAccess != HOLD_Inaccessible)
		text += QObject::tr(", Hold=%1").arg(HoldAccessName(holdAccess));

	quint8 restrictAccess = PersonnelAccess();
	if(restrictAccess && restrictAccess != PERSONNEL_Everyone)
	{
		text += QObject::tr(", Restricted=");
		if(restrictAccess & PERSONNEL_Command)
			text += "Off";
		if(restrictAccess & PERSONNEL_Crew)
			text += "Crw";
		if(restrictAccess & PERSONNEL_Troops)
			text += "Trp";
		if(restrictAccess & PERSONNEL_Passengers)
			text += "Pas";
	}

	return text;
}

QString AuScRoomVoid::ReportDesc() const
{
	QString text(PartName());

	HOLD_ACCESS holdAccess = HoldAccess();
	if(holdAccess != HOLD_Inaccessible)
		text += QObject::tr(", %1").arg(HoldAccessName(holdAccess));

	quint8 restrictAccess = PersonnelAccess();
	if(restrictAccess && restrictAccess != PERSONNEL_Everyone)
	{
		text += QObject::tr(", Restricted to ");
		QString res;
		if(restrictAccess & PERSONNEL_Command)
			res = "Officers";
		if(restrictAccess & PERSONNEL_Crew)
			res += ", Crew";
		if(restrictAccess & PERSONNEL_Troops)
			res += ", Troops";
		if(restrictAccess & PERSONNEL_Passengers)
			res += ", Passengers";
		if(res.left(2) == ", ")
			res = res.mid(2);
		text += res;
	}
	return text;
}

QString AuScRoomVoid::TypeIcon() const
{
	return QString(":data/ScRoomVoid.png");
}

QString AuScRoomVoid::TypeName() const
{
	return QObject::tr("Room Void");
}

bool AuScRoomVoid::IsInsideRoomVoid(const AuPoint& loc, const AuScParts& parts) const
{
	return IsInsideVoid(loc, parts);
}

bool AuScRoomVoid::IsInsideRoomVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return IsInsideVoid(partId, loc, doorStates, landingGearStates, liftStops);
}

bool AuScRoomVoid::IsInsideRoomVoid(const AuPoint& loc, double elevate, const AuScParts& parts) const
{
	AuPoint eloc = loc + m_GravityUp * elevate;
	return IsInsideVoid(eloc, parts);
}

bool AuScRoomVoid::IsInsideRoomVoid(quint16 partId, const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	AuPoint eloc = loc + m_GravityUp * elevate;
	return IsInsideVoid(partId, eloc, doorStates, landingGearStates, liftStops);
}

void AuScRoomVoid::Transform(const AuMatrix4x4& t)
{
	AuScVoid::Transform(t);
	m_GravityUp = t.ProjectVector(m_GravityUp);
	m_GravityUp.Unit();
}

void AuScRoomVoid::EdgeSegments(AuPointArray& edges) const
{
	AuScVoid::EdgeSegments(edges);
	if(m_Faces.empty())
		return;

	AuPoint cen = m_Faces[0]->FaceLocation();
	AuPoint top = cen + m_GravityUp * FeetPerMeter();
	AuPoint p;
	m_GravityUp.ArbitraryAxis(p);
	AuPoint ar1 = cen - (m_GravityUp * FeetPerMeter() * .2) + p * (FeetPerMeter() * .1);
	AuPoint ar2 = cen - (m_GravityUp * FeetPerMeter() * .2) - p * (FeetPerMeter() * .1);
	edges.append(top);
	edges.append(ar1);

	edges.append(ar1);
	edges.append(cen);

	edges.append(cen);
	edges.append(ar2);

	edges.append(ar2);
	edges.append(top);
}

bool AuScRoomVoid::RandomVoidLocation(quint16 partId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	int wid = max.x - min.x;
	int len = max.y - min.y;
	int ht = max.z - min.z;
	if(wid < 2 || len < 2 || ht < 2)
		return false;

	// Allow a margin.
	wid -= 1;
	len -= 1;
	ht -= 1;

	AuPoint floorN;
	int tries=20;
	do
	{
		double x = min.x + g_Au.Dice().Roll(1, wid);
		double y = min.y + g_Au.Dice().Roll(1, len);
		double z = min.z + g_Au.Dice().Roll(1, ht);
		AuPoint loc(x,y,z);
		if(IsInsideVoid(partId, loc, doorStates, landingGearStates, liftStops))
		{
			// Found a location inside the void. Move down to just above the floor.
			voidLoc = loc - m_GravityUp * (HeightAboveFloor(loc, m_GravityUp, AuMatrix4x4::IdentityMatrix(), floorN) - .1);
			return true;
		}
	}
	while(tries--);
	return false;
}

double AuScRoomVoid::HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM, AuPoint& floorN) const
{
	AuPoint floorLoc = FaceLoc(loc, -gravityN, floorN, stopM);
	return floorLoc.Distance(loc);
}

double AuScRoomVoid::HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM) const
{
	AuPoint terN;
	AuPoint ceilLoc = FaceLoc(loc, gravityN, terN, stopM);
	return ceilLoc.Distance(loc);
}

void AuScRoomVoid::FacesInHullSilhouette(AuScFaceList& faces) const
{
	faces.AddFaces(m_Faces);
}

bool AuScRoomVoid::IsAccessGranted(quint8 personnelAccess) const
{
	quint8 access = PersonnelAccess();
	if(!access)
		return true;					// No access restrictions; grant access to everybody.
	return personnelAccess | access;	// Grant access if any bits correspond.
}

quint64 AuScRoomVoid::MemoryUsage() const
{
	quint64 mem = AuScVoid::MemoryUsage();
	mem += sizeof(m_Access);
	mem += sizeof(m_GravityType);
	mem += sizeof(m_GravityUp);
	return mem;
}

AuPoint AuScRoomVoid::GravityUp(const AuMatrix4x4& stopM) const
{
	Q_UNUSED(stopM);
	return m_GravityUp;
}

AuPoint AuScRoomVoid::GravityUp(const AuPoint& gravityN, const AuMatrix4x4& stopM) const
{
	Q_UNUSED(stopM);
	if(m_GravityType == G_Artificial)
		return m_GravityUp;
	else
		return gravityN;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPath::AuScPath()
{
	m_PartId = 0;
	m_PartState = 0;
}

AuScPath::AuScPath(const AuScPath& c) : AuScPart(c)
{
	m_PartId = c.m_PartId;
	m_PartState = c.m_PartState;
	m_Segment = c.m_Segment;
}

AuScPath::AuScPath(const AuScPath& c, const AuScSelectInfo& selInfo) : AuScPart(c)
{
	Q_UNUSED(selInfo);
	m_PartId = c.m_PartId;
	m_PartState = c.m_PartState;
	m_Segment = c.m_Segment;
}

void AuScPath::operator>>(QDataStream& out) const
{
	AuScPart::operator>>(out);

	quint8 hasWhat = m_PartState;
	if(m_PartId)
		hasWhat |= 0x80;

	out << m_Segment;
	out << hasWhat;

	if(hasWhat & 0x80)
		out << m_PartId;
}

void AuScPath::operator<<(QDataStream& in)
{
	AuScPart::operator<<(in);

	quint8 hasWhat;

	if(ReadVersion() >= 0x09)
		in >> m_Segment;
	else
	{
		AuSegment seg;
		in >> seg;
		m_Segment = AuScPathSeg(seg);
	}

	if(ReadVersion() >= 0x04)
		in >> hasWhat;
	else
		hasWhat = 0;

	m_PartState = hasWhat & 0x7f;

	if(hasWhat & 0x80)
		in >> m_PartId;
	else
		m_PartId = 0;
}

bool AuScPath::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPath& c = dynamic_cast<const AuScPath&>(p);
		if(!AuScPart::operator==(c))
			return false;

		if(m_PartId != c.m_PartId)
			return false;
		if(m_PartState != c.m_PartState)
			return false;
		if(m_Segment != c.m_Segment)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuScPath::EdgeIds(QSet<quint32>& edgeIds) const
{
	edgeIds.insert(0);
}

void AuScPath::FaceIds(QSet<quint32>& faceIds) const
{
	Q_UNUSED(faceIds);
}

void AuScPath::VertexIds(QSet<quint32>& vertexIds) const
{
	vertexIds.insert(0);
	vertexIds.insert(1);
}

AuScPoint AuScPath::VertexLoc(quint32 vertexIndex) const
{
	return AuScPoint(m_Segment.Vertex(vertexIndex), 0,0, AuPoint(0,0,1));
}

bool AuScPath::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	Q_UNUSED(findFaces);
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyInRect = false;

	if(findVertices)
	{
		// Test all projected vertices to see if they are inside the pixel rectangle.
		for(quint32 vindex=0; vindex<2; vindex++)
		{
			AuPoint vloc = mvp.Project(m_Segment.Vertex(vindex));
			if(vloc.z > 0 && pixelRect.contains(QPointF(vloc.x, vloc.y)))
				anyInRect = true;
		}
	}

	if(findEdges)
	{
		AuSegment seg(m_Segment.m_Seg);
		seg.Transform(mvp);
		if((seg.m_fLoc1.z > 0 || seg.m_fLoc2.z > 0) &&
				pixelRect.contains(QPointF(seg.m_fLoc1.x, seg.m_fLoc1.y)) &&
				pixelRect.contains(QPointF(seg.m_fLoc2.x, seg.m_fLoc2.y)))
			anyInRect = true;
	}

	return anyInRect;
}

bool AuScPath::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	Q_UNUSED(findFaces);
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyInRect = false;

	if(findVertices)
	{
		// Test all projected vertices to see if they are inside the pixel rectangle.
		for(quint32 vindex=0; vindex<2; vindex++)
		{
			AuPoint vloc = mvp.Project(m_Segment.Vertex(vindex));
			if(vloc.z > 0 && pixelRect.contains(QPointF(vloc.x, vloc.y)))
			{
				selInfo.m_Vertices.insert(vindex);
				anyInRect = true;
			}
		}
	}

	if(findEdges)
	{
		AuSegment seg(m_Segment.m_Seg);
		seg.Transform(mvp);
		if((seg.m_fLoc1.z > 0 || seg.m_fLoc2.z > 0) &&
				pixelRect.contains(QPointF(seg.m_fLoc1.x, seg.m_fLoc1.y)) &&
				pixelRect.contains(QPointF(seg.m_fLoc2.x, seg.m_fLoc2.y)))
		{
			selInfo.m_Edges.insert(0);
			anyInRect = true;
		}
	}

	return anyInRect;
}

bool AuScPath::IsValid() const
{
	return !m_Segment.Loc1().FuzzyEquals(m_Segment.Loc2());
}

void AuScPath::EdgeSegments(AuPointArray& edges) const
{
	edges.append(m_Segment.Loc1());
	edges.append(m_Segment.Loc2());
}

void AuScPath::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	Q_UNUSED(selFaces);
	Q_UNUSED(selLocs);
	Q_UNUSED(unselLocs);
}

void AuScPath::VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;

	for(quint32 vindex=0; vindex<2; vindex++)
		if(selVerts.contains(vindex))
			selLocs.append(AuPointAlpha(m_Segment.Vertex(vindex), selSize));
		else
			unselLocs.append(AuPointAlpha(m_Segment.Vertex(vindex), selSize));
}

void AuScPath::Transform(const AuMatrix4x4& t)
{
	m_Segment.Transform(t);
}

void AuScPath::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	if(selInfo.m_Vertices.contains(0) && selInfo.m_Vertices.contains(1))
		Transform(t);
	else if(selInfo.m_Vertices.contains(0))
		m_Segment.m_Seg.m_fLoc1 = t.Project(m_Segment.Loc1());
	else if(selInfo.m_Vertices.contains(1))
		m_Segment.m_Seg.m_fLoc2 = t.Project(m_Segment.Loc2());
}

void AuScPath::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	if(selInfo.m_Vertices.contains(0) && selInfo.m_Vertices.contains(1))
		Transform(t);
	else if(selInfo.m_Vertices.contains(0))
		m_Segment.m_Seg.m_fLoc1 = t.Project(m_Segment.Loc1());
	else if(selInfo.m_Vertices.contains(1))
		m_Segment.m_Seg.m_fLoc2 = t.Project(m_Segment.Loc2());
}

double AuScPath::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	if(snapLoc)
		return m_Segment.m_Seg.ViewDistance(mvp, pixel, *snapLoc, -1);

	AuPoint edgeLoc;
	return m_Segment.m_Seg.ViewDistance(mvp, pixel, edgeLoc, -1);
}

double AuScPath::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint edgeLoc;
	double dseg = m_Segment.m_Seg.ViewDistance(mvp, pixel, edgeLoc, -1);
	if(dseg >= d_SearchTolerance)
		return d_SearchTolerance;

	edgeSeg = m_Segment.m_Seg;
	return dseg;
}

double AuScPath::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	Q_UNUSED(mvp);
	Q_UNUSED(pixel);
	Q_UNUSED(faceIndex);
	Q_UNUSED(gripLoc);
	return d_SearchTolerance;
}

double AuScPath::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint fpx(pixel);
	double closestDist2 = d_SearchTolerance*d_SearchTolerance;	// We won't find anything that is not closer than this dist.
	bool foundMe = false;

	for(quint32 vindex=0; vindex<2; vindex++)
	{
		AuPoint vloc = mvp.Project(m_Segment.Vertex(vindex));
		if(vloc.z < 0)
			continue;		// Behind eye plane.

		double dist2 = vloc.Distance2_2D(fpx);
		if(dist2 < closestDist2)
		{
			closestDist2 = dist2;
			vertexIndex = vindex;
			if(snapLoc)
				*snapLoc = m_Segment.Vertex(vindex);
			foundMe = true;
		}
	}

	if(!foundMe)
		return d_SearchTolerance;

	return sqrt(closestDist2);
}

bool AuScPath::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(angleTol);
	Q_UNUSED(selInfo);
	return false;	// No faces.
}

bool AuScPath::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	if(selInfo.m_Edges.contains(0))
		return false;
	
	if(!m_Segment.m_Seg.IsSegment(edgeSeg))
		return false;

	selInfo.m_Edges.insert(0);
	return true;
}

bool AuScPath::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(atLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScPath::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(atLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScPath::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	for(quint32 vindex=0; vindex<2; vindex++)
	{
		if(m_Segment.Vertex(vindex).FuzzyEquals(atLoc) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.insert(vindex);
			anyChange = true;
		}
	}

	return anyChange;
}

bool AuScPath::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	if(selInfo.m_Edges.contains(0))
		return false;
	
	if(!m_Segment.m_Seg.IsSegment(edgeSeg))
		return false;

	selInfo.m_Edges.remove(0);
	return true;
}

bool AuScPath::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(atLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScPath::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	for(quint32 vindex=0; vindex<2; vindex++)
	{
		if(m_Segment.Vertex(vindex).FuzzyEquals(atLoc) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.insert(vindex);
			anyChange = true;
		}
	}

	return anyChange;
}

QString AuScPath::SelectedDesc(const AuScParts& parts) const
{
	return QObject::tr("%1")
		.arg(FullName(parts));
}

void AuScPath::MakePath(const AuPoint& start, const AuPoint& end)
{
	m_Segment.m_Seg.m_fLoc1 = start;
	m_Segment.m_Seg.m_fLoc2 = end;
}

void AuScPath::SetFaceSelfIllumination(float selfIllum)
{
	Q_UNUSED(selfIllum);
}

void AuScPath::SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(selfIllum);
}

void AuScPath::SetFaceOpacity(float opacity)
{
	Q_UNUSED(opacity);
}

void AuScPath::SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(opacity);
}

void AuScPath::EffectFaces(SURFACE_EFFECT effect)
{
	Q_UNUSED(effect);
}

void AuScPath::EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
}

void AuScPath::EffectFacesCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScPath::EffectFacesProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScPath::EffectFacesSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(gridPlane);
}

void AuScPath::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texSize);
	Q_UNUSED(unwrap);
	Q_UNUSED(angleTol);
	Q_UNUSED(gridAlign);
	Q_UNUSED(alignAxis);
	Q_UNUSED(gridPlane);
}

SURFACE_EFFECT AuScPath::Effect(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return EFF_Count;
}

void AuScPath::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(vec);
}

void AuScPath::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(pivot);
	Q_UNUSED(angle);
}

void AuScPath::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(base);
	Q_UNUSED(scaleX);
	Q_UNUSED(scaleY);
}

void AuScPath::TextureFaces(SC_TEXTURE_INDEX texIndex)
{
	Q_UNUSED(texIndex);
}

void AuScPath::TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
}

void AuScPath::TextureFacesCylindrical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScPath::TextureFacesProject(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScPath::TextureFacesSpherical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(gridPlane);
}

void AuScPath::TextureUnwrap(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texSize);
	Q_UNUSED(unwrap);
	Q_UNUSED(angleTol);
	Q_UNUSED(gridAlign);
	Q_UNUSED(alignAxis);
	Q_UNUSED(gridPlane);
}

void AuScPath::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(vec);
}

void AuScPath::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(pivot);
	Q_UNUSED(angle);
}

void AuScPath::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(base);
	Q_UNUSED(scaleX);
	Q_UNUSED(scaleY);
}

SC_TEXTURE_INDEX AuScPath::TextureIndex(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return SCTEXI_Count;
}

bool AuScPath::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScPath::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScPath::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScPath::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

void AuScPath::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

bool AuScPath::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	Q_UNUSED(shipHull);
	if(IsOmitted())
		return false;
	return true;
}

bool AuScPath::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

AuSysId AuScPath::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Count;
}

AuPoint AuScPath::Center() const
{
	return m_Segment.m_Seg.CenterPoint();
}

bool AuScPath::ModelExtents(AuPoint& min, AuPoint& max) const
{
	min = m_Segment.m_Seg.m_fLoc1;
	max = m_Segment.m_Seg.m_fLoc1;
	min = min.Min(m_Segment.m_Seg.m_fLoc2);
	max = max.Max(m_Segment.m_Seg.m_fLoc2);
	return true;
}

quint64 AuScPath::MemoryUsage() const
{
	quint64 mem = AuScPart::MemoryUsage();
	mem += sizeof(m_PartId);
	mem += sizeof(m_PartState);
	mem += m_Segment.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPathWalk::AuScPathWalk()
{
}

AuScPathWalk::AuScPathWalk(const AuScPathWalk& c) : AuScPath(c)
{
}

AuScPathWalk::AuScPathWalk(const AuScPathWalk& c, const AuScSelectInfo& selInfo) : AuScPath(c, selInfo)
{
}

void AuScPathWalk::operator>>(QDataStream& out) const
{
	AuScPath::operator>>(out);
}

void AuScPathWalk::operator<<(QDataStream& in)
{
	AuScPath::operator<<(in);
}

bool AuScPathWalk::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPathWalk& c = dynamic_cast<const AuScPathWalk&>(p);
		if(!AuScPath::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPathWalk::Clone() const
{
	return new AuScPathWalk(*this);
}

AuScPart* AuScPathWalk::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPathWalk(*this, selInfo);
}

QString AuScPathWalk::TypeIcon() const
{
	return QString(":data/ScWalkPath.png");
}

QString AuScPathWalk::TypeName() const
{
	return QObject::tr("Walk Path");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPathLadder::AuScPathLadder()
{
	m_Segment.m_Ladder = true;
}

AuScPathLadder::AuScPathLadder(const AuScPathLadder& c) : AuScPath(c)
{
	m_Facing = c.m_Facing;
}

AuScPathLadder::AuScPathLadder(const AuScPathLadder& c, const AuScSelectInfo& selInfo) : AuScPath(c, selInfo)
{
	m_Facing = c.m_Facing;
}

void AuScPathLadder::operator>>(QDataStream& out) const
{
	AuScPath::operator>>(out);
	out << m_Facing;
}

void AuScPathLadder::operator<<(QDataStream& in)
{
	AuScPath::operator<<(in);
	in >> m_Facing;
	m_Segment.m_Ladder = true;
}

bool AuScPathLadder::operator==(const AuScPart& p) const
{
	try
	{
		const AuScPathLadder& c = dynamic_cast<const AuScPathLadder&>(p);
		if(!AuScPath::operator==(c))
			return false;

		if(m_Facing != c.m_Facing)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScPathLadder::Clone() const
{
	return new AuScPathLadder(*this);
}

AuScPart* AuScPathLadder::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScPathLadder(*this, selInfo);
}

void AuScPathLadder::Transform(const AuMatrix4x4& t)
{
	AuScPath::Transform(t);
	m_Facing = t.ProjectVector(m_Facing);
	m_Facing.Unit();
}

void AuScPathLadder::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	AuScPath::Transform(t, selInfo);
}

void AuScPathLadder::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	AuScPath::TransformCopy(t, selInfo);
}

QString AuScPathLadder::TypeIcon() const
{
	return QString(":data/ScLadderPath.png");
}

QString AuScPathLadder::TypeName() const
{
	return QObject::tr("Ladder Path");
}

void AuScPathLadder::EdgeSegments(AuPointArray& edges) const
{
	AuScPath::EdgeSegments(edges);

	edges.append(m_Segment.Loc1());
	edges.append(m_Segment.Loc1() + m_Facing * FeetPerMeter());

	edges.append(m_Segment.Loc2());
	edges.append(m_Segment.Loc2() + m_Facing * FeetPerMeter());
}

bool AuScPathLadder::FindLadder(const AuPoint& fromLoc, AuMatrix4x4& mLadder, double& dist)
{
	AuPoint pint = fromLoc.PointOnSegment(m_Segment.Loc1(), m_Segment.Loc2());
	double ldist = fromLoc.Distance(pint);
	if(ldist >= dist)
		return false;

	// Calculate the matrix to align to this ladder segment.
	AuPoint n = m_Segment.Loc2() - m_Segment.Loc1();
	n.Unit();

	AuPoint x = m_Facing.Cross(n);
	x.Unit();

	mLadder = AuMatrix4x4(pint, x, m_Facing);
	return true;
}

quint64 AuScPathLadder::MemoryUsage() const
{
	quint64 mem = AuScPath::MemoryUsage();
	mem += sizeof(m_Facing);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScWaterVehicleParkingSpot::AuScWaterVehicleParkingSpot()
{
}

AuScWaterVehicleParkingSpot::AuScWaterVehicleParkingSpot(const AuScWaterVehicleParkingSpot& c) : AuScIcon(c)
{
}

AuScWaterVehicleParkingSpot::AuScWaterVehicleParkingSpot(const AuScWaterVehicleParkingSpot& c, const AuScSelectInfo& selInfo) :
	AuScIcon(c, selInfo)
{
}

void AuScWaterVehicleParkingSpot::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScWaterVehicleParkingSpot::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScWaterVehicleParkingSpot::operator==(const AuScPart& p) const
{
	try
	{
		const AuScWaterVehicleParkingSpot& c = dynamic_cast<const AuScWaterVehicleParkingSpot&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScWaterVehicleParkingSpot::Clone() const
{
	return new AuScWaterVehicleParkingSpot(*this);
}

AuScPart* AuScWaterVehicleParkingSpot::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScWaterVehicleParkingSpot(*this, selInfo);
}

QString AuScWaterVehicleParkingSpot::TypeIcon() const
{
	return QString(":data/ScParkWaterVehicle.png");
}

QString AuScWaterVehicleParkingSpot::TypeName() const
{
	return QObject::tr("Water Vehicle Parking Spot");
}

// PT-109 is 24m long (79'), beam is 6m (20').
// Sizes match AuScWaterVehicleLauncher.
double AuScWaterVehicleParkingSpot::IconBack() const
{
	return FeetPerMeter()*12;
}

double AuScWaterVehicleParkingSpot::IconFore() const
{
	return FeetPerMeter()*12;
}

double AuScWaterVehicleParkingSpot::IconLeft() const
{
	return FeetPerMeter()*6;
}

double AuScWaterVehicleParkingSpot::IconRight() const
{
	return FeetPerMeter()*6;
}

AuMatrix4x4 AuScWaterVehicleParkingSpot::PositionMatrix() const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(o, x, y);
}

void AuScWaterVehicleParkingSpot::BuildingCursorSegments(AuPointArray& edges) const
{
	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(1));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(2));
	edges.append(m_Face.Vertex(3));

	edges.append(m_Face.Vertex(3));
	edges.append(m_Face.Vertex(0));

	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(3));
}

bool AuScWaterVehicleParkingSpot::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBarrier::AuScBarrier()
{
	m_PartId = 0;
	m_PartState = 0;
}

AuScBarrier::AuScBarrier(const AuScBarrier& c) : AuScPart(c), m_Faces(c.m_Faces), m_Silhouette(c.m_Silhouette)
{
	m_PartId = c.m_PartId;
	m_PartState = c.m_PartState;
}

AuScBarrier::AuScBarrier(const AuScBarrier& c, const AuScSelectInfo& selInfo) : AuScPart(c), m_Faces(c.m_Faces, selInfo),
	m_Silhouette(c.m_Silhouette)
{
	m_PartId = c.m_PartId;
	m_PartState = c.m_PartState;
}

void AuScBarrier::operator>>(QDataStream& out) const
{
	AuScPart::operator>>(out);

	quint8 hasWhat = m_PartState;
	if(m_PartId)
		hasWhat |= 0x80;
	hasWhat |= 0x40;			// silhouette added

	out << m_Faces;
	out << hasWhat;

	if(hasWhat & 0x80)
		out << m_PartId;
	out << m_Silhouette;
}

void AuScBarrier::operator<<(QDataStream& in)
{
	AuScPart::operator<<(in);

	quint8 hasWhat;

	in >> m_Faces;
	in >> hasWhat;

	m_PartState = hasWhat & 0x7f;

	if(hasWhat & 0x80)
		in >> m_PartId;
	else
		m_PartId = 0;

	if(hasWhat & 0x40)
	{
		if(ReadVersion() >= 0x0d)
			in >> m_Silhouette;
		else
			m_Silhouette.ReadOld(in);
	}
}

bool AuScBarrier::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBarrier& c = dynamic_cast<const AuScBarrier&>(p);
		if(!AuScPart::operator==(c))
			return false;

		if(m_Faces != c.m_Faces)
			return false;
		if(m_PartId != c.m_PartId)
			return false;
		if(m_PartState != c.m_PartState)
			return false;
		//if(m_Silhouette != c.m_Silhouette)
			//return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScBarrier::Clone() const
{
	return new AuScBarrier(*this);
}

AuScPart* AuScBarrier::CloneParts(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return new AuScBarrier(*this);
}

void AuScBarrier::EdgeIds(QSet<quint32>& edgeIds) const
{
	m_Faces.EdgeIds(edgeIds);
}

void AuScBarrier::FaceIds(QSet<quint32>& faceIds) const
{
	m_Faces.FaceIds(faceIds);
}

void AuScBarrier::VertexIds(QSet<quint32>& vertexIds) const
{
	m_Faces.VertexIds(vertexIds);
}

AuScPoint AuScBarrier::VertexLoc(quint32 vertexIndex) const
{
	return m_Faces.VertexLocation(vertexIndex);
}

bool AuScBarrier::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces);
}

bool AuScBarrier::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.InViewRectangle(mvp, pixelRect, selInfo, findVertices, findEdges, findFaces);
}

bool AuScBarrier::IsValid() const
{
	return !m_Faces.empty();
}

void AuScBarrier::EdgeSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

void AuScBarrier::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;
	m_Faces.FaceSpriteLocs(selFaces, selSize, selLocs, unselSize, unselLocs);
}

void AuScBarrier::VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;
	m_Faces.VertexSpriteLocs(selVerts, selSize, selLocs, unselSize, unselLocs);
}

void AuScBarrier::Transform(const AuMatrix4x4& t)
{
	m_Faces.Transform(t);
}

void AuScBarrier::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	m_Faces.Transform(t, selInfo.m_Vertices, selInfo.m_Edges, selInfo.m_Faces);
}

void AuScBarrier::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	m_Faces.TransformCopy(t, selInfo);
}

double AuScBarrier::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.EdgeDistance(mvp, pixel, snapLoc);
}

double AuScBarrier::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.EdgeDistance(mvp, pixel, edgeSeg);
}

double AuScBarrier::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.FaceDistance(mvp, pixel, faceIndex, gripLoc);
}

double AuScBarrier::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	return m_Faces.VertexDistance(mvp, pixel, vertexIndex, snapLoc);
}

bool AuScBarrier::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;
	return m_Faces.SelectContiguous(angleTol, selInfo);
}

bool AuScBarrier::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectEdgesAt(edgeSeg, selInfo);
}

bool AuScBarrier::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectFaceAt(atLoc, selInfo);
}

bool AuScBarrier::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectFacesAt(atLoc, selInfo);
}

bool AuScBarrier::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAt(atLoc, selInfo);
}

bool AuScBarrier::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScBarrier::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAtEffectCoord(texLoc, selInfo);
}

bool AuScBarrier::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAtTextureCoord(texLoc, selInfo);
}

bool AuScBarrier::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	if(IsHidden())
		return false;

	return m_Faces.SelectVerticesAtTextureCoord(texLoc, selInfo);
}

bool AuScBarrier::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectEdgesAt(edgeSeg, selInfo);
}

bool AuScBarrier::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectFacesAt(atLoc, selInfo);
}

bool AuScBarrier::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	return m_Faces.DeselectVerticesAt(atLoc, selInfo);
}

void AuScBarrier::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	m_Faces.RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScBarrier::MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
		bool closeEnds)
{
	m_Faces.Clear();
	m_Faces.MakeExtrusion(bnd, extVec, gridPlane, steps, closeEnds);
}

void AuScBarrier::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSpindle(profile, steps, gridPlane);
}

void AuScBarrier::MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSphereLongLat(center, radius, lonSteps, latSteps, gridPlane);
}

void AuScBarrier::MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();
	m_Faces.MakeSphereUniform(center, radius, equatorSteps, textureRepetitions, gridPlane);
}

void AuScBarrier::MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides)
{
	m_Faces.Clear();
	m_Faces.MakeTube(path, diameter, sides, smoothSides);
}

void AuScBarrier::MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides)
{
	m_Faces.Clear();
	m_Faces.MakeTube(path, shape, smoothSides);
}

void AuScBarrier::Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
{
	m_Faces.Faces(faces, selInfo);
}

void AuScBarrier::Faces(AuScFaceList& faces) const
{
	faces = m_Faces;
}

void AuScBarrier::AddFaces(const AuScFaceList& faces)
{
	m_Faces.AddFaces(faces);
}

void AuScBarrier::DeleteFaces(const QSet<quint32>& faceIds)
{
	m_Faces.DeleteFaces(faceIds);
}

void AuScBarrier::ReverseVertexNormals()
{
	m_Faces.ReverseVertexNormals();
}

void AuScBarrier::ReverseFaces()
{
	m_Faces.ReverseTriangleFacing();
	m_Faces.ReverseVertexNormals();
}

void AuScBarrier::ReverseFaces(const AuScSelectInfo& selInfo)
{
	m_Faces.ReverseTriangleFacing(selInfo);
	m_Faces.ReverseVertexNormals(selInfo);
}

void AuScBarrier::DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions)
{
	m_Faces.DivideFaces(selInfo, divisions);
}

void AuScBarrier::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	m_Faces.EffectIntensityVertices(selInfo, intensity);
}

void AuScBarrier::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	m_Faces.GlowVertices(selInfo, glow);
}

void AuScBarrier::FlattenVertices()
{
	m_Faces.FlattenVertexNormals();
}

void AuScBarrier::FlattenVertices(const AuScSelectInfo& selInfo)
{
	m_Faces.FlattenVertexNormals(selInfo);
}

bool AuScBarrier::CutFaces(const AuPointArray& cutLine)
{
	return m_Faces.CutFaces(cutLine);
}

bool AuScBarrier::PunchFaces(const AuPointArray& cutBnd)
{
	return m_Faces.PunchFaces(cutBnd);
}

void AuScBarrier::SetFaceSelfIllumination(float selfIllum)
{
	m_Faces.SetSelfIllumination(selfIllum);
}

void AuScBarrier::SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum)
{
	m_Faces.SetSelfIllumination(selInfo, selfIllum);
}

void AuScBarrier::SetFaceOpacity(float opacity)
{
	m_Faces.SetOpacity(opacity);
}

void AuScBarrier::SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	m_Faces.SetOpacity(selInfo, opacity);
}

void AuScBarrier::EffectFaces(SURFACE_EFFECT effect)
{
	m_Faces.SetEffect(effect);
}

void AuScBarrier::EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	m_Faces.SetEffect(selInfo, effect);
}

void AuScBarrier::EffectFacesCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectCylindrical(selInfo, effect, texRepsAtEquator, texSize, gridPlane);
}

void AuScBarrier::EffectFacesProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectProject(selInfo, effect, texSize, gridPlane);
}

void AuScBarrier::EffectFacesSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectSpherical(selInfo, effect, texRepsAtEquator, gridPlane);
}

void AuScBarrier::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	m_Faces.EffectUnwrap(selInfo, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

SURFACE_EFFECT AuScBarrier::Effect(const AuScSelectInfo& selInfo) const
{
	qint8 texI = m_Faces.Effect(selInfo);
	if(texI == -2)
		texI = EFF_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = EFF_Count;	// Face texture indexes differ.
	return (SURFACE_EFFECT)texI;
}

void AuScBarrier::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	m_Faces.EffectMove(selInfo, vec);
}

void AuScBarrier::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	m_Faces.EffectRotate(selInfo, pivot, angle);
}

void AuScBarrier::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	m_Faces.EffectStretch(selInfo, base, scaleX, scaleY);
}

void AuScBarrier::TextureFaces(SC_TEXTURE_INDEX texIndex)
{
	m_Faces.SetTextureIndex(texIndex);
}

void AuScBarrier::TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex)
{
	m_Faces.SetTextureIndex(selInfo, texIndex);
}

void AuScBarrier::TextureFacesCylindrical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureCylindrical(selInfo, texIndex, texRepsAtEquator, texSize, gridPlane);
}

void AuScBarrier::TextureFacesProject(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureProject(selInfo, texIndex, texSize, gridPlane);
}

void AuScBarrier::TextureFacesSpherical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureSpherical(selInfo, texIndex, texRepsAtEquator, gridPlane);
}

void AuScBarrier::TextureUnwrap(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	m_Faces.TextureUnwrap(selInfo, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
}

void AuScBarrier::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	m_Faces.TextureMove(selInfo, vec);
}

void AuScBarrier::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	m_Faces.TextureRotate(selInfo, pivot, angle);
}

void AuScBarrier::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	m_Faces.TextureStretch(selInfo, base, scaleX, scaleY);
}

SC_TEXTURE_INDEX AuScBarrier::TextureIndex(const AuScSelectInfo& selInfo) const
{
	qint8 texI = m_Faces.TextureIndex(selInfo);
	if(texI == -2)
		texI = SCTEXI_Count+1;	// No faces in the selection.
	else if(texI == -1)
		texI = SCTEXI_Count;	// Face texture indexes differ.
	return (SC_TEXTURE_INDEX)texI;
}

AuPoint AuScBarrier::FaceLoc(const AuPoint& loc, const AuPoint& vec, AuPoint& terN, const AuMatrix4x4& stopM) const
{
	Q_UNUSED(stopM);
	AuPoint hitLoc;
	m_Faces.FindNearestFace(loc, vec, hitLoc, terN);
	return hitLoc;
}

void AuScBarrier::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

bool AuScBarrier::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	return m_Faces.Sight(eyeLoc, sightLoc, sightSurfaceNormal);
}

AuSysId AuScBarrier::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Hull;
}

void AuScBarrier::RefreshObstructionsCache(quint16 partId, const AuScParts& parts)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	m_Silhouette.Build(m_Faces, (float)d_ObstructMargin);
}

bool AuScBarrier::RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts)
{
	if(m_Silhouette.IsValid())
		return false;
	RefreshObstructionsCache(partId, parts);
	return true;
}

bool AuScBarrier::IsInsideObstructionModel(const AuPoint& loc) const
{
	return m_Silhouette.IsInside(loc);
}

bool AuScBarrier::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	if(!m_Silhouette.IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal))
		return false;

	// Get precise with barriers.
	AuPoint sLoc(toLoc);
	AuPoint sNorm;
	if(!m_Faces.Sight(fromLoc, sLoc, sNorm))
		return false;
	hitLoc = sLoc;
	hitNormal = sNorm;
	return true;
}

bool AuScBarrier::IsPathBlocked(quint16 partId, const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
		const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	Q_UNUSED(partId);
	Q_UNUSED(liftStops);
	if(m_PartId)
	{
		// This solid is associated with a state of a part.
		AuDoorStateHash::const_iterator di = doorStates.find(m_PartId);
		if(di != doorStates.cend())
		{
			// Associated with a door so nothing is inside this void unless the door is in the desired state.
			if(!CompareAssociatedDoorState((*di).State()))
				return false;				// Door is not in the associated state.
		}
		else
		{
			QHash<quint16, quint8>::const_iterator gi = landingGearStates.m_States.find(m_PartId);
			if(gi != landingGearStates.m_States.cend())
			{
				// Associated with a landing gear so nothing is inside this void unless the gear is in the desired state.
				if(!CompareAssociatedLandingGearState(*gi))
					return false;				// Door is not in the associated state.
			}
		}
	}

	return IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal);
}

QString AuScBarrier::SelectedDesc(const AuScParts& parts) const
{
	return QObject::tr("%1, %L2 faces")
		.arg(FullName(parts))
		.arg(m_Faces.size());
}

QString AuScBarrier::ReportDesc() const
{
	return PartName();
}

QString AuScBarrier::TypeIcon() const
{
	return QString(":data/ScBarrier.png");
}

QString AuScBarrier::TypeName() const
{
	return QObject::tr("Barrier");
}

void AuScBarrier::FacesInHullSilhouette(AuScFaceList& faces) const
{
	faces.AddFaces(m_Faces);
}

AuPoint AuScBarrier::Center() const
{
	AuPoint min,max;
	m_Faces.Extents(min,max);
	return (min + max)/2;
}

bool AuScBarrier::BlockSegment(const AuPoint& fromLoc, AuPoint& toLoc, AuPoint& floorN) const
{
	if(fromLoc.FuzzyEquals(toLoc))
		return false;
	bool hitMe = false;
	AuPoint intersect;
	AuPoint vec = toLoc - fromLoc;
	vec.Unit();
	double segLen = fromLoc.Distance(toLoc);
	for(AuScFaceList::const_iterator fi = m_Faces.cbegin(); fi != m_Faces.cend(); fi++)
	{
		bool hitRay = (*fi)->IntersectRay(fromLoc, vec, intersect);
		if(!hitRay)
			continue;
		// Did it hit within the segment.
		double intLen = fromLoc.Distance(intersect);
		// Call it a hit even if it's fuzzy so we don't fall through.
		//if(FuzzyGreaterThan(intLen, segLen))
			//continue;
		if(intLen >= segLen)
			continue;
		toLoc = intersect;
		floorN = (*fi)->FaceNormal();
		segLen = intLen;
		hitMe = true;
	}
	return hitMe;
}

bool AuScBarrier::ModelExtents(AuPoint& min, AuPoint& max) const
{
	if(m_Faces.empty())
		return false;
	m_Faces.Extents(min, max);
	return true;
}

quint64 AuScBarrier::MemoryUsage() const
{
	quint64 mem = AuScPart::MemoryUsage();
	mem += m_Faces.MemoryUsage();
	mem += sizeof(m_PartId);
	mem += sizeof(m_PartState);
	mem += m_Silhouette.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScRocketParkingSpot::AuScRocketParkingSpot()
{
}

AuScRocketParkingSpot::AuScRocketParkingSpot(const AuScRocketParkingSpot& c) : AuScIcon(c)
{
}

AuScRocketParkingSpot::AuScRocketParkingSpot(const AuScRocketParkingSpot& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
}

void AuScRocketParkingSpot::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScRocketParkingSpot::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScRocketParkingSpot::operator==(const AuScPart& p) const
{
	try
	{
		const AuScRocketParkingSpot& c = dynamic_cast<const AuScRocketParkingSpot&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScRocketParkingSpot::Clone() const
{
	return new AuScRocketParkingSpot(*this);
}

AuScPart* AuScRocketParkingSpot::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScRocketParkingSpot(*this, selInfo);
}

QString AuScRocketParkingSpot::TypeIcon() const
{
	return QString(":data/ScParkRocket.png");
}

QString AuScRocketParkingSpot::TypeName() const
{
	return QObject::tr("Space Rocket Parking Spot");
}

// Rocket footprint is ? x ?.
double AuScRocketParkingSpot::IconBack() const
{
	return FeetPerMeter()*5;
}

double AuScRocketParkingSpot::IconFore() const
{
	return FeetPerMeter()*5;
}

double AuScRocketParkingSpot::IconLeft() const
{
	return FeetPerMeter()*5;
}

double AuScRocketParkingSpot::IconRight() const
{
	return FeetPerMeter()*5;
}

AuMatrix4x4 AuScRocketParkingSpot::PositionMatrix() const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(o, x, y);
}

void AuScRocketParkingSpot::BuildingCursorSegments(AuPointArray& edges) const
{
	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(1));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(2));
	edges.append(m_Face.Vertex(3));

	edges.append(m_Face.Vertex(3));
	edges.append(m_Face.Vertex(0));

	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(3));
}

AuSysId AuScRocketParkingSpot::SightedSystem(quint16 partId) const
{
	// Part id is not included in the system id because it's just a parking spot. The actual landing system has no representation
	// that can be spotted and repaired in the final scene. Instead, repairs to the landing system with a part id of 0 result in
	// repairs to any/all damaged landing systems.
	Q_UNUSED(partId);
	return SYS_VehicleLanding;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScSite::AuScSite()
{
	m_Grade = Level;
	m_Grass = 0;
	m_Pavement = WORLDTEX_ConstructionZone;
	m_Rocks = 0;
	m_Shrubs = 0;
	m_TerBlend = 0;
	m_Trees = 0;
}

AuScSite::AuScSite(const AuScSite& c) : AuScMeshBase(c)
{
	m_Grade = c.m_Grade;
	m_Grass = c.m_Grass;
	m_Pavement = c.m_Pavement;
	m_Rocks = c.m_Rocks;
	m_Shrubs = c.m_Shrubs;
	m_TerBlend = c.m_TerBlend;
	m_Trees = c.m_Trees;
}

AuScSite::AuScSite(const AuScSite& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
	m_Grade = c.m_Grade;
	m_Grass = c.m_Grass;
	m_Pavement = c.m_Pavement;
	m_Rocks = c.m_Rocks;
	m_Shrubs = c.m_Shrubs;
	m_TerBlend = c.m_TerBlend;
	m_Trees = c.m_Trees;
}

void AuScSite::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
	quint8 hasWhat = 0;
	if(m_Grade)
		hasWhat |= 0x01;
	if(m_Grass)
		hasWhat |= 0x02;
	if(m_Pavement)
		hasWhat |= 0x04;
	if(m_Shrubs)
		hasWhat |= 0x08;
	if(m_TerBlend)
		hasWhat |= 0x10;
	if(m_Trees)
		hasWhat |= 0x20;
	if(m_Rocks)
		hasWhat |= 0x40;
	out << hasWhat;

	if(hasWhat & 0x01)
		out << m_Grade;
	if(hasWhat & 0x02)
		out << m_Grass;
	if(hasWhat & 0x04)
		out << m_Pavement;
	if(hasWhat & 0x08)
		out << m_Shrubs;
	if(hasWhat & 0x10)
		out << m_TerBlend;
	if(hasWhat & 0x20)
		out << m_Trees;
	if(hasWhat & 0x40)
		out << m_Rocks;
}

void AuScSite::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;

	if(hasWhat & 0x01)
		in >> m_Grade;
	else
		m_Grade = 0;
	if(hasWhat & 0x02)
		in >> m_Grass;
	else
		m_Grass = 0;
	if(hasWhat & 0x04)
		in >> m_Pavement;
	else
		m_Pavement = 0;
	if(hasWhat & 0x08)
		in >> m_Shrubs;
	else
		m_Shrubs = 0;
	if(hasWhat & 0x10)
		in >> m_TerBlend;
	else
		m_TerBlend = 0;
	if(hasWhat & 0x20)
		in >> m_Trees;
	else
		m_Trees = 0;
	if(hasWhat & 0x40)
		in >> m_Rocks;
	else
		m_Rocks = 0;
}

bool AuScSite::operator==(const AuScPart& p) const
{
	try
	{
		const AuScSite& c = dynamic_cast<const AuScSite&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		if(m_Grade != c.m_Grade)
			return false;
		if(m_Grass != c.m_Grass)
			return false;
		if(m_Pavement != c.m_Pavement)
			return false;
		if(m_Rocks != c.m_Rocks)
			return false;
		if(m_Shrubs != c.m_Shrubs)
			return false;
		if(m_TerBlend != c.m_TerBlend)
			return false;
		if(m_Trees != c.m_Trees)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

bool AuScSite::IsValid() const
{
	if(m_Grade > Cut)
		return false;
	if(m_Grass > 3)
		return false;
	if(m_Pavement > WORLDTEX_RockTan)
		return false;	// Shaders only support 7 pavement textures.
	if(m_Rocks > 1)
		return false;
	if(m_Shrubs > 1)
		return false;
	if(m_TerBlend > 1)
		return false;
	if(m_Trees > 1)
		return false;
	return AuScMeshBase::IsValid();
}

AuScPart* AuScSite::Clone() const
{
	return new AuScSite(*this);
}

AuScPart* AuScSite::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScSite(*this, selInfo);
}

QString AuScSite::TypeIcon() const
{
	return QString(":data/ScSite.png");
}

QString AuScSite::TypeName() const
{
	return QObject::tr("Site");
}

QString AuScSite::SelectedDesc(const AuScParts& parts) const
{
	QString desc = AuScMeshBase::SelectedDesc(parts);
	if(Grade() == Level)
		desc += QObject::tr(", Grade=Level");
	else if(Grade() == Cut)
		desc += QObject::tr(", Grade=Cut");
	else if(Grade() == Fill)
		desc += QObject::tr(", Grade=Fill");

	if(TerrainBlend() == 0)
		desc += QObject::tr(", Blend=Flat");
	else
		desc += QObject::tr(", Blend=Roll");

	if(m_Pavement == WORLDTEX_ConstructionZone)
		desc += QObject::tr(", Pavement=None");
	else if(m_Pavement == WORLDTEX_ConcreteHeavy)
		desc += QObject::tr(", Pavement=Concrete");
	else if(m_Pavement == WORLDTEX_Tarmac)
		desc += QObject::tr(", Pavement=Asphalt");
	else if(m_Pavement == WORLDTEX_DirtSmooth)
		desc += QObject::tr(", Pavement=Dirt");
	else if(m_Pavement == WORLDTEX_CobblestoneLight)
		desc += QObject::tr(", Pavement=Cobblestone");
	// End of Pavements that automatically raise ambient light level.
	else if(m_Pavement == WORLDTEX_DirtCrackedSmooth)
		desc += QObject::tr(", Pavement=Field");
	else if(m_Pavement == WORLDTEX_RockTan)
		desc += QObject::tr(", Pavement=Rock");
	// End of the Pavement textures
	/*
	else if(m_Pavement == WORLDTEX_ConcreteDarkModerate)
		desc += QObject::tr(", Pavement=Concrete Dark");
	else if(m_Pavement == WORLDTEX_DirtCoarse)
		desc += QObject::tr(", Pavement=Dirt Coarse");
	else if(m_Pavement == WORLDTEX_DirtGray)
		desc += QObject::tr(", Pavement=Dirt Gray");
	else if(m_Pavement == WORLDTEX_DirtHeavy)
		desc += QObject::tr(", Pavement=Dirt Heavy");
	else if(m_Pavement == WORLDTEX_DirtModerate)
		desc += QObject::tr(", Pavement=Dirt Moderate");
	else if(m_Pavement == WORLDTEX_Pebbles)
		desc += QObject::tr(", Pavement=Pebbles");
	else if(m_Pavement == WORLDTEX_RockBrown)
		desc += QObject::tr(", Pavement=Rock Brown");
	else if(m_Pavement == WORLDTEX_RockBrownModerate)
		desc += QObject::tr(", Pavement=Rock Brown Moderate");
	else if(m_Pavement == WORLDTEX_RockGrayCoarse)
		desc += QObject::tr(", Pavement=Rock Gray Coarse");
	else if(m_Pavement == WORLDTEX_RockGrayHeavy)
		desc += QObject::tr(", Pavement=Rock Gray Heavy");
	else if(m_Pavement == WORLDTEX_RockGrayModerate)
		desc += QObject::tr(", Pavement=Rock Gray Moderate");
	else if(m_Pavement == WORLDTEX_RockGraySmooth)
		desc += QObject::tr(", Pavement=Rock Gray Smooth");
	else if(m_Pavement == WORLDTEX_RockRedHeavy)
		desc += QObject::tr(", Pavement=Rock Red Heavy");
	else if(m_Pavement == WORLDTEX_RockRedModerate)
		desc += QObject::tr(", Pavement=Rock Red Moderate");
	else if(m_Pavement == WORLDTEX_RockRedSmooth)
		desc += QObject::tr(", Pavement=Rock Red Smooth");
	else if(m_Pavement == WORLDTEX_SandSmooth)
		desc += QObject::tr(", Pavement=Sand");
		*/
	else
		desc += QObject::tr(", Pavement=%1").arg(m_Pavement);

	if(Rocks() == 0)
		desc += QObject::tr(", Rocks=Remove");
	else
		desc += QObject::tr(", Rocks=Leave");

	if(Grass() == 0)
		desc += QObject::tr(", Grass=Remove");
	else if(Grass() == 1)
		desc += QObject::tr(", Grass=Leave");
	else if(Grass() == 2)
		desc += QObject::tr(", Grass=Mow");
	else if(Grass() == 3)
		desc += QObject::tr(", Grass=Lawn");

	if(Shrubs() == 0)
		desc += QObject::tr(", Shrubs=Remove");
	else
		desc += QObject::tr(", Shrubs=Leave");

	if(Trees() == 0)
		desc += QObject::tr(", Trees=Remove");
	else
		desc += QObject::tr(", Trees=Leave");
	return desc;
}

bool AuScSite::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	Q_UNUSED(shipHull);
	if(IsOmitted())
		return false;
	return true;
}

bool AuScSite::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

void AuScSite::MakeSite(const AuPointArray& bnd, const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();

	// Transform the boundary to the real XY plane so the math is easier.
	AuPointArray cbnd(bnd);
	cbnd.Transform(gridPlane.Inverse());

	// Prepare boundary that will be used for the top and bottom.
	if(!cbnd.IsClockwise())
		cbnd.ReverseOrder();

	// Triangulate the boundary with edge lines at the boundary.
	AuPointArray tris;
	QVector<bool> edges;
	cbnd.TriangulateBoundary(tris, edges);

	m_Faces.AddFace(AuScFacePolygon(tris, edges));

	// Transform the result to the design plane.
	m_Faces.Transform(gridPlane);
}

double AuScSite::FieldArea() const
{
	// Pavement type determines if this can be a field.
	if(m_Pavement != WORLDTEX_DirtCrackedSmooth)
		return 0;
	// Pavement is ok for a field.
	return m_Faces.Area();
}

bool AuScSite::SiteExtents(AuPoint& min, AuPoint& max) const
{
	if(m_Faces.empty())
		return false;
	m_Faces.Extents(min, max);
	return true;
}

float AuScSite::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const
{
	switch(Grade())
	{
	case Terrain:						// Terrain elevation is unmodified.
		break;
	case Level:							// Terrain that lands in the site is raised or lowered as necessary to site elevation.
		{
			float sin = m_Faces.SiteInfluence(loc, siteLoc);
			if(sin > 0)
			{
				terBlend = m_TerBlend;
				return sin;
			}
		}
		break;
	case Fill:							// Terrain that lands in the site is raised to site elevation, if it is below the site.
		{
			float sin = m_Faces.SiteInfluence(loc, siteLoc);
			if(sin > 0 && loc.z <= siteLoc.z)
			{
				terBlend = m_TerBlend;
				return sin;				// Terrain is lower than site or the same; influence it.
			}
		}
		break;
	case Cut:							// Terrain that lands in the site is lowered to site elevation, if it is above the site.
		{
			float sin = m_Faces.SiteInfluence(loc, siteLoc);
			if(sin > 0 && loc.z >= siteLoc.z)
			{
				terBlend = m_TerBlend;
				return sin;				// Terrain is higher than site or the same; influence it.
			}
		}
		break;
	}
	return 0;
}

bool AuScSite::InBuildingSite(const AuPoint& loc) const
{
	return m_Faces.InSite(loc);
}

bool AuScSite::InBuildingSite(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const
{
	if(!m_Faces.InSite(loc))
		return false;
	switch(Pavement())
	{
	case WORLDTEX_ConstructionZone:
	case WORLDTEX_DirtSmooth:
	case WORLDTEX_DirtCrackedSmooth:
		siteGround = OG_Sand;
		break;
	case WORLDTEX_ConcreteHeavy:
	case WORLDTEX_Tarmac:
		siteGround = OG_Pavement;
		break;
	case WORLDTEX_CobblestoneLight:
	case WORLDTEX_RockTan:
		siteGround = OG_Rock;
		break;
	default:
		siteGround = OG_None;
		break;
	}
	if(m_Grass == 0)
		hideGrass = 1;	// hide the grass
	else if(m_Grass == 1)
		hideGrass = 0;	// do not hide the grass
	else
		hideGrass = m_Grass;
	return true;
}

float AuScSite::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
		const AuZoneResource& res) const
{
	Q_UNUSED(farmShrub);
	Q_UNUSED(orchardTree);
	Q_UNUSED(res);
	// I can save a bunch of work when I know I don't affect things in my site area other than the terrain elevation.
	if(!m_Grass && !m_Rocks && !m_Shrubs && !m_Trees && m_Pavement == WORLDTEX_ConstructionZone)
		return BuildingSiteInfluence(loc, siteLoc, terBlend);

	float sin = m_Faces.SiteInfluence(loc, siteLoc);
	if(sin == 0)
		return 0;

	// At this point we know that 'loc' is in this site.
	terBlend = m_TerBlend;
	if(m_Grass == 0)
		hideGrass = 1;	// hide the grass
	else if(m_Grass == 1)
		hideGrass = 0;	// do not hide the grass
	else
		hideGrass = m_Grass;
	hideRocks = !m_Rocks;
	hideShrubs = !m_Shrubs;
	hideTrees = !m_Trees;
	if(m_Pavement != WORLDTEX_ConstructionZone)
		pavementType = Pavement();

	switch(Grade())
	{
	case Terrain:						// Terrain elevation is unmodified.
		break;
	case Level:							// Terrain that lands in the site is raised or lowered as necessary to site elevation.
		return sin;
	case Fill:							// Terrain that lands in the site is raised to site elevation, if it is below the site.
		if(loc.z <= siteLoc.z)
			return sin;					// Terrain is lower than site or the same; influence it.
		break;
	case Cut:							// Terrain that lands in the site is lowered to site elevation, if it is above the site.
		if(loc.z >= siteLoc.z)
			return sin;					// Terrain is higher than site or the same; influence it.
		break;
	}
	return 0;
}

void AuScSite::GlowVertexLocs(const AuPointArray& vertLocs, const QColor& glow)
{
	// Select the vertices that are supposed to glow.
	AuScSelectInfo sel;
	for(AuPointArray::const_iterator vi = vertLocs.cbegin(); vi != vertLocs.cend(); vi++)
		m_Faces.SelectVerticesAt(*vi, sel);
	m_Faces.FaceIds(sel.m_Faces);
	m_Faces.GlowVertices(sel, glow);
}

void AuScSite::BuildingCursorSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

void AuScSite::BuildingCursorSiteSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

void AuScSite::BuildZoneCheckLocs(AuPointArray& locs) const
{
	m_Faces.VertexLocations(locs);
	for(AuPointArray::iterator li = locs.begin(); li != locs.end(); li++)
		(*li).z = 0;			// Flatten them all to the plane of the insertion point.
}

bool AuScSite::RandomSiteLocation(const AuScParts& parts, AuPoint& siteLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates) const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	int wid = max.x - min.x;
	int len = max.y - min.y;
	if(wid < 2 || len < 2)
		return false;

	// Allow a margin.
	wid -= 1;
	len -= 1;

	int tries=20;
	do
	{
		double x = min.x + g_Au.Dice().Roll(1, wid);
		double y = min.y + g_Au.Dice().Roll(1, len);
		double z = min.z;
		AuPoint loc(x,y,z);
		if(m_Faces.InSite(loc) && !parts.IsInsideObstructionModel(loc, doorStates, landingGearStates))
		{
			// Found an unobstructed location in the site.
			siteLoc = loc;
			return true;
		}
	}
	while(tries--);
	return false;
}

quint64 AuScSite::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	mem += sizeof(m_Grade);
	mem += sizeof(m_Grass);
	mem += sizeof(m_Pavement);
	mem += sizeof(m_Rocks);
	mem += sizeof(m_Shrubs);
	mem += sizeof(m_TerBlend);
	mem += sizeof(m_Trees);
	return mem;
}

void AuScSite::SetSiteProperties(quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs, quint8 terBlend,
		quint8 trees)
{
	if(grade != 255)
		m_Grade = grade;

	if(grass != 255)
		m_Grass = grass;

	if(pavement != 255)
		m_Pavement = pavement;

	if(rocks != 255)
		m_Rocks = rocks;

	if(shrubs != 255)
		m_Shrubs = shrubs;

	if(terBlend != 255)
		m_TerBlend = terBlend;

	if(trees != 255)
		m_Trees = trees;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScTree::AuScTree()
{
	m_PlantCID = CID_Invalid;
}

AuScTree::AuScTree(const AuScTree& c) : AuScIcon(c)
{
	m_PlantCID = c.m_PlantCID;
}

AuScTree::AuScTree(const AuScTree& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
	m_PlantCID = c.m_PlantCID;
}

void AuScTree::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
	out << m_PlantCID;
}

void AuScTree::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
	in >> m_PlantCID;
}

bool AuScTree::operator==(const AuScPart& p) const
{
	try
	{
		const AuScTree& c = dynamic_cast<const AuScTree&>(p);
		if(!AuScIcon::operator==(c))
			return false;
		if(m_PlantCID != c.m_PlantCID)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScTree::Clone() const
{
	return new AuScTree(*this);
}

AuScPart* AuScTree::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScTree(*this, selInfo);
}

QString AuScTree::TypeIcon() const
{
	return QString(":data/ScTree.png");
}

QString AuScTree::TypeName() const
{
	return QObject::tr("Tree");
}

QString AuScTree::SelectedDesc(const AuScParts& parts) const
{
	if(PlantCID() == CID_Invalid)
		return QObject::tr("%1, Type=Random")
			.arg(FullName(parts));
	else if(PlantCID() == CID_FarmMachinery)
		return QObject::tr("%1, Type=Agriculture")
			.arg(FullName(parts));
	else
		return QObject::tr("%1, Type=%2")
			.arg(FullName(parts))
			.arg(g_Au.CommodityName(PlantCID()));
}

double AuScTree::IconBack() const
{
	return FeetPerMeter() * 2;
}

double AuScTree::IconFore() const
{
	return FeetPerMeter() * 2;
}

double AuScTree::IconLeft() const
{
	return FeetPerMeter() * 2;
}

double AuScTree::IconRight() const
{
	return FeetPerMeter() * 2;
}

GLuint AuScTree::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/ScTree.png");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

float AuScTree::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
		const AuZoneResource& res) const
{
	Q_UNUSED(siteLoc);
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(farmShrub);
	Q_UNUSED(terBlend);
	AuPoint o(m_Face.FaceLocation());
	if(loc.Distance_2D(o) < 24/2 + 2)			// DaWorldGlobe::CreateTrees uses 24' spacing.
	{
		COMMODITYID cid = PlantCID();
		if(cid == CID_Invalid)
		{
			float noise = (float)loc.x - (int)loc.x;
			cid = res.GenerateTreeResource(noise);
		}
		else if(cid == CID_FarmMachinery)
			hideGrass = 1;	// Make rows of crops stand out by clearing around the plants..
		orchardTree = cid;
	}
	return 0;
}

quint64 AuScTree::MemoryUsage() const
{
	quint64 mem = AuScIcon::MemoryUsage();
	mem += sizeof(m_PlantCID);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScShrub::AuScShrub()
{
	m_PlantCID = CID_Invalid;
}

AuScShrub::AuScShrub(const AuScShrub& c) : AuScIcon(c)
{
	m_PlantCID = c.m_PlantCID;
}

AuScShrub::AuScShrub(const AuScShrub& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
	m_PlantCID = c.m_PlantCID;
}

void AuScShrub::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
	out << m_PlantCID;
}

void AuScShrub::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
	in >> m_PlantCID;
}

bool AuScShrub::operator==(const AuScPart& p) const
{
	try
	{
		const AuScShrub& c = dynamic_cast<const AuScShrub&>(p);
		if(!AuScIcon::operator==(c))
			return false;
		if(m_PlantCID != c.m_PlantCID)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScShrub::Clone() const
{
	return new AuScShrub(*this);
}

AuScPart* AuScShrub::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScShrub(*this, selInfo);
}

QString AuScShrub::TypeIcon() const
{
	return QString(":data/ScShrub.png");
}

QString AuScShrub::TypeName() const
{
	return QObject::tr("Shrub");
}

QString AuScShrub::SelectedDesc(const AuScParts& parts) const
{
	if(PlantCID() == CID_Invalid)
		return QObject::tr("%1, Type=Random")
			.arg(FullName(parts));
	else if(PlantCID() == CID_FarmMachinery)
		return QObject::tr("%1, Type=Agriculture")
			.arg(FullName(parts));
	else
		return QObject::tr("%1, Type=%2")
			.arg(FullName(parts))
			.arg(g_Au.CommodityName(PlantCID()));
}

double AuScShrub::IconBack() const
{
	return FeetPerMeter();
}

double AuScShrub::IconFore() const
{
	return FeetPerMeter();
}

double AuScShrub::IconLeft() const
{
	return FeetPerMeter();
}

double AuScShrub::IconRight() const
{
	return FeetPerMeter();
}

GLuint AuScShrub::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/ScShrub.png");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

float AuScShrub::BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
		const AuZoneResource& res) const
{
	Q_UNUSED(siteLoc);
	Q_UNUSED(hideRocks);
	Q_UNUSED(hideShrubs);
	Q_UNUSED(hideTrees);
	Q_UNUSED(pavementType);
	Q_UNUSED(orchardTree);
	Q_UNUSED(terBlend);
	AuPoint o(m_Face.FaceLocation());
	if(loc.Distance_2D(o) < 12/2 + 2)			// DaWorldGlobe::CreateShrubs uses 12' spacing.
	{
		COMMODITYID cid = PlantCID();
		if(cid == CID_Invalid)
		{
			float noise = (float)loc.x - (int)loc.x;
			cid = res.GenerateShrubResource(noise);
		}
		else if(cid == CID_FarmMachinery)
			hideGrass = 1;	// Make rows of crops stand out by clearing around the plants..
		farmShrub = cid;
	}
	return 0;
}

quint64 AuScShrub::MemoryUsage() const
{
	quint64 mem = AuScIcon::MemoryUsage();
	mem += sizeof(m_PlantCID);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScFence::AuScFence()
{
	m_DesignId = 0;
	m_Style = WireT;
}

AuScFence::AuScFence(const AuScFence& c) : AuScPart(c)
{
	m_DesignId = c.m_DesignId;
	m_Segment = c.m_Segment;
	m_Style = c.m_Style;
}

AuScFence::AuScFence(const AuScFence& c, const AuScSelectInfo& selInfo) : AuScPart(c)
{
	Q_UNUSED(selInfo);
	m_DesignId = c.m_DesignId;
	m_Segment = c.m_Segment;
	m_Style = c.m_Style;
}

void AuScFence::operator>>(QDataStream& out) const
{
	AuScPart::operator>>(out);
	out << m_Segment;
	quint8 hasWhat = m_Style;

	// m_DesignId added in version 0x11
	if(m_DesignId)
		hasWhat |= 0x80;

	out << hasWhat;
	if(hasWhat & 0x80)
		out << m_DesignId;
}

void AuScFence::operator<<(QDataStream& in)
{
	AuScPart::operator<<(in);
	in >> m_Segment;
	quint8 hasWhat;
	in >> hasWhat;
	m_Style = hasWhat & 0x7f;
	if(hasWhat & 0x80)
		in >> m_DesignId;
}

bool AuScFence::operator==(const AuScPart& p) const
{
	try
	{
		const AuScFence& c = dynamic_cast<const AuScFence&>(p);
		if(!AuScPart::operator==(c))
			return false;
		if(m_DesignId != c.m_DesignId)
			return false;
		if(m_Style != c.m_Style)
			return false;
		if(m_Segment != c.m_Segment)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScFence::Clone() const
{
	return new AuScFence(*this);
}

AuScPart* AuScFence::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScFence(*this, selInfo);
}

QString AuScFence::TypeIcon() const
{
	return StyleIcon(Style());
}

QString AuScFence::TypeName() const
{
	return StyleName(Style());
}

QString AuScFence::SelectedDesc(const AuScParts& parts) const
{
	return QObject::tr("%1")
		.arg(FullName(parts));
}

//static
QString AuScFence::StyleName(STYLE fenceStyle)
{
	if(fenceStyle < 0 || fenceStyle >= STYLE_Count)
		return "";
	QString styleNames[STYLE_Count] =
	{
		QObject::tr("Wood Posts"),
		QObject::tr("Tee Posts"),
		QObject::tr("Wire Fence on Wood Posts"),
		QObject::tr("Wire Fence on Tee Posts"),
		QObject::tr("Range Fence on Wood Posts"),
		QObject::tr("Range Fence on Tee Posts"),
		QObject::tr("Chain Link Fence"),
		QObject::tr("Chain Link Fence with Razor Wire"),
		QObject::tr("Split Rail Fence"),
		QObject::tr("Stockade Fence"),
		QObject::tr("Sand Bag Wall"),
		QObject::tr("Brick Wall"),
		QObject::tr("Stone Wall"),
		QObject::tr("Custom Wall or Fence"),
	};
	return styleNames[fenceStyle];
}

//static
QString AuScFence::StyleIcon(STYLE fenceStyle)
{
	if(fenceStyle < 0 || fenceStyle >= STYLE_Count)
		return "";
	QString styleNames[STYLE_Count] =
	{
		QObject::tr(":data/ScFencePosts.png"),
		QObject::tr(":data/ScFencePostsT.png"),
		QObject::tr(":data/ScFenceWire.png"),
		QObject::tr(":data/ScFenceWireT.png"),
		QObject::tr(":data/ScFenceRange.png"),
		QObject::tr(":data/ScFenceRangeT.png"),
		QObject::tr(":data/ScFenceChain.png"),
		QObject::tr(":data/ScFenceChainRazor.png"),
		QObject::tr(":data/ScFenceRail.png"),
		QObject::tr(":data/ScFenceWood.png"),
		QObject::tr(":data/ScFenceSandBag.png"),
		QObject::tr(":data/ScFenceBrick.png"),
		QObject::tr(":data/ScFenceStone.png"),
		QObject::tr(":data/bFenceWall.png"),
	};
	return styleNames[fenceStyle];
}

void AuScFence::EdgeIds(QSet<quint32>& edgeIds) const
{
	edgeIds.insert(0);
}

void AuScFence::FaceIds(QSet<quint32>& faceIds) const
{
	Q_UNUSED(faceIds);
}

void AuScFence::VertexIds(QSet<quint32>& vertexIds) const
{
	vertexIds.insert(0);
	vertexIds.insert(1);
}

AuScPoint AuScFence::VertexLoc(quint32 vertexIndex) const
{
	return AuScPoint(m_Segment.Vertex(vertexIndex), 0,0, AuPoint(0,0,1));
}

bool AuScFence::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	Q_UNUSED(findFaces);
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyInRect = false;

	if(findVertices)
	{
		// Test all projected vertices to see if they are inside the pixel rectangle.
		for(quint32 vindex=0; vindex<2; vindex++)
		{
			AuPoint vloc = mvp.Project(m_Segment.Vertex(vindex));
			if(vloc.z > 0 && pixelRect.contains(QPointF(vloc.x, vloc.y)))
				anyInRect = true;
		}
	}

	if(findEdges)
	{
		AuSegment seg(m_Segment);
		seg.Transform(mvp);
		if((seg.m_fLoc1.z > 0 || seg.m_fLoc2.z > 0) &&
				pixelRect.contains(QPointF(seg.m_fLoc1.x, seg.m_fLoc1.y)) &&
				pixelRect.contains(QPointF(seg.m_fLoc2.x, seg.m_fLoc2.y)))
			anyInRect = true;
	}

	return anyInRect;
}

bool AuScFence::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	Q_UNUSED(findFaces);
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyInRect = false;

	if(findVertices)
	{
		// Test all projected vertices to see if they are inside the pixel rectangle.
		for(quint32 vindex=0; vindex<2; vindex++)
		{
			AuPoint vloc = mvp.Project(m_Segment.Vertex(vindex));
			if(vloc.z > 0 && pixelRect.contains(QPointF(vloc.x, vloc.y)))
			{
				selInfo.m_Vertices.insert(vindex);
				anyInRect = true;
			}
		}
	}

	if(findEdges)
	{
		AuSegment seg(m_Segment);
		seg.Transform(mvp);
		if((seg.m_fLoc1.z > 0 || seg.m_fLoc2.z > 0) &&
				pixelRect.contains(QPointF(seg.m_fLoc1.x, seg.m_fLoc1.y)) &&
				pixelRect.contains(QPointF(seg.m_fLoc2.x, seg.m_fLoc2.y)))
		{
			selInfo.m_Edges.insert(0);
			anyInRect = true;
		}
	}

	return anyInRect;
}

bool AuScFence::IsValid() const
{
	return !m_Segment.m_fLoc1.FuzzyEquals(m_Segment.m_fLoc2);
}

void AuScFence::EdgeSegments(AuPointArray& edges) const
{
	edges.append(m_Segment.m_fLoc1);
	edges.append(m_Segment.m_fLoc2);
}

void AuScFence::FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	Q_UNUSED(selFaces);
	Q_UNUSED(selLocs);
	Q_UNUSED(unselLocs);
}

void AuScFence::VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const
{
	int unselSize = g_Au.DesignerVertexSize();
	int selSize = unselSize * 3 / 2;

	for(quint32 vindex=0; vindex<2; vindex++)
		if(selVerts.contains(vindex))
			selLocs.append(AuPointAlpha(m_Segment.Vertex(vindex), selSize));
		else
			unselLocs.append(AuPointAlpha(m_Segment.Vertex(vindex), selSize));
}

void AuScFence::Transform(const AuMatrix4x4& t)
{
	m_Segment.Transform(t);
}

void AuScFence::Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	if(selInfo.m_Vertices.contains(0) && selInfo.m_Vertices.contains(1))
		Transform(t);
	else if(selInfo.m_Vertices.contains(0))
		m_Segment.m_fLoc1 = t.Project(m_Segment.m_fLoc1);
	else if(selInfo.m_Vertices.contains(1))
		m_Segment.m_fLoc2 = t.Project(m_Segment.m_fLoc2);
}

void AuScFence::TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
{
	if(selInfo.m_Vertices.contains(0) && selInfo.m_Vertices.contains(1))
		Transform(t);
	else if(selInfo.m_Vertices.contains(0))
		m_Segment.m_fLoc1 = t.Project(m_Segment.m_fLoc1);
	else if(selInfo.m_Vertices.contains(1))
		m_Segment.m_fLoc2 = t.Project(m_Segment.m_fLoc2);
}

double AuScFence::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	if(snapLoc)
		return m_Segment.ViewDistance(mvp, pixel, *snapLoc, -1);

	AuPoint edgeLoc;
	return m_Segment.ViewDistance(mvp, pixel, edgeLoc, -1);
}

double AuScFence::EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint edgeLoc;
	double dseg = m_Segment.ViewDistance(mvp, pixel, edgeLoc, -1);
	if(dseg >= d_SearchTolerance)
		return d_SearchTolerance;

	edgeSeg = m_Segment;
	return dseg;
}

double AuScFence::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	Q_UNUSED(mvp);
	Q_UNUSED(pixel);
	Q_UNUSED(faceIndex);
	Q_UNUSED(gripLoc);
	return d_SearchTolerance;
}

double AuScFence::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return d_SearchTolerance;

	AuPoint fpx(pixel);
	double closestDist2 = d_SearchTolerance*d_SearchTolerance;	// We won't find anything that is not closer than this dist.
	bool foundMe = false;

	for(quint32 vindex=0; vindex<2; vindex++)
	{
		AuPoint vloc = mvp.Project(m_Segment.Vertex(vindex));
		if(vloc.z < 0)
			continue;		// Behind eye plane.

		double dist2 = vloc.Distance2_2D(fpx);
		if(dist2 < closestDist2)
		{
			closestDist2 = dist2;
			vertexIndex = vindex;
			if(snapLoc)
				*snapLoc = m_Segment.Vertex(vindex);
			foundMe = true;
		}
	}

	if(!foundMe)
		return d_SearchTolerance;

	return sqrt(closestDist2);
}

bool AuScFence::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(angleTol);
	Q_UNUSED(selInfo);
	return false;	// No faces.
}

bool AuScFence::SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	if(selInfo.m_Edges.contains(0))
		return false;
	
	if(!m_Segment.IsSegment(edgeSeg))
		return false;

	selInfo.m_Edges.insert(0);
	return true;
}

bool AuScFence::SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(atLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScFence::SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(atLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScFence::SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	for(quint32 vindex=0; vindex<2; vindex++)
	{
		if(m_Segment.Vertex(vindex).FuzzyEquals(atLoc) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.insert(vindex);
			anyChange = true;
		}
	}

	return anyChange;
}

bool AuScFence::DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	if(selInfo.m_Edges.contains(0))
		return false;
	
	if(!m_Segment.IsSegment(edgeSeg))
		return false;

	selInfo.m_Edges.remove(0);
	return true;
}

bool AuScFence::DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(atLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScFence::DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
{
	// Don't find parts that are hidden.
	if(IsHidden())
		return false;

	bool anyChange = false;

	for(quint32 vindex=0; vindex<2; vindex++)
	{
		if(m_Segment.Vertex(vindex).FuzzyEquals(atLoc) && !selInfo.m_Vertices.contains(vindex))
		{
			selInfo.m_Vertices.insert(vindex);
			anyChange = true;
		}
	}

	return anyChange;
}

void AuScFence::MakeFence(STYLE style, quint32 customDesignId, const AuPoint& start, const AuPoint& end)
{
	m_DesignId = customDesignId;
	m_Segment.m_fLoc1 = start;
	m_Segment.m_fLoc2 = end;
	m_Style = style;
}

void AuScFence::SetFaceSelfIllumination(float selfIllum)
{
	Q_UNUSED(selfIllum);
}

void AuScFence::SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(selfIllum);
}

void AuScFence::SetFaceOpacity(float opacity)
{
	Q_UNUSED(opacity);
}

void AuScFence::SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(opacity);
}

void AuScFence::EffectFaces(SURFACE_EFFECT effect)
{
	Q_UNUSED(effect);
}

void AuScFence::EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
}

void AuScFence::EffectFacesCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScFence::EffectFacesProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScFence::EffectFacesSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(gridPlane);
}

void AuScFence::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(effect);
	Q_UNUSED(texSize);
	Q_UNUSED(unwrap);
	Q_UNUSED(angleTol);
	Q_UNUSED(gridAlign);
	Q_UNUSED(alignAxis);
	Q_UNUSED(gridPlane);
}

SURFACE_EFFECT AuScFence::Effect(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return EFF_Count;
}

void AuScFence::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(vec);
}

void AuScFence::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(pivot);
	Q_UNUSED(angle);
}

void AuScFence::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(base);
	Q_UNUSED(scaleX);
	Q_UNUSED(scaleY);
}

void AuScFence::TextureFaces(SC_TEXTURE_INDEX texIndex)
{
	Q_UNUSED(texIndex);
}

void AuScFence::TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
}

void AuScFence::TextureFacesCylindrical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScFence::TextureFacesProject(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texSize);
	Q_UNUSED(gridPlane);
}

void AuScFence::TextureFacesSpherical(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texRepsAtEquator);
	Q_UNUSED(gridPlane);
}

void AuScFence::TextureUnwrap(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(texIndex);
	Q_UNUSED(texSize);
	Q_UNUSED(unwrap);
	Q_UNUSED(angleTol);
	Q_UNUSED(gridAlign);
	Q_UNUSED(alignAxis);
	Q_UNUSED(gridPlane);
}

void AuScFence::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(vec);
}

void AuScFence::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(pivot);
	Q_UNUSED(angle);
}

void AuScFence::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	Q_UNUSED(selInfo);
	Q_UNUSED(base);
	Q_UNUSED(scaleX);
	Q_UNUSED(scaleY);
}

SC_TEXTURE_INDEX AuScFence::TextureIndex(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return SCTEXI_Count;
}

bool AuScFence::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScFence::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScFence::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

bool AuScFence::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	Q_UNUSED(texLoc);
	Q_UNUSED(selInfo);
	return false;
}

void AuScFence::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	double segLen = m_Segment.Length();
	int fenceV = (int)ceil(segLen) / 2;			// one cubic foot per foot of length	1/2 because all was too much
	int postV = (int)ceil(segLen / 12) / 2;		// one cubic foot per post every 12'	1/2 because all was too much
	int totV;
	switch(FenceStyle())
	{
	case Posts:
		postV /= 2;
		//if(postV < 1)
			//postV = 1;
		if(postV)
			pc.AddRequirement(CID_Wood, CE_Required, postV);
		break;
	case SplitRail:
		postV /= 2;
		fenceV /= 12;
		totV = postV + fenceV;
		//if(totV < 1)
			//totV = 1;
		if(totV)
			pc.AddRequirement(CID_Wood, CE_Required, totV);
		break;
	case Wire:
	case Range:
		postV /= 2;
		//if(postV < 1)
			//postV = 1;
		if(postV)
			pc.AddRequirement(CID_Wood, CE_Required, postV);
		fenceV /= 100;
		//if(fenceV < 1)
			//fenceV = 1;
		if(fenceV)
			pc.AddRequirement(CID_Metal, CE_Required, fenceV);
		break;
	case PostsT:
		postV /= 6;
		//if(postV < 1)
			//postV = 1;
		if(postV)
			pc.AddRequirement(CID_Metal, CE_Required, postV);
		break;
	case WireT:
	case RangeT:
		postV /= 6;
		fenceV /= 100;
		totV = postV + fenceV;
		//if(totV < 1)
			//totV = 1;
		if(totV)
			pc.AddRequirement(CID_Metal, CE_Required, totV);
		break;
	case ChainLink:
	case ChainLinkRazorWire:
		postV /= 8;
		fenceV /= 40;
		totV = postV + fenceV;
		//if(totV < 1)
			//totV = 1;
		if(totV)
			pc.AddRequirement(CID_Metal, CE_Required, totV);
		break;
	case WoodYard:
		postV /= 2;
		fenceV /= 8;
		totV = postV + fenceV;
		//if(totV < 1)
			//totV = 1;
		if(totV)
			pc.AddRequirement(CID_Lumber, CE_Required, totV);
		break;
	case SandBagWall:
		fenceV /= 20;
		//if(fenceV < 1)
			//fenceV = 1;
		if(fenceV)
			pc.AddRequirement(CID_Textiles, CE_Required, fenceV);
		break;
	case BrickWall:
	case StoneWall:
	case Custom:
		totV = postV + fenceV;
		//if(totV < 1)
			//totV = 1;
		if(totV)
			pc.AddRequirement(hullMaterial, CE_Required, totV);
		break;
	case STYLE_Count:
		break;
	}
}

bool AuScFence::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	Q_UNUSED(shipHull);
	if(IsOmitted())
		return false;
	return true;
}

bool AuScFence::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

AuSysId AuScFence::SightedSystem(quint16 partId) const
{
	Q_UNUSED(partId);
	return SYS_Count;
}

AuPoint AuScFence::Center() const
{
	return m_Segment.CenterPoint();
}

bool AuScFence::ModelExtents(AuPoint& min, AuPoint& max) const
{
	min = m_Segment.m_fLoc1;
	max = m_Segment.m_fLoc1;
	min = min.Min(m_Segment.m_fLoc2);
	max = max.Max(m_Segment.m_fLoc2);
	return true;
}

quint64 AuScFence::MemoryUsage() const
{
	quint64 mem = AuScPart::MemoryUsage();
	mem += sizeof(m_DesignId);
	mem += sizeof(m_Segment);
	mem += sizeof(m_Style);
	return mem;
}

//static
void AuScFence::FencePostFaces(STYLE fenceStyle, AuScFaceList& faces)
{
	if(fenceStyle > STYLE_Count)
	{
		faces.Clear();
		return;
	}

	s_FenceLock.lockForRead();
	if(s_FencePosts[fenceStyle].empty())
	{
		s_FenceLock.unlock();
		// Load the appropriate post model from a file, based on fence style.
		AuDesign* fDesign = 0;
		switch(fenceStyle)
		{
		case Posts:
			fDesign = AuDesign::LoadFromFile(":data/FencePosts.SoH", SOH_FILE_TYPE);
			break;
		case Wire:
			fDesign = AuDesign::LoadFromFile(":data/FenceWire.SoH", SOH_FILE_TYPE);
			break;
		case Range:
			fDesign = AuDesign::LoadFromFile(":data/FenceRange.SoH", SOH_FILE_TYPE);
			break;
		case SplitRail:
			fDesign = AuDesign::LoadFromFile(":data/FenceSplitRail.SoH", SOH_FILE_TYPE);
			break;
		case WoodYard:
			fDesign = AuDesign::LoadFromFile(":data/FenceWoodYard.SoH", SOH_FILE_TYPE);
			break;
		case PostsT:
			fDesign = AuDesign::LoadFromFile(":data/FencePostsT.SoH", SOH_FILE_TYPE);
			break;
		case WireT:
			fDesign = AuDesign::LoadFromFile(":data/FenceWireT.SoH", SOH_FILE_TYPE);
			break;
		case RangeT:
			fDesign = AuDesign::LoadFromFile(":data/FenceRangeT.SoH", SOH_FILE_TYPE);
			break;
		case ChainLink:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLink.SoH", SOH_FILE_TYPE);
			break;
		case ChainLinkRazorWire:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLinkRazor.SoH", SOH_FILE_TYPE);
			break;
		case SandBagWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceSandBagWall.SoH", SOH_FILE_TYPE);
			break;
		case BrickWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceBrickWall.SoH", SOH_FILE_TYPE);
			break;
		case StoneWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceStoneWall.SoH", SOH_FILE_TYPE);
			break;
		case Custom:
			break;
		case STYLE_Count:
			fDesign = AuDesign::LoadFromFile(":data/FenceFlags.SoH", SOH_FILE_TYPE);
			break;
		}
		if(!fDesign)
		{
			faces.Clear();
			return;
		}
		s_FenceLock.lockForWrite();
		fDesign->VisibleFaces(s_FencePosts[fenceStyle], d_GroupPost);
		delete fDesign;
	}
	faces = s_FencePosts[fenceStyle];
	s_FenceLock.unlock();
}

void AuScFence::FencePostFaces(AuScFaceList& faces) const
{
	FencePostFaces(Style(), faces);
}

//static
void AuScFence::FencePostBarrierFaces(STYLE fenceStyle, AuScFaceList& faces)
{
	if(fenceStyle >= STYLE_Count)
	{
		faces.Clear();
		return;
	}

	s_FenceLock.lockForRead();
	if(s_FencePostBarriers[fenceStyle].empty())
	{
		s_FenceLock.unlock();
		// Load the appropriate post model from a file, based on fence style.
		AuDesign* fDesign = 0;
		switch(fenceStyle)
		{
		case Posts:
			fDesign = AuDesign::LoadFromFile(":data/FencePosts.SoH", SOH_FILE_TYPE);
			break;
		case Wire:
			fDesign = AuDesign::LoadFromFile(":data/FenceWire.SoH", SOH_FILE_TYPE);
			break;
		case Range:
			fDesign = AuDesign::LoadFromFile(":data/FenceRange.SoH", SOH_FILE_TYPE);
			break;
		case SplitRail:
			fDesign = AuDesign::LoadFromFile(":data/FenceSplitRail.SoH", SOH_FILE_TYPE);
			break;
		case WoodYard:
			fDesign = AuDesign::LoadFromFile(":data/FenceWoodYard.SoH", SOH_FILE_TYPE);
			break;
		case PostsT:
			fDesign = AuDesign::LoadFromFile(":data/FencePostsT.SoH", SOH_FILE_TYPE);
			break;
		case WireT:
			fDesign = AuDesign::LoadFromFile(":data/FenceWireT.SoH", SOH_FILE_TYPE);
			break;
		case RangeT:
			fDesign = AuDesign::LoadFromFile(":data/FenceRangeT.SoH", SOH_FILE_TYPE);
			break;
		case ChainLink:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLink.SoH", SOH_FILE_TYPE);
			break;
		case ChainLinkRazorWire:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLinkRazor.SoH", SOH_FILE_TYPE);
			break;
		case SandBagWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceSandBagWall.SoH", SOH_FILE_TYPE);
			break;
		case BrickWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceBrickWall.SoH", SOH_FILE_TYPE);
			break;
		case StoneWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceStoneWall.SoH", SOH_FILE_TYPE);
			break;
		case Custom:
		case STYLE_Count:
			break;
		}
		if(!fDesign)
		{
			faces.Clear();
			return;
		}
		s_FenceLock.lockForWrite();
		fDesign->BarrierFaces(s_FencePostBarriers[fenceStyle], d_GroupPost);
		delete fDesign;
	}
	faces = s_FencePostBarriers[fenceStyle];
	s_FenceLock.unlock();
}

//static
void AuScFence::FenceSectionBarrierFaces(STYLE fenceStyle, AuScFaceList& faces)
{
	if(fenceStyle >= STYLE_Count)
	{
		faces.Clear();
		return;
	}

	s_FenceLock.lockForRead();
	if(s_FenceSectionBarriers[fenceStyle].empty())
	{
		s_FenceLock.unlock();
		// Load the appropriate post model from a file, based on fence style.
		AuDesign* fDesign = 0;
		switch(fenceStyle)
		{
		case Posts:
			fDesign = AuDesign::LoadFromFile(":data/FencePosts.SoH", SOH_FILE_TYPE);
			break;
		case Wire:
			fDesign = AuDesign::LoadFromFile(":data/FenceWire.SoH", SOH_FILE_TYPE);
			break;
		case Range:
			fDesign = AuDesign::LoadFromFile(":data/FenceRange.SoH", SOH_FILE_TYPE);
			break;
		case SplitRail:
			fDesign = AuDesign::LoadFromFile(":data/FenceSplitRail.SoH", SOH_FILE_TYPE);
			break;
		case WoodYard:
			fDesign = AuDesign::LoadFromFile(":data/FenceWoodYard.SoH", SOH_FILE_TYPE);
			break;
		case PostsT:
			fDesign = AuDesign::LoadFromFile(":data/FencePostsT.SoH", SOH_FILE_TYPE);
			break;
		case WireT:
			fDesign = AuDesign::LoadFromFile(":data/FenceWireT.SoH", SOH_FILE_TYPE);
			break;
		case RangeT:
			fDesign = AuDesign::LoadFromFile(":data/FenceRangeT.SoH", SOH_FILE_TYPE);
			break;
		case ChainLink:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLink.SoH", SOH_FILE_TYPE);
			break;
		case ChainLinkRazorWire:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLinkRazor.SoH", SOH_FILE_TYPE);
			break;
		case SandBagWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceSandBagWall.SoH", SOH_FILE_TYPE);
			break;
		case BrickWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceBrickWall.SoH", SOH_FILE_TYPE);
			break;
		case StoneWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceStoneWall.SoH", SOH_FILE_TYPE);
			break;
		case Custom:
		case STYLE_Count:
			break;
		}
		if(!fDesign)
		{
			faces.Clear();
			return;
		}
		s_FenceLock.lockForWrite();
		fDesign->BarrierFaces(s_FenceSectionBarriers[fenceStyle], d_GroupSection);
		delete fDesign;
	}
	faces = s_FenceSectionBarriers[fenceStyle];
	s_FenceLock.unlock();
}

//static
void AuScFence::FenceSectionFaces(STYLE fenceStyle, AuScFaceList& faces)
{
	if(fenceStyle > STYLE_Count)
	{
		faces.Clear();
		return;
	}

	s_FenceLock.lockForRead();
	if(s_FenceSections[fenceStyle].empty())
	{
		s_FenceLock.unlock();
		// Load the appropriate post model from a file, based on fence style.
		AuDesign* fDesign = 0;
		switch(fenceStyle)
		{
		case Posts:
			fDesign = AuDesign::LoadFromFile(":data/FencePosts.SoH", SOH_FILE_TYPE);
			break;
		case Wire:
			fDesign = AuDesign::LoadFromFile(":data/FenceWire.SoH", SOH_FILE_TYPE);
			break;
		case Range:
			fDesign = AuDesign::LoadFromFile(":data/FenceRange.SoH", SOH_FILE_TYPE);
			break;
		case SplitRail:
			fDesign = AuDesign::LoadFromFile(":data/FenceSplitRail.SoH", SOH_FILE_TYPE);
			break;
		case WoodYard:
			fDesign = AuDesign::LoadFromFile(":data/FenceWoodYard.SoH", SOH_FILE_TYPE);
			break;
		case PostsT:
			fDesign = AuDesign::LoadFromFile(":data/FencePostsT.SoH", SOH_FILE_TYPE);
			break;
		case WireT:
			fDesign = AuDesign::LoadFromFile(":data/FenceWireT.SoH", SOH_FILE_TYPE);
			break;
		case RangeT:
			fDesign = AuDesign::LoadFromFile(":data/FenceRangeT.SoH", SOH_FILE_TYPE);
			break;
		case ChainLink:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLink.SoH", SOH_FILE_TYPE);
			break;
		case ChainLinkRazorWire:
			fDesign = AuDesign::LoadFromFile(":data/FenceChainLinkRazor.SoH", SOH_FILE_TYPE);
			break;
		case SandBagWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceSandBagWall.SoH", SOH_FILE_TYPE);
			break;
		case BrickWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceBrickWall.SoH", SOH_FILE_TYPE);
			break;
		case StoneWall:
			fDesign = AuDesign::LoadFromFile(":data/FenceStoneWall.SoH", SOH_FILE_TYPE);
			break;
		case Custom:
			break;
		case STYLE_Count:
			fDesign = AuDesign::LoadFromFile(":data/FenceFlags.SoH", SOH_FILE_TYPE);
			break;
		}
		if(!fDesign)
		{
			faces.Clear();
			return;
		}
		s_FenceLock.lockForWrite();
		fDesign->VisibleFaces(s_FenceSections[fenceStyle], d_GroupSection);
		delete fDesign;
	}
	faces = s_FenceSections[fenceStyle];
	s_FenceLock.unlock();
}

bool AuScFence::SiteExtents(AuPoint& min, AuPoint& max) const
{
	min = min.Min(m_Segment.m_fLoc1);
	min = min.Min(m_Segment.m_fLoc2);
	max = max.Max(m_Segment.m_fLoc1);
	max = max.Max(m_Segment.m_fLoc2);
	return true;
}

//static
double AuScFence::FencePostSpacing(STYLE fenceStyle)
{
	if(fenceStyle >= STYLE_Count)
		return 1;
	double postSpacing[STYLE_Count] = 
	{
		12,			// Posts
		12,			// Posts T
		12,			// Wire
		12,			// Wire T
		12,			// Range
		12,			// Range T
		10,			// Chain Link
		10,			// Chain Link Razor Wire
		124./12.,	// Split Rail 10'4" spacing.
		8,			// Wood Yard
		10,			// Sand Bag Wall
		12,			// Brick Wall
		16,			// Stone Wall
		0.,			// Custom
	};
	return postSpacing[fenceStyle];
}

double AuScFence::FenceVolume() const
{
	double fenceV = m_Segment.Length();	// one cubic foot per foot of length
	double postV = 1;					// one cubic foot per post

	// Count the volume of both posts even though abutting fence sections could eliminate one of them.
	return (postV*2 + fenceV) * CubicMetersPerCubicFoot();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScSpacecraftParkingSpot::AuScSpacecraftParkingSpot()
{
}

AuScSpacecraftParkingSpot::AuScSpacecraftParkingSpot(const AuScSpacecraftParkingSpot& c) : AuScIcon(c)
{
}

AuScSpacecraftParkingSpot::AuScSpacecraftParkingSpot(const AuScSpacecraftParkingSpot& c, const AuScSelectInfo& selInfo) :
	AuScIcon(c, selInfo)
{
}

void AuScSpacecraftParkingSpot::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScSpacecraftParkingSpot::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScSpacecraftParkingSpot::operator==(const AuScPart& p) const
{
	try
	{
		const AuScSpacecraftParkingSpot& c = dynamic_cast<const AuScSpacecraftParkingSpot&>(p);
		if(!AuScIcon::operator==(c))
			return false;

		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScSpacecraftParkingSpot::Clone() const
{
	return new AuScSpacecraftParkingSpot(*this);
}

AuScPart* AuScSpacecraftParkingSpot::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScSpacecraftParkingSpot(*this, selInfo);
}

QString AuScSpacecraftParkingSpot::TypeIcon() const
{
	return QString(":data/ScParkSpacecraft.png");
}

QString AuScSpacecraftParkingSpot::TypeName() const
{
	return QObject::tr("Spacecraft Parking Spot");
}

double AuScSpacecraftParkingSpot::IconBack() const
{
	return GroundSpawnSizeLimit();
}

double AuScSpacecraftParkingSpot::IconFore() const
{
	return GroundSpawnSizeLimit();
}

double AuScSpacecraftParkingSpot::IconLeft() const
{
	return GroundSpawnSizeLimit();
}

double AuScSpacecraftParkingSpot::IconRight() const
{
	return GroundSpawnSizeLimit();
}

AuMatrix4x4 AuScSpacecraftParkingSpot::PositionMatrix() const
{
	AuPoint o(m_Face.FaceLocation());
	AuPoint n(m_Face.FaceNormal());
	AuPoint x(m_Face.Vertex(3) - m_Face.Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(o, x, y);
}

void AuScSpacecraftParkingSpot::BuildingCursorSegments(AuPointArray& edges) const
{
	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(1));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(2));
	edges.append(m_Face.Vertex(3));

	edges.append(m_Face.Vertex(3));
	edges.append(m_Face.Vertex(0));

	edges.append(m_Face.Vertex(0));
	edges.append(m_Face.Vertex(2));

	edges.append(m_Face.Vertex(1));
	edges.append(m_Face.Vertex(3));
}

bool AuScSpacecraftParkingSpot::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(eyeLoc);
	Q_UNUSED(sightLoc);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	Q_UNUSED(sightSurfaceNormal);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScCamera::AuScCamera()
{
}

AuScCamera::AuScCamera(const AuScCamera& c) : AuScIcon(c)
{
}

AuScCamera::AuScCamera(const AuScCamera& c, const AuScSelectInfo& selInfo) : AuScIcon(c, selInfo)
{
}

void AuScCamera::operator>>(QDataStream& out) const
{
	AuScIcon::operator>>(out);
}

void AuScCamera::operator<<(QDataStream& in)
{
	AuScIcon::operator<<(in);
}

bool AuScCamera::operator==(const AuScPart& p) const
{
	try
	{
		const AuScCamera& c = dynamic_cast<const AuScCamera&>(p);
		if(!AuScIcon::operator==(c))
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScCamera::Clone() const
{
	return new AuScCamera(*this);
}

AuScPart* AuScCamera::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScCamera(*this, selInfo);
}

QString AuScCamera::TypeIcon() const
{
	return QString(":data/ScCamera.png");
}

QString AuScCamera::TypeName() const
{
	return QObject::tr("Camera");
}

QString AuScCamera::SelectedDesc(const AuScParts& parts) const
{
	return FullName(parts);
}

double AuScCamera::IconBack() const
{
	return FeetPerMeter() * .25;
}

double AuScCamera::IconFore() const
{
	return FeetPerMeter() * .25;
}

double AuScCamera::IconLeft() const
{
	return FeetPerMeter() * .25;
}

double AuScCamera::IconRight() const
{
	return FeetPerMeter() * .25;
}

GLuint AuScCamera::OwnTexture(AuGLEffects& re) const
{
	QString texName(":data/ScCamera.png");
	QImage texImg(texName);
	return re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, texImg, GL_LINEAR, GL_LINEAR, true, 1);
}

quint64 AuScCamera::MemoryUsage() const
{
	quint64 mem = AuScIcon::MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBerthName::AuScBerthName()
{
	m_PartId = 0;
}

AuScBerthName::AuScBerthName(const AuScBerthName& c) : AuScScreen(c)
{
	m_PartId = c.m_PartId;
}

AuScBerthName::AuScBerthName(const AuScBerthName& c, const AuScSelectInfo& selInfo) : AuScScreen(c, selInfo)
{
	m_PartId = c.m_PartId;
}

void AuScBerthName::operator>>(QDataStream& out) const
{
	AuScScreen::operator>>(out);
	out << m_PartId;
}

void AuScBerthName::operator<<(QDataStream& in)
{
	AuScScreen::operator<<(in);
	in >> m_PartId;
}

bool AuScBerthName::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBerthName& c = dynamic_cast<const AuScBerthName&>(p);
		if(!AuScScreen::operator==(c))
			return false;
		if(m_PartId != c.m_PartId)
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScBerthName::Clone() const
{
	return new AuScBerthName(*this);
}

AuScPart* AuScBerthName::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScBerthName(*this, selInfo);
}

QString AuScBerthName::TypeIcon() const
{
	return QString(":data/ScName.png");
}

QString AuScBerthName::TypeName() const
{
	return QObject::tr("Berth Assignee's Name");
}

float AuScBerthName::Opacity() const
{
	return m_Face.Opacity();
}

void AuScBerthName::SetOpacity(float opacity)
{
	return m_Face.SetOpacity(opacity);
}

void AuScBerthName::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexEffectIntensity(iv, intensity);
}

void AuScBerthName::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	for(int iv=0; iv<m_Face.VertexCount(); iv++)
		if(selInfo.m_Vertices.contains(iv))
			m_Face.SetVertexGlow(iv, glow);
}

void AuScBerthName::PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(paint);
}

void AuScBerthName::PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	if(selInfo.m_Faces.contains(0))
		m_Face.SetPaint(theme, opacity);
}

void AuScBerthName::PaintInUse(QList<QColor>& paintInUse) const
{
	if(m_Face.IsPainted() && !paintInUse.contains(m_Face.Paint()))
		paintInUse.append(m_Face.Paint());
}

void AuScBerthName::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	if(selInfo.m_Faces.contains(0))
	{
		m_Face.SetShine(shine);
		m_Face.SetReflectivity(reflectivity);
	}
}

void AuScBerthName::SetFaceSelfIllumination(float selfIllum)
{
	m_Face.SetSelfIllumination(selfIllum);
}

GLuint AuScBerthName::OwnTexture(AuGLEffects& re) const
{
	QString text = QObject::tr("Berth Assignee's Name");

	AuGLTextItem textItem;
	re.GetRenderTextItem(textItem, text, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
	return textItem.m_Texture;
}

void AuScBerthName::ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const
{
	Q_UNUSED(pc);
	Q_UNUSED(streamline);
	Q_UNUSED(armor);
	Q_UNUSED(hullMaterial);
}

quint64 AuScBerthName::MemoryUsage() const
{
	quint64 mem = AuScScreen::MemoryUsage();
	mem += sizeof(m_PartId);
	return mem;
}

bool AuScBerthName::IsVisibleInFinalModel() const
{
	return AssociatedPartId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScEnvironmentVoid::AuScEnvironmentVoid()
{
}

AuScEnvironmentVoid::AuScEnvironmentVoid(const AuScEnvironmentVoid& c) : AuScHullVoid(c)
{
}

AuScEnvironmentVoid::AuScEnvironmentVoid(const AuScEnvironmentVoid& c, const AuScSelectInfo& selInfo) : AuScHullVoid(c)
{
	Q_UNUSED(selInfo);
}

bool AuScEnvironmentVoid::operator==(const AuScPart& p) const
{
	try
	{
		const AuScEnvironmentVoid& c = dynamic_cast<const AuScEnvironmentVoid&>(p);
		if(!AuScHullVoid::operator==(c))
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuScPart* AuScEnvironmentVoid::Clone() const
{
	return new AuScEnvironmentVoid(*this);
}

AuScPart* AuScEnvironmentVoid::CloneParts(const AuScSelectInfo& selInfo) const
{
	Q_UNUSED(selInfo);
	return new AuScEnvironmentVoid(*this);
}

QString AuScEnvironmentVoid::SelectedDesc(const AuScParts& parts) const
{
	return QObject::tr("%1, %L2 faces")
		.arg(FullName(parts))
		.arg(m_Faces.size());
}

QString AuScEnvironmentVoid::TypeIcon() const
{
	return QString(":data/ScEnvironmentVoid.png");
}

QString AuScEnvironmentVoid::TypeName() const
{
	return QObject::tr("Environment Void");
}

bool AuScEnvironmentVoid::RandomVoidLocation(quint16 partId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	int wid = max.x - min.x;
	int len = max.y - min.y;
	int ht = max.z - min.z;
	if(wid < 2 || len < 2 || ht < 2)
		return false;

	// Allow a margin.
	wid -= 1;
	len -= 1;
	ht -= 1;

	AuPoint gravityUp(0,0,1);
	AuPoint floorN;
	int tries=20;
	do
	{
		double x = min.x + g_Au.Dice().Roll(1, wid);
		double y = min.y + g_Au.Dice().Roll(1, len);
		double z = min.z + g_Au.Dice().Roll(1, ht);
		AuPoint loc(x,y,z);
		if(IsInsideVoid(partId, loc, doorStates, landingGearStates, liftStops))
		{
			// Found a location inside the void. Move down to just above the floor.
			voidLoc = loc - gravityUp * (HeightAboveFloor(loc, gravityUp, AuMatrix4x4::IdentityMatrix(), floorN) - .1);
			return true;
		}
	}
	while(tries--);
	return false;
}

double AuScEnvironmentVoid::HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM,
		AuPoint& floorN) const
{
	AuPoint floorLoc = FaceLoc(loc, -gravityN, floorN, stopM);
	return floorLoc.Distance(loc);
}

bool AuScEnvironmentVoid::IsInsideEnvironmentVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
{
	return IsInsideVoid(partId, loc, doorStates, landingGearStates, liftStops);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScBuildZone::AuScBuildZone()
{
}

AuScBuildZone::AuScBuildZone(const AuScBuildZone& c) : AuScMeshBase(c)
{
}

AuScBuildZone::AuScBuildZone(const AuScBuildZone& c, const AuScSelectInfo& selInfo) : AuScMeshBase(c, selInfo)
{
}

void AuScBuildZone::operator>>(QDataStream& out) const
{
	AuScMeshBase::operator>>(out);
}

void AuScBuildZone::operator<<(QDataStream& in)
{
	AuScMeshBase::operator<<(in);
}

bool AuScBuildZone::operator==(const AuScPart& p) const
{
	try
	{
		const AuScBuildZone& c = dynamic_cast<const AuScBuildZone&>(p);
		if(!AuScMeshBase::operator==(c))
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

bool AuScBuildZone::IsValid() const
{
	return AuScMeshBase::IsValid();
}

AuScPart* AuScBuildZone::Clone() const
{
	return new AuScBuildZone(*this);
}

AuScPart* AuScBuildZone::CloneParts(const AuScSelectInfo& selInfo) const
{
	return new AuScBuildZone(*this, selInfo);
}

QString AuScBuildZone::TypeIcon() const
{
	return QString(":data/ScBuildZone.png");
}

QString AuScBuildZone::TypeName() const
{
	return QObject::tr("Build Zone");
}

QString AuScBuildZone::SelectedDesc(const AuScParts& parts) const
{
	QString desc = AuScMeshBase::SelectedDesc(parts);
	return desc;
}

bool AuScBuildZone::Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
{
	Q_UNUSED(partId);
	Q_UNUSED(parts);
	Q_UNUSED(shipHull);
	if(IsOmitted())
		return false;
	return true;
}

bool AuScBuildZone::Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
		const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const
{
	Q_UNUSED(partId);
	Q_UNUSED(doorStates);
	Q_UNUSED(landingGearStates);
	Q_UNUSED(liftStops);
	return m_Faces.Sight(eyeLoc, sightLoc, sightSurfaceNormal);
}

bool AuScBuildZone::SightBuildZone(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	AuPoint sightSurfaceNormal;
	return m_Faces.Sight(eyeLoc, sightLoc, sightSurfaceNormal);
}

void AuScBuildZone::MakeBuildZone(const AuPointArray& bnd, const AuMatrix4x4& gridPlane)
{
	m_Faces.Clear();

	// Transform the boundary to the real XY plane so the math is easier.
	AuPointArray cbnd(bnd);
	cbnd.Transform(gridPlane.Inverse());

	// Prepare boundary that will be used for the top and bottom.
	if(!cbnd.IsClockwise())
		cbnd.ReverseOrder();

	// Triangulate the boundary with edge lines at the boundary.
	AuPointArray tris;
	QVector<bool> edges;
	cbnd.TriangulateBoundary(tris, edges);

	m_Faces.AddFace(AuScFacePolygon(tris, edges));

	// Transform the result to the design plane.
	m_Faces.Transform(gridPlane);
}

bool AuScBuildZone::InBuildZone(const AuPoint& loc) const
{
	// Check to see if 'loc' is above my mesh by shooting a ray down from it.
	if(loc.z < m_Faces.Min().z)
		return false;	// Location is below my lowest extent.
	AuPoint intersectLoc;
	return m_Faces.IntersectRay(loc, AuPoint(0,0,-1), intersectLoc);
}

bool AuScBuildZone::InBuildZone(const AuPoint& loc, double withinHeight) const
{
	// Check to see if 'loc' is above my mesh by shooting a ray down from it.
	if(loc.z < m_Faces.Min().z)
		return false;	// Location is below my lowest extent.
	AuPoint intersectLoc;
	if(!m_Faces.IntersectRay(loc, AuPoint(0,0,-1), intersectLoc))
		return false;	// Ray does not intersect my faces.
	return loc.z - intersectLoc.z <= withinHeight;
}

void AuScBuildZone::BuildingCursorSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

void AuScBuildZone::BuildingCursorSiteSegments(AuPointArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

quint64 AuScBuildZone::MemoryUsage() const
{
	quint64 mem = AuScMeshBase::MemoryUsage();
	return mem;
}

void AuScBuildZone::BuildingConstructionSegments(AuSegmentArray& edges) const
{
	m_Faces.EdgeSegments(edges);
}

bool AuScBuildZone::RandomBuildZoneLocation(AuPoint& siteLoc) const
{
	AuPoint min,max;
	m_Faces.Extents(min, max);
	int wid = max.x - min.x;
	int len = max.y - min.y;
	if(wid < 2 || len < 2)
		return false;

	// Allow a margin.
	wid -= 1;
	len -= 1;

	int tries=20;
	do
	{
		double x = min.x + g_Au.Dice().Roll(1, wid);
		double y = min.y + g_Au.Dice().Roll(1, len);
		double z = min.z;
		AuPoint loc(x,y,z);
		if(m_Faces.InSite(loc))
		{
			// Found a location in the build zone.
			siteLoc = loc;
			return true;
		}
	}
	while(tries--);
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
