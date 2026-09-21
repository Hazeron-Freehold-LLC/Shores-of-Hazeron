//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesign.h,v 1.104 2022/02/19 18:19:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesign_H
#define AuDesign_H

#include "audata.h"
#include "augleffects.h"
#include "auid.h"
#include "aupose.h"
#include "dbcommodity.h"

class AuDoorStateHash;
class AuGLEffects;
class AuLiftHash;
class AuScFaceList;
class AuScPart;
class AuScParts;
class AuScPartSet;
class AuScReportGameTableHash;
class AuScReportLiftHash;
class AuScReportServicePanelHash;
class AuScReportStatusDisplayHash;
class AuScSelectInfo;
class AuScSelection;
class DeProcessComponentArray;
class QProgressDialog;

// A design texture can be a custom image or it can refer to one of our texture library textures.
class AUDATA_DLL AuDesignTex
{
	QImage								m_Image;				// Can be a custom image, or
	QString								m_ImageName;			// The file name sans path.
	quint16								m_TexLib;				// Can be an AuTextureLibItem id.

public:
	inline AuDesignTex()
	{
		m_TexLib = 0;
	}

	inline AuDesignTex(const AuDesignTex& c)
	{
		m_Image = c.m_Image;
		m_ImageName = c.m_ImageName;
		m_TexLib = c.m_TexLib;
	}

	inline const AuDesignTex& operator=(const AuDesignTex& c)
	{
		m_Image = c.m_Image;
		m_ImageName = c.m_ImageName;
		m_TexLib = c.m_TexLib;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_TexLib;
		out << m_Image;
		out << m_ImageName;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_TexLib;
		in >> m_Image;
		in >> m_ImageName;
	}

public:
	inline const QImage& Image() const
	{
		return m_Image;
	}

	inline const QString& ImageName() const
	{
		return m_ImageName;
	}

	bool IsValid() const;

	static int MaxImageDimension();

	quint64 MemoryUsage() const;

	inline quint16 TexLibId() const
	{
		return m_TexLib;
	}

	inline void SetImage(const QString& name, const QImage& image)
	{
		m_Image = image;
		m_ImageName = name;
		m_TexLib = 0;
	}

	inline void SetTexLibId(quint16 texLib)
	{
		m_TexLib = texLib;
		m_Image = QImage();
		m_ImageName = QString();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuDesignTex& dt)
{
	dt >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDesignTex& dt)
{
	dt << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDesignPosition
{
	AuMatrix4x4							m_EnvM;
	float								m_HeadTilt;
	AuPose								m_Pose;

public:
	inline AuDesignPosition(const AuMatrix4x4& envM, float headTilt, const AuPose& pose) : m_EnvM(envM), m_Pose(pose)
	{
		m_HeadTilt = headTilt;
	}

	inline AuDesignPosition()
	{
		m_HeadTilt = 0;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_Pose;
		out << m_HeadTilt;
		out << m_EnvM;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Pose;
		in >> m_HeadTilt;
		in >> m_EnvM;
	}

public:	// Methods
	inline const AuMatrix4x4& EnvTransform() const
	{
		return m_EnvM;
	}

	inline float HeadTilt() const
	{
		return m_HeadTilt;
	}

	bool IsValid() const;

	inline const AuPose& Pose() const
	{
		return m_Pose;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuDesignPosition& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDesignPosition& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDesign
{
public:
	// This value gets serialized. Add new entries at the end. Never change the values of existing enums.
	enum BLUEPRINT_TYPE
	{
		BLUEPRINT_Invalid,
		BLUEPRINT_Spacecraft,
		BLUEPRINT_Building,
		BLUEPRINT_Item,
		BLUEPRINT_Count					// LAST ITEM
	};

	// States control what can be done with a design.
	enum STATE
	{
		Edit,							// Design is being developed. Can be modified. Cannot be manufactured.
		Final,							// Design is complete and finalized. Can no longer be changed. Can be manufactured.
		Deleted,						// Deleted by architect. Still exists but no longer shown to architect in lists.
		FinalAssembly					// Design is finalized as an assembly. Can no longer be changed. Cannot be manufactured.
	};

private:
	AuId								m_ArchId;				// architect a_Person atom id
	QString								m_ArchName;				// name of the architect a_Person atom
	QString								m_Description;			// Description of this design.
	QHash<quint8, AuMatrix4x4>			m_GridPos;				// Saved grid positions. Key is position number, 1 to 9.
	quint32								m_Id;
	QString								m_Name;					// Name of this design.
	quint8								m_State;				// STATE
protected:
	AuDesignTex							m_TexturePalette[SCTEXI_Count];
private:
	QImage								m_Thumbnail;			// For selection in design library.
	QHash<quint8, AuDesignPosition>		m_ViewPos;				// Saved view positions. Key is position number, 1 to 9.

public:	// Constructors
	AuDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect);
	AuDesign();
	virtual ~AuDesign();
	AuDesign(const AuDesign &c);

public:	// Operators
	AuDesign& operator=(const AuDesign &c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuDesign& c) const
	{
		Q_ASSERT(false);				//DHH Function added to satisfy dll export build in Windows. Does anybody use it?
		return m_Id == c.m_Id;
	}

public:	// Methods
	inline const AuId& ArchitectId() const
	{
		return m_ArchId;
	}

	inline const QString& ArchitectName() const
	{
		return m_ArchName;
	}

	// Returns faces of barriers from a particular group of the design. Used by fences to get their Post and Section barriers.
	void BarrierFaces(AuScFaceList& barrierFaces, const QString& groupName) const;

	// 'which' is a part id.
	COMMODITYID BayModule(quint16 which) const;

	quint16 Beds() const;

	AuPoint BerthLocation(quint16 partId) const;

	quint16 CaptBerths() const;

	void CitizenPostLocs(QHash<quint16, AuPoint>& locs) const;

	void CitizenPosts(AuScPartSet& posts) const;

	// Returns a duplicate of this design, allocated on the heap.
	virtual AuDesign* Clone() const = 0;

	// Clears architect information from the design and from all parts whose architect is 'id'.
	// This is done in preparation for saving a design to a file. See corresponding SetArchitectAfterFileLoad function.
	void ClearArchitectForFileSave(const AuId& id);

	virtual void ClearParts() = 0;

	// For buildings.
	void ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestPowerPlant, AuPoint& closestSensor) const;

	// For spacecraft
	void ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestManeuverDrive, AuPoint& closestPowerPlant,
			AuPoint& closestSensor, AuPoint& closestFTLDrive, AuPoint& closestSiren) const;

	quint16 ClosestRemoteDoorInRoom(bool forVehicle, const AuPoint& loc, quint16 roomId, const AuDoorStateHash& doorStates) const;

	// Fills 'stations' with part ids that create a command station.
	void CommandStations(AuScPartSet& stations) const;

	virtual unsigned int CompressedSizeLimit() const = 0;

	// Fills 'contrails' with information about contrail emitters.
	// Key is location; value is diameter of emitter.
	virtual void ContrailEmitters(QMap<AuPoint, float>& contrails) const;

	// Fills 'stations' with part ids that create a control station.
	// Control stations are operated by crew.
	void ControlStations(AuScPartSet& stations) const;

	static AuDesign* Create(BLUEPRINT_TYPE designType);

	// Creates a custom MIME data object of the parts in 'partIds'. Coordinates are relative to 'designPlane' by
	// transforming the parts through the inverse of the designPlane.
	QMimeData* CreateMIME(const AuId& architectId, bool objectMode, const AuScSelection& partIds,
			const AuMatrix4x4& designPlane) const;

	quint16 CrewBerths() const;

	void CutFaces(const AuScSelection& sel, const AuPointArray& cutLine, AuScParts& partsChanged);

	inline const QString& Description() const
	{
		return m_Description;
	}

	// Returns the description as a single line.
	// Format example: Streamlined Starship 32m^3 (using cubed symbol instead of ^3).
	virtual QString DescriptionOneLine() const
	{
		return Description();
	}

	bool DeselectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
			bool findEdges, bool findFaces) const;

	// Deselects all vertices on selected parts that are at 'partId's vertex at 'partVertex'.
	// Used by the texture unwrapper. 'partId' is expected to already be in 'sel'.
	bool DeselectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const;

	// Fills 'stations' with part ids that create a designer station.
	// Designer stations are not operated by crew.
	void DesignerStations(AuScPartSet& stations) const;

	// Copies only the fields the server wants from a design record loaded from a file is sent to the server.
	virtual void DesignLoaded(const AuDesign& design);

	// Collects information about all doors.
	void DoorStates(AuDoorStateHash& doors) const;

	// Returns surface effect common to all selected faces.
	// Returns EFF_Count if they differ.
	SURFACE_EFFECT Effect(const AuScSelection& sel) const;

	// Fills 'xrefs' with any external design ids referenced by this design. i.e. custom fences.
	void ExternalReferences(QSet<quint32>& xrefs) const;

	void Faces(AuScFaceList& faces) const;

	void Faces(AuScFaceList& faces, const AuScSelection& sel) const;

	// Lists only faces that are not part of omitted parts.
	void FacesNotOmitted(AuScFaceList& faces) const;

	// Fills 'textures' with the palette of textures used for all fence and wall geometry.
	static void FenceTexturePalette(AuGLEffects& re, AuTexturePalette& textures);

	// Prepares the design to be saved as a read-only assembly, ready to be published
	// Minimal verification is performed, mainly insuring the design has a valid name.
	virtual bool FinalizeAssembly(QStringList& msgs) = 0;

	// Prepares the design to be saved as a read-only design, ready to be published and manufactured in the game universe.
	virtual bool FinalizeDesign(QStringList& msgs, QProgressDialog& progress) = 0;

	// Prepares the design to be used for a preview in the designer.
	virtual void FinalizePreview() = 0;

	quint16 FindEdgeAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& sel, bool objectMode,
			bool objectIsolate, AuPoint* snapLoc=0) const;

	// 'partType' is an AuScPart::PART_TYPE.
	// Returns part id of a 'partType' part.
	// Returns 0 if none is found.
	quint16 FindPart(quint8 partType) const;

	quint16 FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, bool findVertices, bool findEdges,
			bool findFaces) const;

	// Only looks at parts that are already in 'partIds'.
	quint16 FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds,
			bool findVertices, bool findEdges, bool findFaces) const;

	// For each selected part in 'partIds', adds all things that fall within 'pixelRect' to its corresponding selection entry.
	// 'pixelRect' is in OpenGL projected viewport coords, i.e. (-1,-1, to 1,1 with 0,0 at center of screen).
	// 'partIds' will contain vertex indexes that fall within the selection.
	void FindPartObjectsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices,
			bool findEdges, bool findFaces) const;

	void FindPartsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices,
			bool findEdges, bool findFaces) const;

	quint16 FindVertexAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& sel, bool objectMode,
			bool objectIsolate, AuPoint* snapLoc=0) const;

	// Repairs part associations after a copy/paste operation. The pasted parts are assigned nu part ids. Associations within those
	// parts are repaired to point at the corresponding nu parts.
	// 'nuPartIds' key=old part id, value=copied part id
	void FixPartAssociations(const QHash<quint16, quint16>& nuPartIds);

	// 'which' is a part id.
	COMMODITYID FTLDriveModule(quint16 which) const;

	void GameTables(AuScReportGameTableHash& tables) const;

	// Returns 0=ok, 1=fail invalid 'posId', 2=no stored pos to restore, 3=stored pos is invalid
	int GridPositionLoad(quint8 posId, AuMatrix4x4& pos) const;

	// Returns the list of currently stored grid position ids. These are the keys of the m_GridPos hash.
	QSet<quint8> GridPositions() const;

	// Returns 0=added, 1=replaced, 2=fail invalid 'posId', 3=fail invalid 'pos'
	int GridPositionSave(quint8 posId, const AuMatrix4x4& pos);

	// Returns the orientation for objects lying on the ground.
	virtual GROUND_ORIENT GroundOrientation() const
	{
		return GORI_ZPosUp;
	}

	quint16 GroundVehicleParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void GroundVehicleParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot.
	void GroundVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	void GuardPostLocs(QHash<quint16, AuPoint>& locs) const;

	void GuardPosts(AuScPartSet& posts) const;

	bool HasHelm() const;

	// Returns true if the design contains any parts that were sealed by an architect EXCEPT 'architectId'.
	bool HasSealedParts(const AuId& archId) const;

	// Returns true if the selection contains any parts that were sealed by an architect EXCEPT 'architectId'.
	bool HasSealedParts(const AuId& archId, const AuScSelection& sel) const;

	AuPoint HelmLocation() const;

	// Returns the material that corresponds to the hull module.
	inline COMMODITYID HullMaterial() const
	{
		return HullModuleMaterial(HullModule());
	}

	virtual COMMODITYID HullModule() const
	{
		return CID_Invalid;
	}

	inline quint32 Id() const
	{
		return m_Id;
	}

	// Inverts the part object selection in 'sel'.
	void InvertPartObjectSelection(AuScSelection& sel) const;

	// True if the design is marked as deleted in the library.
	inline bool IsDeleted() const
	{
		return m_State == Deleted;
	}

	// True if the design has been finalized. That means it has an up-to-date collision model and thumbnail.
	inline bool IsFinal() const
	{
		return m_State == Final || m_State == FinalAssembly;
	}

	// Returns true if 'loc' is inside the obstruction model of the hull, considering exterior voids.
	// Returns true if 'loc' is inside a room void or a hull or landing gear.
	// Returns false if 'loc' is inside a hull void or outside the hull obstruction model completely.
	// This is a rough test for being inside the hull or landing gear.
	bool IsInsideHull(const AuPoint& loc) const;

	bool IsInsideObstructionModel(const AuPoint& loc) const;

	// Returns true if 'roomId' is a room void associated with the open state of 'doorId'.
	bool IsOpenDoorVoid(quint16 roomId, quint16 doorId) const;

	// Test for obstruction from 'fromLoc' to 'toLoc'.
	// Returns true if blocked and 'hitNormal' is set for calculation of bounce/reflection of movement.
	// Returns false if not blocked and 'hitNormal' is unchanged.
	bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	// True if the design cannot be overwritten in the library.
	inline bool IsReadOnly() const
	{
		return !IsWritable();
	}

	// True if the design can be overwritten in the library.
	inline bool IsWritable() const
	{
		return m_State == Edit;
	}

	// Fills 'landingGear' with part ids.
	void LandingGear(AuScPartSet& landingGear) const;

	void LandingSystemPosition(quint16 landerId, AuMatrix4x4& landerPosM) const;

	void LaunchPosition(quint16 launcherId, AuMatrix4x4& launchPadM, AuPoint& launchExit) const;

	//virtual void ListComponents(DeProcessComponentArray& pc) const = 0;

	void ListRoomNames(QStringList& roomNames) const;

	void LivestockPostLocs(QHash<quint16, AuPoint>& locs) const;

	void LivestockPosts(AuScPartSet& posts) const;

	// The design is loaded using Unpack. The caller now owns the pointer.
	// 'fileType' .SoH file uses SOH_FILE_TYPE.
	// Use a different 'fileType' when loading from a cache so the file is not identical to a .SoH file.
	static AuDesign* LoadFromByteArray(const QByteArray& fileData, quint8 fileType);

	static AuDesign* LoadFromDataStream(QDataStream& ds, quint8 fileType);

	static AuDesign* LoadFromFile(const QString& fileName, quint8 fileType);

	void Loop(const AuScSelection& sel, const AuPoint& loc, AuPointArray& loop) const;

	virtual quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	quint16 OfficerBerths() const;

	// Returns the seat location of a console or station.
	AuPoint OperatorLocation(quint16 partId) const;

	// Packs this designs's type and data into a byte array using operator>>. Use Unpack to restore.
	QByteArray Pack() const;

	// Retrieves a list of all unique paint colors in use by faces.
	void PaintInUse(QList<QColor>& paintInUse) const;

	// Returns part id if successful.
	// 'partsAdded' is filled with clones for caller creation of undo data.
	virtual quint16 PartAdd(const AuScPart& part, AuScParts& partsAdded);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartAddFaces(quint16 partId, const AuScFaceList& faces, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartAddState(quint16 partId, quint8 state, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartAddStop(quint16 partId, const QString& stop, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsDelete' is filled with deleted parts for caller creation of undo data.
	virtual bool PartDelete(quint16 partId, AuScParts& partsDeleted);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartDeleteState(quint16 partId, quint8 state, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartDeleteStop(quint16 partId, const QString& stop, AuScParts& partsChanged);

	// Collects properties from selection. Sets uncommon values to invalid/grey states.
	void PartDoorProperties(const AuScSelection& partIds, Qt::CheckState& isAutomatic, Qt::CheckState& initLocked,
			Qt::CheckState& remoteSpacecraft, Qt::CheckState& remoteVehicle);

	// For each part in 'partIds', fills the edge arrays with all possible edge ids.
	void PartEdgeIds(AuScSelection& partIds) const;

	// For each part in 'partIds', fills the face arrays with all possible face ids.
	void PartFaceIds(AuScSelection& partIds) const;

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartFacesDivide(const AuScSelection& sel, quint8 divisions, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartFacesFracture(const AuScSelection& sel, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartFacesFuse(const AuScSelection& sel, AuScParts& partsChanged);

	// 'paint' alpha value is the blend factor.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartFacesPaint(const AuScSelection& sel, const QColor& paint, AuScParts& partsChanged);

	// 'theme' is a AuScFace::COLOR_THEME.
	bool PartFacesPaint(const AuScSelection& sel, quint8 theme, quint8 opacity, AuScParts& partsChanged);

	bool PartFacesPolish(const AuScSelection& sel, float shine, float reflectivity, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartFacesReverse(const AuScSelection& sel, AuScParts& partsChanged);

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

	// Acquires a list of unique group names from existing parts. Result will include a blank entry if any of them are blank.
	void PartGroupNames(QStringList& groupNames) const;

	// Fills 'partIds' with a selection object for every part, with sub-vertex arrays empty.
	void PartIds(AuScSelection& partIds) const;

	// Fills 'partIds' with a selection object for every part that is not omitted, with sub-vertex arrays empty.
	void PartIdsNotOmitted(AuScSelection& partIds) const;

	void PartLightBulbProperties(const AuScSelection& partIds, QColor& color, float& kL, float& kQ, float& cosOuterCone,
			float& cosInnerCone) const;

	void PartLightLensProperties(const AuScSelection& partIds, QColor& color) const;

	// Returns the module associated with 'partId'.
	COMMODITYID PartModule(quint16 partId) const;

	QString PartName(quint16 partId) const;

	// Adds part objects (vertices, edges, faces) of 'part' to 'partId'.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartObjectAdd(quint16 partId, const AuScPart& part, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	// 'partsDeleted' is filled with clones for caller creation of undo data.
	virtual bool PartObjectDelete(quint16 partId, const AuScSelectInfo& selInfo, AuScParts& partsChanged, AuScParts& partsDeleted);

	// For each part in 'partIds', fills the edge, face and vertex arrays with all possible ids.
	void PartObjectIds(AuScSelection& partIds) const;

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartRenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName, AuScParts& partsChanged);

	QString PartReportDesc(quint16 partId) const;

	virtual const AuScParts& Parts() const = 0;

	virtual AuScParts& Parts() =  0;

	// Associates paths and voids with the state of the one single selected door or landing gear.
	// Associates service panels with a system controlled by a station.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsAssociate(const AuScSelection& sel, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsAdded' is filled with clones for caller creation of undo data.
	virtual bool PartsCopy(const AuScSelection& partIds, AuScParts& partsAdded);

	virtual bool PartsCopySymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane, AuScParts& partsAdded);

	bool PartsDissociate(const AuScSelection& sel, AuScParts& partsChanged);

	// Sets the face surface effect without changing texture coordinates.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectFaces(const AuScSelection& sel, SURFACE_EFFECT effect, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectFacesCylindrical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectFacesProject(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane,
			AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectFacesSpherical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	// Moves texture coordinates on selected vertices using 'vec'.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged);

	// Rotates texture coordinates on selected vertices around 'pivot', which is in texture coordinates.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged);

	// Stretches texture coordinates on selected vertices from 'base', which is in texture coordinates.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY, AuScParts& partsChanged);

	// 'unwrap' is a AuScFaceList::UNWRAP_METHOD.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsEffectUnwrap(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize, quint8 unwrap, float angleTol,
			bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	QString PartSelectedDesc(quint16 partId) const;

	bool PartSetGroup(quint16 partId, const QString& groupName, AuScParts& partsChanged);

	bool PartSetHidden(quint16 partId, bool hide, AuScParts& partsChanged);

	bool PartSetLight(quint16 partId, const QColor& color, float kL, float kQ, float cosOuterCone, float cosInnerCone,
			AuScParts& partsChanged);

	bool PartSetLocked(quint16 partId, bool lock, AuScParts& partsChanged);

	virtual bool PartSetOmitted(quint16 partId, bool omit, AuScParts& partsChanged);

	bool PartSetOpacity(quint16 partId, float opacity, AuScParts& partsChanged);

	virtual bool PartSetSealed(quint16 partId, bool seal, AuScParts& partsChanged);

	bool PartSetState(quint16 partId, quint8 state, AuScParts& partsChanged);

	// Sets the current stop position.
	// The empty stop position is the natural model position for editing. We only edit it in that position.
	bool PartSetStop(quint16 partId, const QString& stop, AuScParts& partsChanged);

	// Renames the stop name at a door 'partId'.
	bool PartSetStopName(quint16 partId, const QString& name, AuScParts& partsChanged);

	// Collects properties from selection. Sets uncommon values to invalid/grey states.
	void PartSiteProperties(const AuScSelection& partIds, quint8& grade, quint8& grass, quint8& pavement, quint8& rocks,
			quint8& shrubs, quint8& terBlend, quint8& trees);

	// Uses selected jigs to cut meshes.
	// 'archId' is used to prevent cutting sealed parts that were made by another architect.
	// Returns true if any changes are made.
	// 'partsAdded' is filled with clones for caller creation of undo data.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	// 'partsDeleted' is filled with clones for caller creation of undo data.
	bool PartsJigCut(const AuId& archId, const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged,
			AuScParts& partsDeleted);

	// Adds part objects (vertices, edges, faces) of all selected parts to the first selected part.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	// 'partsDeleted' is filled with clones for caller creation of undo data.
	bool PartsJoin(quint16 targetPart, const AuScSelection& sel, AuScParts& partsChanged, AuScParts& partsDeleted);

	// Converts each selected part into a decal if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeDecal(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a door if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeDoor(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a hull if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeHull(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a landing gear if possible. Converted parts are deleted, moved to 'partsDeleted'.
	virtual bool PartsMakeLandingGear(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a light lens if possible. Converted parts are deleted, moved to 'partsDeleted'.
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

	// Switch locations can be AuPoint::Invalid to restore them to the center of the closed door geometry.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsMoveDoorSwitches(const AuScSelection& sel, const AuPoint& switch1, const AuPoint& switch2,
			AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetDoorProperties(const AuScSelection& sel, Qt::CheckState isAutomatic, Qt::CheckState initLocked,
			Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetFaceOpacity(const AuScSelection& sel, float opacity, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetFaceSelfIllumination(const AuScSelection& sel, float selfIllum, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetLightBulbProperties(const AuScSelection& sel, const QColor& color, float kL, float kQ, float cosOuterCone,
			float cosInnerCone, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetLightLensProperties(const AuScSelection& sel, const QColor& color, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetRoomProperties(const AuScSelection& sel, bool isSunlit, INTERIOR_LIGHT initLighting, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetSiteProperties(const AuScSelection& sel, quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs,
			quint8 terBlend, quint8 trees, AuScParts& partsChanged);

	// 'personnelAccess' is a bitwise combination of PERSONNEL_ACCESS bits.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSetVoidProperties(const AuScSelection& sel, GRAVITY gravityType, const AuPoint& gravityUp, HOLD_ACCESS holdAccess,
			quint8 personnelAccess, AuScParts& partsChanged);

	// Removes selected part objects and creates a new part from them.
	// Returns true if any changes are made.
	// 'partsAdded' is filled with clones for caller creation of undo data.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsSplit(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged);

	// Sets the face texture index without changing texture coordinates.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureFaces(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureFacesCylindrical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator, double texSize,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureFacesProject(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize, const AuMatrix4x4& gridPlane,
			AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureFacesSpherical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	// Moves texture coordinates on selected vertices using 'vec'.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged);

	// Rotates texture coordinates on selected vertices around 'pivot', which is in texture coordinates.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged);

	// Stretches texture coordinates on selected vertices from 'base', which is in texture coordinates.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY, AuScParts& partsChanged);

	// 'unwrap' is a AuScFaceList::UNWRAP_METHOD.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartsTextureUnwrap(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize, quint8 unwrap, float angleTol,
			bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	// Returns the current stop name of a part.
	QString PartStop(quint16 partId) const;

	void PartStops(quint16 partId, QStringList& stations) const;

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartsTransform(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged, bool objectMode);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	// This is used in object mode to drag copy selected portions of one or more parts.
	virtual bool PartsTransformCopy(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged);

	virtual bool PartsTransformLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged);

	// Sets hull door state on selected doors according to what voids they interconnect.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool PartsUpdateHullDoors(const AuScSelection& sel, AuScParts& partsChanged);

	// Returns true if any changes are made.
	// 'partsUpdated' is filled with clones for caller creation of undo data.
	virtual bool PartUpdate(quint16 partId, const AuScPart& part, AuScParts& partUpdated);

	// For each part in 'partIds', fills the vertex arrays with all possible vertex ids.
	void PartVertexIds(AuScSelection& partIds) const;

	bool PartVerticesEffectIntensity(const AuScSelection& sel, float intensity, AuScParts& partsChanged);

	// Flattens lighting normals at selected vertices of selected faces.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartVerticesFlatten(const AuScSelection& sel, AuScParts& partsChanged);

	bool PartVerticesGlow(const AuScSelection& sel, const QColor& glow, AuScParts& partsChanged);

	// Smooths lighting normals at selected vertices of selected faces.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool PartVerticesSmooth(const AuScSelection& sel, AuScParts& partsChanged);

	quint16 PassengerBerths() const;

	void PunchFaces(const AuScSelection& sel, const AuPointArray& cutBnd, AuScParts& partsChanged);

	// Queries gravity at 'loc'.
	// Returns the fall distance from loc to the floor, in the direction of gravity.
	// 'gravityN' and 'terN' are output parameters. They will be (0,0) if loc is not inside any room voids.
	GRAVITY QueryGravity(const AuPoint& loc, AuPoint& floorLoc, AuPoint& gravityN, AuPoint& terN) const;

	bool ReadTextures(QDataStream& in);

	void ReadTexturesOld(QDataStream& in);

	// Rebuilds all obstruction data.
	virtual void RefreshObstructionsCache() = 0;
	virtual void RefreshObstructionsCache(QProgressDialog& progress) = 0;

	// Rebuilds obstruction data if it appears to be missing.
	// Returns true if anything changes.
	virtual bool RefreshObstructionsCacheIfNeeded() = 0;

	// Returns the location of 'cameraId'.
	AuPoint RemoteCameraLocation(quint16 cameraId) const;

	// Returns the id of the next camera after 'currentCamera', which may be 0 to start the sequence at the beginning.
	// Returns 0 if there are no cameras.
	quint16 RemoteCameraNext(quint16 currentCamera) const;

	// Returns the id of the previous camera before 'currentCamera', which may be 0 to start the sequence at the end.
	// Returns 0 if there are no cameras.
	quint16 RemoteCameraPrevious(quint16 currentCamera) const;

	// Returns true if 'cameraId' identifies a valid part.
	bool RemoteCameraVerify(quint16 cameraId) const;

	// 'isDragging' is true if selected parts are being dragged with the mouse.
	// 'dragT' is applied to selected parts when 'isDragging' is true.
	// 'dragCopy' is true if the selected parts are being copied as they are dragged, to paint them in both places.
	// 'objectIsolate' is true to hide parts that are not selected when in object mode.
	void RenderEdges(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re, const AuScSelection& sel,
			bool objectMode, bool isDragging, const AuMatrix4x4& dragT, bool dragCopy, bool objectIsolate);

	// Only edges of selected parts are rendered.
	// 'dragT' is applied to selected parts.
	void RenderEdgesDragging(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			const AuScSelection& sel, bool objectMode, const AuMatrix4x4& dragT, bool showVertexSprites, bool showFaceSprites);

	// Display face sprites in object mode.
	void RenderFaceSprites(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re, const AuScSelection& sel,
			const AuMatrix4x4& dragT);

	// Display vertex sprites in object mode.
	void RenderVertexSprites(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
			const AuScSelection& sel, const AuMatrix4x4& dragT);

	virtual QString ReportAnalysis() const = 0;

	quint16 RocketParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void RocketParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot.
	void RocketParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	INTERIOR_LIGHT RoomLight(const QString& roomName) const;

	INTERIOR_LIGHT RoomLight(quint16 roomId) const;

	// Returns the part id of the room void that contains 'loc'.
	// Beware this could be ambiguous because room voids can overlap.
	quint16 RoomVoidContainingLoc(const AuPoint& loc) const;

	// Returns the part id of the room void that contains 'loc', elevated by 'elevate' with respect to gravity in each room..
	// Beware this could be ambiguous because room voids can overlap.
	quint16 RoomVoidContainingLoc(const AuPoint& loc, double elevate) const;

	AuPoint RoomVoidGravityUp(quint16 roomId, const AuLiftHash& liftStops) const;

	// On entry, 'gravityN' is the environmental gravity up vector. On exit, it may be changed to the gravity up vector in the
	// room void containing 'loc', if the room's artificial gravity overrides the incoming 'gravityN'.
	// 'getRoomUp' is the caller wants 'gravityN' set to the void's gravity up vector, regardless of its gravity settings.
	// regardless of external gravity conditions or void gravity settings.
	double RoomVoidHeightAboveFloor(quint16 roomId, const AuPoint& loc, bool getRoomUp, AuPoint& gravityN,
			const AuLiftHash& liftStops) const;

	HOLD_ACCESS RoomVoidHoldAccess(quint16 roomId) const;

	// Returns true if 'voidId' is normally sunlit. Does not determine current lighting conditions, just whether sunlight normally
	// illuminates the interior of a room.
	// CAVEAT: More than one room can have the same name.
	bool RoomVoidIsSunlit(quint16 voidId) const;

	// Returns the name of a specific room void.
	QString RoomVoidName(quint16 roomId) const;

	// Fills 'rooms' with names of room voids. Multiple voids may share the same name. They are treated as a single void for
	// air pressure and lighting purposes.
	void RoomVoidNames(QStringList& rooms) const;

	virtual void Rx(QDataStream& in);

	// 'fileType' .SoH file uses SOH_FILE_TYPE.
	// Use a different 'fileType' when storing to a cache so the file is not identical to a .SoH file.
	bool SaveToFile(const QString& fileName, quint8 fileType) const;

	// Adds to 'sel' the faces and vertices that are contiguous and in the same plane as the face(s) already selected, within
	// 'angleTol'.
	bool SelectContiguous(double angleTol, AuScSelection& sel) const;

	bool SelectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
			bool findEdges, bool findFaces) const;

	// Selects all vertices on selected parts that are at 'partId's vertex at 'partVertex'.
	// Used by the texture unwrapper. 'partId' is expected to already be in 'sel'.
	bool SelectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const;

	COMMODITYID SensorModule() const;

	static double SensorRangeDeepSpace(double sensorRangeFt);

	void ServicePanels(AuScReportServicePanelHash& tables) const;

	// Sets the architect information in the design and all parts where the architect is 0.
	// This is done after a design is loaded from a file. See corresponding ClearArchitectForFileSave function.
	void SetArchitectAfterFileLoad(const AuId& id, const QString& name);

	inline void SetArchitectId(const AuId& archId)
	{
		m_ArchId = archId;
	}

	inline void SetArchitectName(const QString& name)
	{
		m_ArchName = name;
	}

	inline void SetDescription(const QString& desc)
	{
		m_Description = desc;
	}

	inline void SetId(quint32 id)
	{
		m_Id = id;
	}

	inline void SetName(const QString& name)
	{
		m_Name = name;
	}

	// The only property this base class picks up is the design name.
	virtual void SetProperties(const AuDesign& design);

	inline void SetState(STATE state)
	{
		m_State = state;
	}

	void SetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count]);

	void SetThumbnail(const QImage& img);

	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	virtual bool SetTransporterPadLocs(const AuScSelection& sel, const AuPointArray& padLocs, AuScParts& partsChanged);

	bool SetTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsChanged);

	bool SetTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis, const AuPoint& gunEmitLoc,
			const AuPoint& gunEmitVec, const AuPoint& gunSightLoc, AuScParts& partsChanged);

	// 'gunnerPos' is a AuScTurret::GUNNER_POSITION.
	bool SetTurretStation(const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc, const AuPoint& gunnerFacing,
			const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane, COMMODITYID module,
			AuScParts& partsChanged);

	virtual bool SetWeaponEmitLoc(const AuScSelection& sel, const AuPoint& emitLoc, AuScParts& partsChanged);

	// Fills 'textures' with the OpenGL texture ids expected by the Sc shaders.
	// Loads the textures if needed.
	// An OpenGL context must be current when this function is called.
	// Palette textures are looked up/loaded to fill the entries in the 'textures' array.
	virtual void ShaderTexturePalette(COMMODITYID hullModule, AuGLEffects& re, AuTexturePalette& textures) const = 0;

	COMMODITYID ShieldModule() const;

	// Returns the system associated with a particular part that was sighted, or SYS_Count if none.
	AuSysId SightedSystem(quint16 partId) const;

	quint16 SpacecraftParkingSpots() const;

	// Matrix for positioning a spacecraft at each parking spot.
	void SpacecraftParkingSpots(QList<AuMatrix4x4>& spots) const;

	quint16 SpaceVehicleParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void SpaceVehicleParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot.
	void SpaceVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	inline STATE State() const
	{
		return (STATE)m_State;
	}

	inline QString StateName() const
	{
		return StateName(State());
	}

	static QString StateName(STATE state);

	void StatusDisplays(AuScReportStatusDisplayHash& tables) const;

	virtual QString StructureName() const = 0;

	// Sets 'patientLoc' to the patient location associated with the medical surgery console 'partId'.
	// Sets 'patientPos' to a PATIENT_POSITION.
	bool SurgeryPatientPos(quint16 partId, AuPoint& patientLoc, quint8& patientPos) const;

	bool SurgeryUnitInRoom(quint16 roomId, AuPoint& patientLoc) const;

	// Returns the location on this design that is targeted by enemy weapons.
	AuPoint TargetLocation() const;

	// Returns SC_TEXTURE_INDEX common to all selected faces.
	// Returns SCTEXI_Count if they differ.
	SC_TEXTURE_INDEX TextureIndex(const AuScSelection& sel) const;

	// Returns the library texture ids assigned to all palette entries.
	// Return values correspond to AuTextureLibItem ids.
	inline void TexturePalette(AuDesignTex texturePalette[SCTEXI_Count])
	{
		for(int tpi=0; tpi<SCTEXI_Count; tpi++)
			texturePalette[tpi] = m_TexturePalette[tpi];
	}

	// Returns the library texture id assigned to a given 'shader' palette entry.
	// Return value corresponds to an AuTextureLibItem id.
	inline quint16 TexturePaletteId(unsigned int texIndex) const
	{
		if(texIndex >= SCTEXI_Count)
			texIndex = 0;
		return m_TexturePalette[texIndex].TexLibId();
	}

	inline const QImage& TexturePaletteImage(unsigned int texIndex) const
	{
		if(texIndex >= SCTEXI_Count)
			texIndex = 0;
		return m_TexturePalette[texIndex].Image();
	}

	inline const QImage& Thumbnail() const
	{
		return m_Thumbnail;
	}

	// Toggles the part object selection in 'sel'.
	void TogglePartObjectSelectionInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& sel) const;

	// Fills 'padLocs' with transporter pad locations of 'partId', which should identify a transporter console or station.
	void TransporterPadLocs(quint16 partId, AuPointArray& padLocs) const;

	quint16 TroopBerths() const;

	// Collects information about turbo lifts. Key is part idf value is default lift stop, which is the current lift stop in the
	// saved design..
	void TurboLifts(AuLiftHash& lifts) const;

	// Collects information about all turbo lifts in the design. Key is turbo lift id. Value is the stops for that lift.
	void TurboLifts(AuScReportLiftHash& lifts) const;

	void TurboLiftStopMatrix(quint16 liftId, const QString& stopName, AuMatrix4x4& stopM) const;

	AuPoint TurretBaseAxis(quint16 turretId) const;

	void TurretBaseMatrix(quint16 turretId, float baseRot, AuMatrix4x4& baseM) const;

	void TurretGunMatrix(quint16 turretId, float baseRot, float gunRot, AuMatrix4x4& gunM) const;

	COMMODITYID TurretModule(quint16 which) const;

	quint16 Turrets() const;

	// Fills 'turrets' with part ids.
	void Turrets(AuScPartSet& turrets) const;

	virtual void Tx(QDataStream& out) const;

	virtual BLUEPRINT_TYPE Type() const = 0;

	// Restores a design from a byte array that was stored using Pack.
	// The returned pointer is allocated on the heap and becomes the property of the caller.
	static AuDesign* Unpack(const QByteArray& ba);

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

	// Gets list of vehicle launcher part ids.
	void VehicleLaunchers(AuScPartSet& launchers) const;

	// Gets list of vehicle landing system part ids.
	void VehicleLandingSystems(AuScPartSet& recs) const;

	// Returns 0=ok, 1=fail invalid 'posId', 2=no stored pos to restore, 3=stored pos is invalid
	int ViewPositionLoad(quint8 posId, AuDesignPosition& pos) const;

	// Returns the list of currently stored view position ids. These are the keys of the m_ViewPos hash.
	QSet<quint8> ViewPositions() const;

	// Returns 0=added, 1=replaced, 2=fail invalid 'posId', 3=fail invalid 'pos'
	int ViewPositionSave(quint8 posId, const AuDesignPosition& pos);

	void VisibleFaces(AuScFaceList& faces) const;

	// Returns visible faces from a particular group of the design. Used by fences to get their Post and Section faces.
	void VisibleFaces(AuScFaceList& faces, const QString& groupName) const;

	quint16 WaterVehicleParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void WaterVehicleParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot.
	void WaterVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	AuPoint WeaponEmitLocation(quint16 partId) const;

	void WriteTextures(QDataStream& out) const;

protected:
	// Returns true if 'partId' has both a hull void and a room void associated with it.
	bool LooksLikeHullDoor(quint16 partId) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuDesign& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDesign& c)
{
	c << in;
	return in;
}

#endif // AuDesign_H
