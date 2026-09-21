//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscparts.h,v 1.274 2022/02/19 18:19:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScParts_H
#define AuScParts_H

#include "audata.h"
#include "augleffects.h"
#include "auid.h"
#include "auscface.h"
#include "auscsilhouette.h"
#include "austation.h"
#include "dbcommodity.h"

using namespace std;

class AuDoorStateHash;
class AuGLEffects;
class AuLandingGearStateHash;
class AuLiftHash;
class AuLiftStopHash;
class AuPartArchitectHash;
class AuQuad;
class AuScDoor;
class AuScPart;
class AuScPartSet;
class AuScReportGameTableHash;
class AuScReportLiftHash;
class AuScReportServicePanelHash;
class AuScReportStatusDisplayHash;
class AuScRoute;
class AuScSelectInfo;
class AuScSelection;
class AuZoneResource;
class DeProcessComponentArray;
class QDataStream;
class QOpenGLWidget;
class QProgressDialog;
class QSqlDatabase;
class QSqlQuery;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPathSeg
{
public:
	quint16								m_Cost;
	bool								m_Ladder;				// true if this is a ladder path segment.
	quint16								m_LiftId;
	AuSegment							m_Seg;
	QString								m_Stop1;				// Turbo lift stop at segment end 1.
	QString								m_Stop2;				// Turbo lift stop at segment end 2.

public:	// Constructors
	inline AuScPathSeg()
	{
		m_Cost = 0;
		m_Ladder = false;
		m_LiftId = 0;
	}

	inline AuScPathSeg(const AuSegment& seg) : m_Seg(seg)
	{
		m_Cost = 0;
		m_Ladder = false;
		m_LiftId = 0;
	}

	inline AuScPathSeg(const AuScPathSeg& c) : m_Seg(c.m_Seg)
	{
		m_Cost = c.m_Cost;
		m_Ladder = c.m_Ladder;
		m_LiftId = c.m_LiftId;
		m_Stop1 = c.m_Stop1;
		m_Stop2 = c.m_Stop2;
	}

	// Path segment from one stop of a lift to another stop of the same lift.
	inline AuScPathSeg(quint16 liftId, const QString& stop1, const AuPoint& loc1, const QString& stop2, const AuPoint& loc2) :
		m_Seg(loc1, loc2)
	{
		m_Cost = 0;
		m_Ladder = false;
		m_LiftId = liftId;
		m_Stop1 = stop1;
		m_Stop2 = stop2;
	}

public:	// Operators
	inline const AuScPathSeg& operator=(const AuScPathSeg& c)
	{
		m_Cost = c.m_Cost;
		m_Ladder = c.m_Ladder;
		m_LiftId = c.m_LiftId;
		m_Seg = c.m_Seg;
		m_Stop1 = c.m_Stop1;
		m_Stop2 = c.m_Stop2;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		quint8 hasWhat = 0;
		if(m_Cost)
			hasWhat |= 0x01;
		if(m_LiftId)
			hasWhat |= 0x02;
		if(!m_Stop1.isEmpty())
			hasWhat |= 0x04;
		if(!m_Stop2.isEmpty())
			hasWhat |= 0x08;
		if(m_Ladder)
			hasWhat |= 0x10;
		out << hasWhat;
		if(hasWhat & 0x01)
			out << m_Cost;
		if(hasWhat & 0x02)
			out << m_LiftId;
		out << m_Seg;
		if(hasWhat & 0x04)
			out << m_Stop1;
		if(hasWhat & 0x08)
			out << m_Stop2;
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 hasWhat;
		in >> hasWhat;
		m_Ladder = hasWhat & 0x10;
		if(hasWhat & 0x01)
			in >> m_Cost;
		else
			m_Cost = 0;
		if(hasWhat & 0x02)
			in >> m_LiftId;
		else
			m_LiftId = 0;
		in >> m_Seg;
		if(hasWhat & 0x04)
			in >> m_Stop1;
		else
			m_Stop1 = QString();
		if(hasWhat & 0x08)
			in >> m_Stop2;
		else
			m_Stop2 = QString();
	}

	inline bool operator==(const AuScPathSeg& c) const
	{
		if(m_Cost != c.m_Cost)
			return false;
		if(m_Ladder != c.m_Ladder)
			return false;
		if(m_LiftId != c.m_LiftId)
			return false;
		if(m_Seg != c.m_Seg)
			return false;
		if(m_Stop1 != c.m_Stop1)
			return false;
		if(m_Stop2 != c.m_Stop2)
			return false;
		return true;
	}

	inline bool operator!=(const AuScPathSeg& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline quint16 Cost() const
	{
		return m_Cost;
	}

	inline bool IsAt(const AuPoint& loc) const
	{
		return Loc1().FuzzyEquals(loc) || Loc2().FuzzyEquals(loc);
	}

	inline bool IsAt(const AuScPathSeg& c) const
	{
		return IsAt(c.Loc1()) || IsAt(c.Loc2());
	}

	inline bool IsLadder() const
	{
		return m_Ladder;
	}

	inline bool IsSameSegment(const AuScPathSeg& c) const
	{
		return m_Seg.IsSegment(c.m_Seg);
	}

	inline quint16 LiftId() const
	{
		return m_LiftId;
	}

	inline const QString& LiftStop1() const
	{
		return m_Stop1;
	}

	inline const QString& LiftStop2() const
	{
		return m_Stop2;
	}

	inline const AuPoint& Loc1() const
	{
		return m_Seg.m_fLoc1;
	}

	inline const AuPoint& Loc2() const
	{
		return m_Seg.m_fLoc2;
	}

	quint64 MemoryUsage() const;

	// Returns a const reference to a statically allocated default constructed path segment.
	static const AuScPathSeg& Null();

	// Returns the other end of the path segment from 'fromLoc'.
	// 'liftStop' is the lift stop that must be selected, if needed at the current stop.
	inline const AuPoint& PathStepLoc(const AuPoint& fromLoc, QString& liftStop) const
	{
		if(m_LiftId)
		{
			if(m_Seg.m_fLoc1.FuzzyEquals(fromLoc))
			{
				liftStop = m_Stop2;
				return m_Seg.m_fLoc2;
			}
			else
			{
				liftStop = m_Stop1;
				return m_Seg.m_fLoc1;
			}
		}
		else
		{
			if(m_Seg.m_fLoc1.FuzzyEquals(fromLoc))
				return m_Seg.m_fLoc2;
			else
				return m_Seg.m_fLoc1;
		}
	}

	inline void Transform(const AuMatrix4x4& t)
	{
		m_Seg.Transform(t);
	}

	inline const AuPoint& Vertex(int index) const
	{
		if(index)
			return m_Seg.m_fLoc2;
		else
			return m_Seg.m_fLoc1;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScPathSeg& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScPathSeg& o)
{
	o << in;
	return in;
}

inline unsigned int qHash(const AuScPathSeg& key)
{
	return key.LiftId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPathSegList : public QList<AuScPathSeg>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	bool AddIfUniqueSegment(const AuScPathSeg& seg);

	// Fuzzy test.
	bool IsAtLoc(const AuPoint& loc) const;

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScPathSegList& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScPathSegList& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Key is a unique identifier for each part.
class AUDATA_DLL AuScParts : public QHash<quint16, AuScPart*>
{
private:
	quint16								m_PartId;				// Starting point for finding the next part id. Not stored.

public:	// Constructors
	AuScParts();
	AuScParts(const AuScParts& c);
	~AuScParts()
	{
		Clear();
	}

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	const AuScParts& operator=(const AuScParts& c);

public:	// Methods
	bool AddFaces(quint16 partId, const AuScFaceList& faces, AuScParts& partsChanged);

	// Returns the id of the new part on success.
	// Returns 0 on failure.
	quint16 AddPart(const AuScPart& s, AuScParts& partsAdded);

	quint16 AddPart(const AuScPart& s);

	// Adds part objects (vertices, edges, faces) of 's' to 'partId'.
	// Returns true if any changes are made.
	bool AddPartObjects(quint16 partId, const AuScPart& s, AuScParts& partsChanged);

	// Returns true if any changes are made.
	bool AddState(quint16 partId, quint8 state, AuScParts& partsChanged);

	bool AddStop(quint16 partId, const QString& stop, AuScParts& partsChanged);

	// Return of 0 indicates failure.
	// Return of 0 indicates failure.
	quint16 AvailablePartId();

	// Returns faces of barriers from a particular group of the design. Used by fences to get their Post and Section barriers.
	void BarrierFaces(AuScFaceList& barrierFaces, const QString& groupName) const;

	// Gets part ids corresponding to bays and FTL drives. These will be the ones with a module associated to them.
	void BaysAndFTLDrives(AuScPartSet& bays, AuScPartSet& ftlDrives);

	quint16 Beds() const;

	AuPoint BerthLocation(quint16 partId) const;

	// Shortens 'toLoc' closer to 'fromLoc' wherever it hits a barrier.
	// 'floorN' is a return value, set to the surface normal of the barrier.
	// Returns true if a barrier is hit.
	bool BlockSegment(const AuPoint& loc, AuPoint& floorLoc, AuPoint& floorN) const;

	// Fills 'edges' with pairs of points.
	// Used to make a representation of a building that is under construction.
	void BuildingConstructionSegments(AuSegmentArray& edges) const;

	// Fills 'edges' with pairs of points.
	// Used to make a cursor of a dragging building and an image of an existing building that obstructs a planned building.
	void BuildingCursorSegments(AuPointArray& edges) const;

	// Fills 'edges' with pairs of points.
	// Used to make a cursor of an existing building site.
	void BuildingCursorSiteSegments(AuPointArray& edges) const;

	// Returns a number from 0 to 1. 0=no influence, 1=full influence.
	float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const;

	float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
			bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
			const AuZoneResource& res) const;

	// Fills 'locs' with my site locations. All points that must be inside a platform build zone are returned.
	void BuildZoneCheckLocs(AuPointArray& locs) const;

	quint16 CaptBerths() const;

	AuPoint CenterOfSelection(const AuScSelection& sel) const;

	// Returns true if 'mimeData' appears to contain at least one part that can be read.
	static bool CheckMIME(bool objectMode, const QMimeData& mimeData);

	void CitizenPostLocs(QHash<quint16, AuPoint>& locs) const;

	void CitizenPosts(AuScPartSet& posts) const;

	// Deletes all part data from the heap.
	void Clear();

	// Sets to null the architect id and name on all parts whose current architect is 'id'. This is intended to preserve ids of
	// architects of parts that were not created by 'id'. See corresponding SetArchitectAfterFileLoad function.
	void ClearArchitectForFileSave(const AuId& id);

	// For buildings.
	void ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestPowerPlant, AuPoint& closestSensor) const;

	// For spacecraft.
	void ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestManeuverDrive, AuPoint& closestPowerPlant,
			AuPoint& closestSensor, AuPoint& closestFTLDrive, AuPoint& closestSiren) const;

	bool ClosestEntrance(AuPoint& entranceLoc, quint16& doorId, quint16& ladderId, const AuPoint& loc, LANDING_ORIENT orient,
			bool useGroundLevelDoors, bool hullDoorsUsable, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Fills 'stations' with part ids that create a command station. i.e. capt chairs.
	void CommandStations(AuScPartSet& stations) const;

	// Fills 'contrails' with information about contrail emitters.
	// Key is location; value is diameter of emitter.
	void ContrailEmitters(QMap<AuPoint, float>& contrails) const;

	// Fills 'stations' with part ids that create a control station.
	// Control stations are operated by crew.
	void ControlStations(AuScPartSet& stations) const;

	quint32 CountFaces() const;

	quint32 CountTriangles() const;

	// Creates a custom MIME data object of the parts in 'partIds'. Coordinates are relative to 'designPlane' by
	// transforming the parts through the inverse of the designPlane.
	QMimeData* CreateMIME(const AuId& architectId, bool objectMode, const AuScSelection& partIds,
			const AuMatrix4x4& designPlane) const;

	quint16 CrewBerths() const;

	void CutFaces(const AuScSelection& sel, const AuPointArray& cutLine, AuScParts& partsChanged);

	bool DeletePart(quint16 partId, AuScParts& partsDeleted);

	// Deletes part objects (vertices, edges, faces) from 'partId'. A clone prior to the changes is added to 'partsChanged' if the
	// part remains valid; it is added to 'partsDeleted' if the part becomes invalid and gets deleted.
	bool DeletePartObjects(quint16 partId, const AuScSelectInfo& selInfo, AuScParts& partsChanged, AuScParts& partsDeleted);

	// Returns true if any changes are made.
	bool DeleteState(quint16 partId, quint8 state, AuScParts& partsChanged);

	bool DeleteStop(quint16 partId, const QString& stop, AuScParts& partsChanged);

	bool DeselectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
			bool findEdges, bool findFaces) const;

	bool DeselectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const;

	// Fills 'stations' with part ids that create a designer station.
	// Designer stations are not operated by crew.
	void DesignerStations(AuScPartSet& stations) const;

	bool DivideFaces(const AuScSelection& sel, quint8 divisions, AuScParts& partsChanged);

	// Collects information about all doors.
	void DoorStates(AuDoorStateHash& doors) const;

	// Returns surface effect common to all selected faces.
	// Returns EFF_Count if they differ.
	SURFACE_EFFECT Effect(const AuScSelection& sel) const;

	// Sets the texture index without changing texture coordinates.
	bool EffectFaces(const AuScSelection& sel, SURFACE_EFFECT effect, AuScParts& partsChanged);

	bool EffectFacesCylindrical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	bool EffectFacesProject(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane,
			AuScParts& partsChanged);

	bool EffectFacesSpherical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	bool EffectIntensityVertices(const AuScSelection& sel, float intensity, AuScParts& partsChanged);

	bool EffectMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged);

	bool EffectRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged);

	bool EffectStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY, AuScParts& partsChanged);

	// 'unwrap' is a AuScFaceList::UNWRAP_METHOD.
	bool EffectUnwrap(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize, quint8 unwrap,
			float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	quint16 EnvironmentVoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Fills 'xrefs' with any external design ids referenced by this design. i.e. custom fences.
	void ExternalReferences(QSet<quint32>& xrefs) const;

	void Faces(AuScFaceList& faces, const AuScSelection& sel) const;

	// Returns the volume of all fence parts, in cubic meters.
	double FenceVolume() const;

	// Returns area of agricultural fields, in square feet. Easily fooled. Does not consider overlap.
	double FieldArea() const;

	bool FillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	bool FillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3, const AuPoint& v4,
			AuScParts& partsChanged);

	bool FillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3, AuScParts& partsChanged);

	// Finalizes parts by removing omitted parts, etc.
	// 'shipHull' is false to apply the hull effect for buildings.
	// 'shipHull' is true to rotate the hull effect for spacecraft hulls.
	void Finalize(bool shipHull, QProgressDialog* progress);

	// 'pixel' is in OpenGL projected viewport coords, i.e. (-1,-1, to 1,1 with 0,0 at center of screen).
	// Returns partId that is found.
	quint16 FindEdgeAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc=0) const;

	quint16 FindEdgeAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds, AuPoint* snapLoc=0) const;

	// 'pixel' is in OpenGL projected viewport coords, i.e. (-1,-1, to 1,1 with 0,0 at center of screen).
	// Returns partId that is found.
	quint16 FindFaceAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc=0) const;

	quint16 FindFaceAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds) const;

	quint16 FindPart(/*AuScPart::PART_TYPE*/quint8 partType) const;

	// Searches from 'fromLoc' to find a ladder segment.
	// Uses IsPathBlocked test to insure fromLoc can get to each ladder.
	bool FindLadderToGrab(const AuPoint& fromLoc, AuMatrix4x4& mLadder, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;
	// In this version, we find a ladder that has at least one end at 'ladderPathLoc'.
	bool FindLadderToGrabAt(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	quint16 FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, bool findVertices, bool findEdges,
			bool findFaces) const;

	// 'pixel' is in OpenGL projected viewport coords, i.e. (-1,-1, to 1,1 with 0,0 at center of screen).
	// Returns partId that is found.
	quint16 FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds, bool findVertices,
			bool findEdges, bool findFaces) const;

	// For each selected part in 'partIds', adds all things that fall within 'pixelRect' to its corresponding selection entry.
	// 'pixelRect' is in OpenGL projected viewport coords, i.e. (-1,-1, to 1,1 with 0,0 at center of screen).
	// 'partIds' will contain vertex indexes that fall within the selection.
	void FindPartObjectsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices,
			bool findEdges, bool findFaces) const;

	// 'pixelRect' is in OpenGL projected viewport coords, i.e. (-1,-1, to 1,1 with 0,0 at center of screen).
	// 'partIds' will contain empty vertex indexes.
	void FindPartsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices, bool findEdges,
			bool findFaces) const;

	// 'pixel' is in OpenGL projected viewport coords, i.e. (-1,-1, to 1,1 with 0,0 at center of screen).
	// Returns partId that is found.
	quint16 FindVertexAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc=0) const;

	quint16 FindVertexAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds, AuPoint* snapLoc=0) const;

	// Repairs part associations after a copy/paste operation. The pasted parts are assigned nu part ids. Associations within those
	// parts are repaired to point at the corresponding nu parts.
	// 'nuPartIds' key=old part id, value=copied part id
	void FixPartAssociations(const QHash<quint16, quint16>& nuPartIds);

	// Flattens all vertices.
	void FlattenVertices();

	// Flattens selected vertices of selected faces.
	bool FlattenVertices(const AuScSelection& sel, AuScParts& partsChanged);

	bool FractureFaces(const AuScSelection& sel, AuScParts& partsChanged);

	bool FuseFaces(const AuScSelection& sel, AuScParts& partsChanged);

	void GameTables(AuScReportGameTableHash& tables) const;

	bool GlowVertices(const AuScSelection& sel, const QColor& glow, AuScParts& partsChanged);

	double GroundHeightAboveCG(GROUND_ORIENT gori) const;

	double GroundHeightAboveCG(const AuLandingGearStateHash& landingGearStates, LANDING_ORIENT lori) const;

	quint16 GroundVehicleParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void GroundVehicleParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot in a room.
	void GroundVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	// Fills 'groupNames' with the unique list of group names of all parts. Will include a blank entry if any part has no group.
	void GroupNames(QStringList& groupNames) const;

	void GuardPostLocs(QHash<quint16, AuPoint>& locs) const;

	void GuardPosts(AuScPartSet& posts) const;

	bool HasBar() const;

	bool HasEnvironmentVoid() const;

	bool HasHelm() const;

	bool HasPantry() const;

	// Returns true if any parts were sealed by an architect EXCEPT 'architectId'.
	bool HasSealedParts(const AuId& archId) const;

	// Returns true if any selected parts were sealed by an architect EXCEPT 'architectId'.
	bool HasSealedParts(const AuId& archId, const AuScSelection& sel) const;

	AuPoint HelmLocation() const;

	quint16 HighestPartId() const;

	// Returns the size that eventually gets passed to ApplyHullFTLEffect. All faces in the model should use the same factor.
	double HullEffectSize() const;

	void HullExtents(AuPoint& min, AuPoint& max) const;
	
	void HullExtents(AuPoint& min, AuPoint& max, const AuLandingGearStateHash& landingGearStates) const;

	// Returns the volume of all hull parts, in cubic meters.
	// This is a gruesome function. Cache the result, please.
	quint32 HullVolume() const;

	quint32 HullVolume(QProgressDialog& progress) const;

	// Fills 'partIds' with the ids of all parts.
	void Ids(AuScSelection& partIds) const;

	// Fills 'partIds' with the ids of all parts that are not omitted.
	void IdsNotOmitted(AuScSelection& partIds) const;

	void Import3DS(const QString& fileName, double scale);

	// Returns true if the 2D location of 'loc' falls within any of my site meshes.
	bool InBuildingSite(const AuPoint& loc) const;

	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	bool InBuildingSite(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const;

	// Returns part id of build zone, if the 3D location of 'loc' falls ABOVE any of my build zone meshes.
	quint16 InBuildZone(const AuPoint& loc) const;

	// Returns true if 'loc' is inside the obstruction model of the hull, considering exterior voids.
	// Returns true if 'loc' is inside a room void or a hull or landing gear.
	// Returns false if 'loc' is inside a hull void or outside the hull obstruction model completely.
	// This is a rough test for being inside the hull or landing gear.
	bool IsInsideHull(const AuPoint& loc) const;

	bool IsInsideHull(const AuPoint& loc, const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates,
			const AuLiftHash& liftStops) const;

	bool IsInsideHullExtents(const AuPoint& loc) const;

	bool IsInsideHullExtents(const AuPoint& loc, const AuLandingGearStateHash& landingGearStates) const;

	// Returns true if 'loc' is inside my obstruction model. Does not consider voids.
	bool IsInsideObstructionModel(const AuPoint& loc) const;

	bool IsInsideObstructionModel(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	// Returns true if 'fromLoc' is on a ladder path to 'ladderPathLoc', which is a route location known to be the end of a ladder
	// segment.
	bool IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	// Returns true if 'roomId' is a room void associated with the open state of 'doorId'.
	bool IsOpenDoorVoid(quint16 roomId, quint16 doorId) const;

	// Returns true if a surface is hit when moving from 'fromLoc' to 'toLoc'.
	// If hit, 'hitNormal' is set to the vector pointing out from the surface that is hit, if any.
	// Returns false if no surface is hit and 'hitNormal' is unchanged.
	bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	// 'doorStates' is the current state of doors, not the states of the doors in the design.
	// 'landingGearStates' is the current state of landing gear, not the states of the landing gear in the design.
	bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Returns true if 'loc' is outside the hull or contained inside any void, ignoring associated part states.
	// Returns false if 'loc' is never unobstructed.
	bool IsPathEverUnobstructed(const AuPoint& loc) const;

	// 'archId' is used to prevent cutting sealed parts that were made by another architect.
	bool JigCutParts(const AuId& archId, const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged,
			AuScParts& partsDeleted);

	bool JoinParts(quint16 targetPart, const AuScSelection& sel, AuScParts& partsChanged, AuScParts& partsDeleted);

	// Fills 'landingGear' with part ids.
	void LandingGear(AuScPartSet& landingGear) const;

	void LandingSystemPosition(quint16 landerId, AuMatrix4x4& landerPosM) const;

	quint16 LauncherForDoor(quint16 doorId, const AuScDoor& door) const;

	void LaunchPosition(quint16 launcherId, AuMatrix4x4& launchPadM, AuPoint& launchExit) const;

	// 'streamline' is an AuScDesign::STREAMLINE enum.
	// 'armor' is an armor factor in the range of 0 to 10.
	// Buildings pass CID_Lumber as their hull material. Spacecraft are not built of lumber.
	void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	// Gets a list of the environment voids in the parts list.
	void ListEnvironmentVoids(AuScParts& envVoids) const;

	void ListRoomNames(QStringList& roomNames) const;

	void LivestockPostLocs(QHash<quint16, AuPoint>& locs) const;

	void LivestockPosts(AuScPartSet& posts) const;

	void Loop(const AuScSelection& sel, const AuPoint& loc, AuPointArray& loop) const;

	// Converts each selected part into a decal if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeDecal(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a door if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeDoor(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a head light if possible. Converted parts are deleted, moved to 'partsDeleted'.
	//bool MakeHeadlightLens(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a hull if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeHull(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a landing gear if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeLandingGear(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a mesh if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeMesh(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a nav light if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeNavLightLens(const AuScSelection& sel, const QColor& color, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a room if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeRoom(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a rotating beacon if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeRotatingBeaconLens(const AuScSelection& sel, const QColor& color, const AuPoint& rotationAxis, AuScParts& partsAdded,
			AuScParts& partsDeleted);

	// Converts each selected part into a head light if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeLightLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a strobe if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeStrobeLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts one selected room and door jig combination into a lift. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeTurboLift(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted);

	// Converts each selected part into a turret if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeTurret(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsAdded,
			AuScParts& partsDeleted);

	// Converts each selected part into a window if possible. Converted parts are deleted, moved to 'partsDeleted'.
	bool MakeWindow(const AuScSelection& sel, float opacity, AuScParts& partsAdded, AuScParts& partsDeleted);

	quint64 MemoryUsage() const;

	// Returns the extents of every vertex of the model, including all states of parts that have them.
	void ModelExtents(AuPoint& min, AuPoint& max) const;
	
	// Returns the module associated with a part, if any. Could return CID_Invalid.
	COMMODITYID Module(quint16 partId) const;

	// Switch locations can be AuPoint::Invalid to restore them to the center of the closed door geometry.
	// Returns true if any changes are made.
	// 'partsChanged' is filled with clones for caller creation of undo data.
	bool MoveDoorSwitches(const AuScSelection& sel, const AuPoint& switch1, const AuPoint& switch2, AuScParts& partsChanged);

	quint16 OfficerBerths() const;

	AuPoint OperatorLocation(quint16 partId) const;

	// Alpha value of 'paint' is the blend factor.
	bool PaintFaces(const AuScSelection& sel, const QColor& paint, AuScParts& partsChanged);

	bool PaintFaces(const AuScSelection& sel, AuScFace::COLOR_THEME theme, quint8 opacity, AuScParts& partsChanged);

	// Retrieves a list of all unique paint colors in use by faces.
	void PaintInUse(QList<QColor>& paintInUse) const;

	// List all part architects that are different from 'archId'.
	void PartArchitects(const AuId& archId, AuPartArchitectHash& partArchs) const;

	// Gets information about all path segments.
	void PathSegments(AuScPathSegList& segs) const;

	quint16 PassengerBerths() const;

	bool PolishFaces(const AuScSelection& sel, float shine, float reflectivity, AuScParts& partsChanged);

	void PunchFaces(const AuScSelection& sel, const AuPointArray& cutBnd, AuScParts& partsChanged);

	// Returns type of gravity, if 'loc' is affected by my gravity.
	// Returns G_Count if 'loc' is not affected by my gravity.
	// Only valid in the designer due to lift stop matrices, door states, and landing gear states.
	GRAVITY QueryGravity(const AuPoint& loc, AuPoint& floorLoc, AuPoint& gravityN, AuPoint& terN) const;

	quint16 RandomBuildZoneLocation(AuPoint& zoneLoc) const;

	bool RandomBuildZoneLocation(quint16 zoneId, AuPoint& zoneLoc) const;

	bool RandomRoomLocation(quint16 roomId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Locates a random location inside a room void named 'roomName'. Returns false if none is found.
	// 'roomName' is compared against the format returned by ListRoomNames, which is 'room name N' where N is the room id.
	// It is also compared case insensitively against leading fragments, so 'bridge' would match 'Bridge 12'.
	bool RandomRoomLocation(const QString& roomName, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	quint16 RandomEnvironmentVoidLocation(AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Finds a location in a room void or an environment void.
	quint16 RandomRoomVoidLocation(AuPoint& voidLoc, quint8 personnelAccess, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Returns a location on a site mesh that is not blocked by any obstructions.
	quint16 RandomSiteLocation(AuPoint& siteLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	bool RandomSiteLocation(quint16 siteId, AuPoint& siteLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	// Returns parts that are read from a custom MIME data object 'mimeData', as returned by CreateMIME.
	static void ReadMIME(const AuId& architectId, const QString& architectName, bool objectMode, const QMimeData& mimeData,
			AuScParts& parts);

	// Resets/rebuilds all obstruction data, i.e. silhouettes.
	// Obstruction data is not stored and it is too much work to maintain with every modification in the designer.
	// This must be called before IsPathBlocked can be used after creating, modifying or loading a parts list.
	// This MUST be called only on the scene client.
	void RefreshObstructionsCache();
	void RefreshObstructionsCache(QProgressDialog& progress);

	// Resets/rebuilds obstruction data where needed, i.e. silhouettes that report !IsValid.
	// This MUST be called only on the scene client.
	void RefreshObstructionsCacheForFinalize(QProgressDialog& progress);

	// Resets/rebuilds obstruction data where needed, i.e. silhouettes that report !IsValid.
	// This MUST be called only on the scene client.
	bool RefreshObstructionsCacheIfNeeded();

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

	bool RenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName, AuScParts& partsChanged);

	// Returns the ReportDesc of a part.
	QString ReportDesc(quint16 partId) const;

	// Reports an html table of parts, similar to the parts list table on the parts user interface.
	// 'archId' is the primary architect. Parts designed by other architects will be identified as such.
	QString ReportParts(const AuId& archId) const;

	void ReverseFaces();

	bool ReverseFaces(const AuScSelection& sel, AuScParts& partsChanged);

	bool ReverseVertexNormals();

	quint16 RocketParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void RocketParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot in a room.
	void RocketParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	// Returns the id of a room with 'roomName'.
	// CAVEAT: More than one room can share the same name.
	quint16 RoomId(const QString& roomName) const;

	// Returns true if 'roomName' is normally sunlit. Does not determine current lighting conditions, just whether sunlight normally
	// illuminates the interior of a room.
	// CAVEAT: More than one room can have the same name.
	bool RoomIsSunlit(const QString& roomName) const;

	// Returns initial interior lighting conditions of 'roomName'.
	// CAVEAT: More than one room can have the same name.
	INTERIOR_LIGHT RoomLight(const QString& roomName) const;

	// Returns initial interior lighting conditions of 'roomId'.
	INTERIOR_LIGHT RoomLight(quint16 roomId) const;

	// Returns the part id of the room void that contains 'loc'.
	// Beware this could be ambiguous because room voids can overlap.
	quint16 RoomVoidContainingLoc(const AuPoint& loc) const;

	quint16 RoomVoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	quint16 RoomVoidContainingLoc(const AuPoint& loc, double elevate) const;

	quint16 RoomVoidContainingLoc(const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	AuPoint RoomVoidGravityUp(quint16 roomId, const AuLiftHash& liftStops) const;

	// Assumes loc has already been checked to be inside the room void 'roomId'.
	// 'gravityN' will not be changed unless 'loc' is in a room with artificial g.
	// 'getRoomUp' is the caller wants 'gravityN' set to the void's gravity up vector, regardless of its gravity settings.
	// regardless of external gravity conditions or void gravity settings.
	double RoomVoidHeightAboveFloor(quint16 roomId, const AuPoint& loc, bool getRoomUp, AuPoint& gravityN,
			const AuLiftHash& liftStops) const;

	HOLD_ACCESS RoomVoidHoldAccess(quint16 roomId) const;

	// Returns true if 'voidId' is normally sunlit. Does not determine current lighting conditions, just whether sunlight normally
	// illuminates the interior of a room.
	// CAVEAT: More than one room can have the same name. Finds the void and uses its name to call RoomIsSunlit.
	bool RoomVoidIsSunlit(quint16 voidId) const;

	// Fills 'rooms' with names of room voids. Multiple voids may share the same name. They are treated as a single void for
	// air pressure and lighting purposes.
	void RoomVoidNames(QStringList& rooms) const;

	// Determines a route from 'fromLoc' to 'toLoc', either or both of which might not be aboard.
	// 'canUnlockDoors' is true if the locked state of doors can be ignored.
	// Returns true if a route is found.
	bool Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, LANDING_ORIENT orient, bool canUnlockDoors,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Adds to 'sel' the faces and vertices that are contiguous and in the same plane as the face(s) already selected, within
	// 'angleTol'.
	bool SelectContiguous(double angleTol, AuScSelection& sel) const;

	// Returns the description of 'partId' when it is selected with the mouse.
	QString SelectedDesc(quint16 partId) const;

	bool SelectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
			bool findEdges, bool findFaces) const;

	bool SelectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const;

	COMMODITYID SensorModule() const;

	void ServicePanels(AuScReportServicePanelHash& panels) const;

	// Sets the architect id and name on all parts whose current architect is null. This is intended to preserve ids of
	// architects of parts that were not created by 'id'. See corresponding ClearArchitectForFileSave function.
	void SetArchitectAfterFileLoad(const AuId& id, const QString& name);

	bool SetDoorProperties(const AuScSelection& sel, Qt::CheckState isAutomatic, Qt::CheckState initLocked,
			Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle, AuScParts& partsChanged);

	bool SetFaceOpacity(const AuScSelection& sel, float opacity, AuScParts& partsChanged);

	bool SetFaceSelfIllumination(const AuScSelection& sel, float selfIllum, AuScParts& partsChanged);

	bool SetGroupName(quint16 partId, const QString& groupName, AuScParts& partsChanged);

	bool SetHidden(quint16 partId, bool hide, AuScParts& partsChanged);

	bool SetLight(quint16 partId, const QColor& color, float kL, float kQ, float cosOuterCone, float cosInnerCone,
			AuScParts& partsChanged);

	bool SetLightBulbProperties(const AuScSelection& sel, const QColor& color, float kL, float kQ, float cosOuterCone,
			float cosInnerCone, AuScParts& partsChanged);

	bool SetLightLensProperties(const AuScSelection& sel, const QColor& color, AuScParts& partsChanged);

	bool SetLocked(quint16 partId, bool lock, AuScParts& partsChanged);

	bool SetOmitted(quint16 partId, bool omit, AuScParts& partsChanged);

	bool SetOpacity(quint16 partId, float opacity, AuScParts& partsChanged);

	bool SetRoomProperties(const AuScSelection& sel, bool isSunlit, INTERIOR_LIGHT initLighting, AuScParts& partsChanged);

	bool SetSealed(quint16 partId, bool seal, AuScParts& partsChanged);

	bool SetSiteProperties(const AuScSelection& sel, quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs,
			quint8 terBlend, quint8 trees, AuScParts& partsChanged);

	bool SetState(quint16 partId, quint8 state, AuScParts& partsChanged);

	bool SetStop(quint16 partId, const QString& stop, AuScParts& partsChanged);

	bool SetStopName(quint16 partId, const QString& name, AuScParts& partsChanged);

	bool SetTransporterPadLocs(const AuScSelection& sel, const AuPointArray& padLocs, AuScParts& partsChanged);

	bool SetTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsChanged);

	bool SetTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis, const AuPoint& gunEmitLoc,
			const AuPoint& gunEmitVec, const AuPoint& gunSightLoc, AuScParts& partsChanged);

	bool SetTurretStation(const AuScSelection& sel, /*AuScTurret::GUNNER_POSITION*/quint8 gunnerPos, const AuPoint& gunnerLoc,
			const AuPoint& gunnerFacing, const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane,
			COMMODITYID module, AuScParts& partsChanged);

	bool SetVoidProperties(const AuScSelection& sel, GRAVITY gravityType, const AuPoint& gravityUp, HOLD_ACCESS holdAccess,
			quint8 personnelAccess, AuScParts& partsChanged);

	bool SetWeaponEmitLoc(const AuScSelection& sel, const AuPoint& emitLoc, AuScParts& partsChanged);

	COMMODITYID ShieldModule() const;

	// Check sight segment from 'eyeLoc' to 'sightLoc'.
	// Returns true if the sight segment hits any parts. 'sightPart' is the specific part that is hit.
	// If I am hit, 'sightLoc' is adjusted closer to 'eyeLoc', to the point that was hit, and 'sightSurfaceNormal' is set.
	bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	// Check sight segment from 'eyeLoc' to 'sightLoc'.
	// Returns true if the sight segment hits any build zone parts. 'sightPart' is the specific build zone part that is hit.
	// If I am hit, 'sightLoc' is adjusted closer to 'eyeLoc', to the point that was hit.
	bool SightBuildZone(const AuPoint& eyeLoc, AuPoint& sightLoc, quint16& sightPart) const;

	// Returns the system associated with a particular part that was sighted, or SYS_Count if none.
	// 'which' is set to the specific subsystem part id.
	AuSysId SightedSystem(quint16 partId) const;

	void SiteExtents(AuPoint& min, AuPoint& max) const;
	
	// Smooths all vertices.
	bool SmoothVertices();

	// Smooths selected vertices of selected faces.
	bool SmoothVertices(const AuScSelection& sel, AuScParts& partsChanged);

	quint16 SpacecraftParkingSpots() const;

	// Matrix for positioning a spacecraft at each parking spot.
	void SpacecraftParkingSpots(QList<AuMatrix4x4>& spots) const;

	quint16 SpaceVehicleParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void SpaceVehicleParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot in a room.
	void SpaceVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	bool SplitParts(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged);

	void StatusDisplays(AuScReportStatusDisplayHash& displays) const;

	// Returns the lift stop at lift door 'partId'.
	QString StopName(quint16 partId) const;

	void Stops(quint16 partId, QStringList& stops) const;

	// Sets 'patientLoc' to the patient location associated with the medical surgery console 'partId'.
	// Sets 'patientPos' to a PATIENT_POSITION.
	bool SurgeryPatientPos(quint16 partId, AuPoint& patientLoc, quint8& patientPos) const;

	bool SurgeryUnitInRoom(quint16 roomId, AuPoint& patientLoc) const;

	bool SurgeryUnitInRoom(quint16 roomId, AuPoint& patientLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Returns the location that is targeted by enemy weapons.
	AuPoint TargetLoc() const;

	// Sets the texture index without changing texture coordinates.
	bool TextureFaces(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, AuScParts& partsChanged);

	bool TextureFacesCylindrical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator, double texSize,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	bool TextureFacesProject(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize, const AuMatrix4x4& gridPlane,
			AuScParts& partsChanged);

	bool TextureFacesSpherical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	// Returns SC_TEXTURE_INDEX common to all selected faces.
	// Returns SCTEXI_Count if they differ.
	SC_TEXTURE_INDEX TextureIndex(const AuScSelection& sel) const;

	bool TextureMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged);

	bool TextureRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged);

	bool TextureStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY, AuScParts& partsChanged);

	// 'unwrap' is a AuScFaceList::UNWRAP_METHOD.
	bool TextureUnwrap(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize, quint8 unwrap,
			float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane, AuScParts& partsChanged);

	void Transform(const AuMatrix4x4& t);

	quint16 TroopBerths() const;

	// Returns the turbo lift that has a stop at 'doorId'. 'door' is the door.
	quint16 TurboLiftForDoor(quint16 doorId, const AuScDoor& door) const;

	// Collects information about turbo lifts. Key is part idf value is default lift stop, which is the current lift stop in the
	// saved design..
	void TurboLifts(AuLiftHash& lifts) const;

	// Collects information about all turbo lifts in the design. Key is turbo lift id. Value is the stops for that lift.
	void TurboLifts(AuScReportLiftHash& lifts) const;

	// Returns the current stop name of turbo lift 'partId'.
	QString TurboLiftAtStop(quint16 partId) const;

	void TurboLiftStopMatrix(quint16 liftId, const QString& stopName, AuMatrix4x4& stopM) const;

	void TurboLiftStops(quint16 liftId, AuLiftStopHash& liftStops) const;

	quint16 Turrets() const;

	// Fills 'turrets' with part ids.
	void Turrets(AuScPartSet& turrets) const;

	// Adds parts and their ids.
	void UndoRedoAdd(const AuScParts& parts);

	// Deletes parts by id.
	void UndoRedoDelete(const QList<quint16>& partIds);

	// Swaps parts 1:1 by id.
	void UndoRedoSwap(AuScParts& swapParts);

	bool UpdatePart(quint16 partId, const AuScPart& s, AuScParts& partsUpdated);

	// Gets list of vehicle recovery system part ids.
	void VehicleLandingSystems(AuScPartSet& recs) const;

	// Gets list of vehicle launcher part ids.
	void VehicleLaunchers(AuScPartSet& launchers) const;

	void VisibleFaces(AuScFaceList& faces) const;

	void VisibleFaces(AuScFaceList& faces, const QString& groupName) const;

	// Returns any kind of void. Does not have to be a room void.
	quint16 VoidContainingLoc(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, quint16 exceptVoidId) const;

	// Returns only voids that are not associated with other parts. Does not have to be a room void.
	quint16 VoidContainingLocConstant(const AuPoint& loc, quint16 exceptVoidId) const;

	// Returns only voids that are associated with other parts. Does not have to be a room void.
	quint16 VoidContainingLocVariable(const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, quint16 exceptVoidId) const;

	quint16 WaterVehicleParkingSpots() const;

	// Matrix for positioning a vehicle at each parking spot.
	void WaterVehicleParkingSpots(QList<AuMatrix4x4>& spots) const;

	// Matrix for positioning a vehicle at each parking spot in a room.
	void WaterVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const;

	AuPoint WeaponEmitLoc(quint16 partId) const;

protected:
	bool ClosestHullExitNode(const AuPoint& toLoc, LANDING_ORIENT orient, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& closestLoc) const;

	bool ClosestPathNode(const AuPoint& toLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& closestLoc) const;

	// Follows an exterior ladder back in Y, for LORI_Tail oriented designs.
	quint16 FollowExteriorLadderBack(AuPoint& ladderLoc, double targetY,
			const AuLandingGearStateHash& landingGearStates, bool locAtTargetY) const;

	// Follows an exterior ladder back in Z, for LORI_Belly oriented designs.
	quint16 FollowExteriorLadderDown(AuPoint& ladderLoc, double targetZ,
			const AuLandingGearStateHash& landingGearStates, bool locAtTargetZ) const;

	bool RouteAround(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc,
			const AuLandingGearStateHash& landingGearStates) const;

	bool RouteToNode(AuScRoute& route, const AuPoint& toNode, bool canUnlockDoors,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScParts& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScParts& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPart
{
public:
	// One enum entry for every AuScPart class that can be instantiated.
	// USED FOR IDENTITY IN SERIAL DATA STREAMS. DO NOT CHANGE THE VALUE OF THESE ENUMS.
	enum PART_TYPE
	{
		Part_Invalid,						// 0 value is invalid.
		Part_Mesh,
		Part_DoorJig,
		Part_WindowJig,
		Part_Hull,
		Part_Room,
		Part_Door,
		Part_Window,
		Part_LandingGear,
		Part_Lift,
		Part_StatusAlert,
		Part_StatusLifeSupport,
		Part_StatusMission,
		Part_PanelFTLDrive,
		Part_PanelLifeSupport,
		Part_PanelManeuverDrive,
		Part_PanelPowerPlant,
		Part_PanelSensor,
		Part_PanelShield,
		Part_PanelWeapon,
		Part_StationCommand,
		Part_StationEngineer,
		Part_StationFireControl,
		Part_StationHelm,
		Part_StationMedic,
		Part_StationNavigator,
		Part_StationPowerRelay,
		Part_StationSensor,
		Part_StationShield,
		Part_StationTransporter,
		Part_ConsoleCommand,
		Part_ConsoleEngineer,
		Part_ConsoleFireControl,
		Part_ConsoleHelm,
		Part_ConsoleMedic,
		Part_ConsoleNavigator,
		Part_ConsolePowerRelay,
		Part_ConsoleSensor,
		Part_ConsoleShield,
		Part_ConsoleTransporter,
		Part_EnvironmentVoid,
		Part_Camera,
		Part_TroopPost,
		Part_GroundVehicleParkingSpot,
		Part_SpaceVehicleParkingSpot,
		Part_SpaceVehicleLauncher,
		Part_BerthCapt,
		Part_BerthCrew,
		Part_BerthOfficer,
		Part_BerthPassenger,
		Part_BerthTroop,
		Part_GameTable,
		Part_ContrailEmitter,
		Part_HeadlightLens_Deprecated,
		Part_NavLightLens,
		Part_RotatingBeaconLens,
		Part_StrobeLens,
		Part_Turret,
		Part_PodBay,
		Part_Sketch,
		Part_LaunchTubeJig,
		Part_CuttingJig,
		Part_SpaceVehicleLandingSystem,
		Part_LightBulb,
		Part_StatusWorldMap,
		Part_Decal,
		Part_EmpireFlag,
		Part_EmpireName,
		Part_ShipName,
		Part_CaptName,
		Part_DecalJig,
		Part_HullVoid,
		Part_RoomVoid,
		Part_PathWalk,
		Part_PathLadder,
		Part_PanelCapacitor,
		Part_WaterVehicleParkingSpot,
		Part_Barrier,
		Part_RocketParkingSpot,
		Part_Site,
		Part_Tree,
		Part_Shrub,
		Part_Fence,
		Part_CitizenPost,
		Part_LivestockPost,
		Part_SpacecraftParkingSpot,
		Part_StationDesigner,
		Part_LightLens,
		Part_BerthName,
		Part_BuildZone,
		PART_Count							// LAST ITEM MUST BE <= 255
	};

	enum PART_SOUND
	{
		SOUND_Silent,
		SOUND_FTLDrive,
		SOUND_ManeuverDrive,
		SOUND_PowerPlant,
		SOUND_Sensor,
		SOUND_Siren,
	};

	enum PATIENT_POSITION
	{
		Patient_Sit,					// Patient can turn in Z, and look up/down, e.g. swivel chair or old console/screen combo.
		Patient_SitStationary,			// Patient cannot turn body but can turn head and look up/down.
		Patient_Stand,					// Patient stands, can turn in Z and look up/down..
		Patient_StandStationary,		// Patient stands, cannot turn body but can turn head and look up/down..
		Patient_Prone,					// Patient lies prone.
	};

private:
	enum BITS
	{
		BIT_Hide						= 0x01,					// When on, part is not visible or snappable in designer.
		BIT_Lock						= 0x02,					// When on, part cannot be selected in designer.
		BIT_Omit						= 0x04,					// When on, part is not included in final design.
		BIT_Seal						= 0x08,					// When on, only architect can edit in object mode.
	};

	AuId								m_ArchId;				// Avatar id who created this part.
	QString								m_ArchName;				// Avatar name who created this part.
	quint8								m_Bits;					// BITS or'd together. Top nibble was serialization version.
	QString								m_Group;				// Lighting group. Subassembly grouping for fences.
	quint8								m_StreamVersion;		// Version read from data stream, otherwise 0.

public:	// Constructors
	AuScPart();
	virtual ~AuScPart();
	AuScPart(const AuScPart& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// CAVEAT: Does not compare silhouettes.
	virtual bool operator==(const AuScPart& c) const;

	inline bool operator!=(const AuScPart& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	// Returns a map of states (key) and state names (value) that can be added.
	virtual QMap<quint8, QString> AddableStates() const
	{
		return QMap<quint8, QString>();
	}

	virtual void AddFaces(const AuScFaceList& /*faces*/)
	{
	}

	// State information is used by doors, landing gear, elevators, etc.
	// Returns true if any changes are made.
	virtual bool AddState(quint8 state)
	{
		Q_UNUSED(state);
		return false;
	}

	virtual bool AddStop(const QString& stop)
	{
		Q_UNUSED(stop);
		return false;
	}

	inline const AuId& ArchId() const
	{
		return m_ArchId;
	}

	inline const QString& ArchName() const
	{
		return m_ArchName;
	}

	virtual quint16 AssociatedPartId() const
	{
		return 0;
	}

	virtual quint8 AssociatedPartState() const
	{
		return 0;
	}

	// Returns the current stop of a turbo lift.
	virtual QString AtStop() const
	{
		return "";
	}

	virtual const AuPoint& BaseAxis() const
	{
		return AuPoint::Null();
	}

	// Returns the number added to the design analysis.
	inline quint16 Beds() const
	{
		return CaptBerths() + CrewBerths() + OfficerBerths() + PassengerBerths() + TroopBerths();
	}

	virtual AuPoint BerthLocation() const
	{
		return AuPoint(0,0);
	}

	// Shortens 'toLoc' closer to 'fromLoc' wherever it hits a barrier.
	// 'floorN' is a return value, set to the surface normal of the barrier.
	// Returns true if a barrier is hit.
	virtual bool BlockSegment(const AuPoint& fromLoc, AuPoint& toLoc, AuPoint& floorN) const
	{
		Q_UNUSED(fromLoc);
		Q_UNUSED(toLoc);
		Q_UNUSED(floorN);
		return false;
	}

	virtual void BuildingConstructionSegments(AuSegmentArray& edges) const
	{
		Q_UNUSED(edges);
	}

	virtual void BuildingCursorSegments(AuPointArray& edges) const
	{
		Q_UNUSED(edges);
	}

	virtual void BuildingCursorSiteSegments(AuPointArray& edges) const
	{
		Q_UNUSED(edges);
	}

	// Returns a number from 0 to 1. 0=no influence, 1=full influence.
	// 'siteLoc' is the location projected onto the surface of the site mesh.
	// When the return value is 1, loc moves to siteLoc.
	// When the return value is .5, loc moves halfway to siteLoc.
	// When the return value is 0, loc is not affected.
	virtual float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(siteLoc);
		Q_UNUSED(terBlend);
		return 0;
	}

	// Sets 'hide' bools to indicate affect on natural terrain and plants at 'loc'.
	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	// 'farmShrub' or 'orchardTree' will be set accordingly if the location falls within the area of a farm or orchard plant.
	// 'terBlend' 0=linear, 1=cosine.
	virtual float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
			bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
			const AuZoneResource& res) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(siteLoc);
		Q_UNUSED(hideRocks);
		Q_UNUSED(hideGrass);
		Q_UNUSED(hideShrubs);
		Q_UNUSED(hideTrees);
		Q_UNUSED(pavementType);
		Q_UNUSED(farmShrub);
		Q_UNUSED(orchardTree);
		Q_UNUSED(terBlend);
		Q_UNUSED(res);
		return 0;
	}

	virtual void BuildZoneCheckLocs(AuPointArray& locs) const
	{
		Q_UNUSED(locs);
	}

	// Returns true if any states can be added. Must also override AddState.
	virtual bool CanAddState() const
	{
		return false;
	}

	// Returns true if any stops can be added.
	virtual bool CanAddStop() const
	{
		return false;
	}

	// Returns true if SetLight is implemented.
	virtual bool CanChangeLight() const
	{
		return false;
	}

	// Returns true if SetOpacity is implemented.
	virtual bool CanChangeOpacity() const
	{
		return false;
	}

	// Returns true if SetState is implemented.
	virtual bool CanChangeState() const
	{
		return false;
	}

	// Returns true if SetStop is implemented.
	virtual bool CanChangeStop() const
	{
		return false;
	}

	// Returns true if SetStopName is implemented.
	virtual bool CanChangeStopName() const
	{
		return false;
	}

	// Returns true if any states can be deleted.
	virtual bool CanDeleteState() const
	{
		return false;
	}

	virtual bool CanDeleteStop() const
	{
		return false;
	}

	// Returns true if this part can be fractured into triangles.
	virtual bool CanFracture() const = 0;

	// Returns true if this part can be fused with triangles.
	virtual bool CanFuse() const = 0;

	// Returns true if this part's faces can be joined to 'joinTarget' in an editing operation.
	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	// Used by functions that convert selected parts into some other kind of part, using its mesh.
	virtual bool CanMakeIntoOtherPart() const = 0;

	// Returns the number added to the design analysis.
	virtual quint16 CaptBerths() const
	{
		return 0;
	}

	// Returns the geometric center of this part.
	virtual AuPoint Center() const = 0;

	// Return false if this part is a void that has a non-planar face.
	virtual bool CheckVoidFacesPlanar() const
	{
		return true;
	}

	// Sets to null the architect id and name if current architect is 'id'. This is used to preserve ids of
	// architects of parts that were not created by 'id'. See corresponding SetArchitectAfterFileLoad function.
	inline void ClearArchitectForFileSave(const AuId& id)
	{
		if(ArchId() == id)
			SetArchitect(AuId(0), "");
	}

	virtual AuScPart* Clone() const = 0;

	// Creates a clone of this part containing only the selected objects (vertices, edges, faces).
	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const = 0;

	// Returns the station type and operator positioning for this station's seat that is closest to 'opLoc'.
	// 'gravityUp' is the up direction in the void space containing this station.
	virtual AuStation ClosestStation(const AuPoint& opLoc, const AuPoint& gravityUp) const
	{
		Q_UNUSED(opLoc);
		return OperatorStation(gravityUp);
	}

	// 'doorState' is a AuScDoor::STATES.
	bool CompareAssociatedDoorState(quint8 doorState) const;

	// 'landingGearState' is a AuScLandingGear::STATES.
	bool CompareAssociatedLandingGearState(quint8 landingGearState) const;

	// Calls the Faces function. Override if the part has more, such as a state part.
	virtual quint32 CountFaces() const;

	// Calls the Faces function. Override if the part has more, such as a state part.
	virtual quint32 CountTriangles() const;

	// Returns a new object if 'type' is valid.
	static AuScPart* Create(PART_TYPE type);

	// Returns the number added to the design analysis.
	virtual quint16 CrewBerths() const
	{
		return 0;
	}

	virtual bool CutFaces(const AuPointArray& cutLine) = 0;

	virtual QMap<quint8, QString> DeletableStates() const
	{
		return QMap<quint8, QString>();
	}

	virtual QStringList DeletableStops() const
	{
		return QStringList();
	}

	virtual void DeleteFaces(const QSet<quint32>& /*faceIds*/)
	{
	}

	// State information is used by doors, landing gear, elevators, etc.
	// Returns true if any changes are made.
	virtual bool DeleteState(quint8 state)
	{
		Q_UNUSED(state);
		return false;
	}

	virtual bool DeleteStop(const QString& stop)
	{
		Q_UNUSED(stop);
		return false;
	}

	// Returns true if 'selInfo' is modified.
	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const = 0;

	// 'divisions' 1 would divide a face one time, presumably in half.
	virtual void DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions)
	{
		Q_UNUSED(selInfo);
		Q_UNUSED(divisions);
	}

	// Returns locations of location to operate this doors.
	virtual void DoorOpLocs(AuPoint& opLoc1, AuPoint& opLoc2) const
	{
		Q_UNUSED(opLoc1);
		Q_UNUSED(opLoc2);
	}

	// Return the distance in pixels to this part from a 'pixel' location.
	// 'mvp' is the model view projection.
	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const = 0;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const = 0;

	// Fills 'edgeIds' with the ids of all my edges that can be selected.
	virtual void EdgeIds(QSet<quint32>& edgeIds) const = 0;

	// Fills 'edges' with one pair of vertices for every visible edge segment.
	virtual void EdgeSegments(AuPointArray& edges) const = 0;

	// Returns surface effect common to all selected faces.
	// Returns EFF_Count if they differ.
	// Returns EFF_Count+1 if none, i.e. no faces on this part selected.
	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const = 0;

	virtual void EffectFaces(SURFACE_EFFECT effect) = 0;

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect) = 0;

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane) = 0;

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane) = 0;

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane) = 0;

	// Applies the EFF_Hull surface effect to faces that do not already have a surface effect.
	// This effect is applied to faces that are lit like the exterior hull, not to interior faces.
	// 'shipHull' is false to apply the effect to buildings.
	// 'shipHull' is true to rotate the effect to spacecraft hulls.
	virtual void EffectHullFTL(double texSize, bool shipHull)
	{
		Q_UNUSED(texSize);
		Q_UNUSED(shipHull);
	}

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
	{
		Q_UNUSED(selInfo);
		Q_UNUSED(intensity);
	}

	// Moves texture coordinates on selected vertices by 'vec'.
	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec) = 0;

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle) = 0;

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY) = 0;

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane) = 0;

	// For contrail emitters.
	virtual float EmitterDiameter() const
	{
		return 0;
	}

	// Return the distance in pixels to this part from a 'pixel' location.
	// 'mvp' is the model view projection.
	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const = 0;

	// Fills 'faceIds' with the ids of all my faces that can be selected.
	virtual void FaceIds(QSet<quint32>& faceIds) const = 0;

	// Called on a void part after 'loc' has already been determined to be inside this void.
	// 'vec' is the direction to look for the nearest face.
	// 'terN' is an output parameter which is the surface normal to the nearest face.
	virtual AuPoint FaceLoc(const AuPoint& loc, const AuPoint& vec, AuPoint& terN, const AuMatrix4x4& stopM) const
	{
		Q_UNUSED(vec);
		Q_UNUSED(terN);
		Q_UNUSED(stopM);
		return loc;
	}

	virtual void Faces(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	virtual void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
	{
		Q_UNUSED(faces);
		Q_UNUSED(selInfo);
	}

	// Gets the faces of this part that should be considered when building the obstruction silhouette of the hull part that I am
	// inside of.
	virtual void FacesInHullSilhouette(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	// 'selFaces' contains the ids of the selected vertices.
	// 'selLocs' is filled with the face locations that are selected.
	// 'unselLocs' is filled with the face locations that are not selected.
	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const = 0;

	virtual quint32 FenceDesignId() const
	{
		return 0;
	}

	virtual void FencePostFaces(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	virtual void FenceSegments(AuSegmentArray& segs) const
	{
		Q_UNUSED(segs);
	}

	// Fence returns an AuScFence::STYLE.
	virtual int FenceStyle() const
	{
		return 0;
	}

	virtual double FenceVolume() const
	{
		return 0;
	}

	// Returns area of agricultural fields, in square feet.
	virtual double FieldArea() const
	{
		return 0;
	}

	// Finds a ladder within 'dist' distance. Updates dist if one is found.
	virtual bool FindLadder(const AuPoint& fromLoc, AuMatrix4x4& mLadder, double& dist)
	{
		Q_UNUSED(fromLoc);
		Q_UNUSED(mLadder);
		Q_UNUSED(dist);
		return false;
	}

	// Prepares this part for inclusion in a finalized design. That is a design that is valid and can be made into a game object.
	// 'partId' is this part's id in 'parts'.
	// Returns true for parts that should remain in the final design.
	// Returns false if this part should be excluded from the final design.
	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull) = 0;

	virtual void FlattenVertices()
	{
	}

	virtual void FlattenVertices(const AuScSelectInfo& selInfo)
	{
		Q_UNUSED(selInfo);
	}

	virtual bool FractureFaces(const AuScSelectInfo& selInfo)
	{
		Q_UNUSED(selInfo);
		return false;
	}

	// Returns the name, preceeded by the name of its associated part. e.g. 'Turbo Lift, Bridge'.
	QString FullName(const AuScParts& parts) const;

	virtual bool FuseFaces(const AuScSelectInfo& selInfo)
	{
		Q_UNUSED(selInfo);
		return false;
	}

	// Note the only known part to override this version is AuScSite.
	virtual void GlowVertexLocs(const AuPointArray& vertLocs, const QColor& glow)
	{
		Q_UNUSED(vertLocs);
		Q_UNUSED(glow);
	}

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
	{
		Q_UNUSED(selInfo);
		Q_UNUSED(glow);
	}

	virtual GRAVITY GravityType() const
	{
		return G_Natural;
	}

	// Returns the gravity up vector for this part.
	virtual AuPoint GravityUp(const AuMatrix4x4& stopM) const
	{
		Q_UNUSED(stopM);
		return AuPoint(0,0,1);
	}

	// Returns the gravity up vector for this part.
	// 'gravityN' is the current environmental gravity up vector.
	virtual AuPoint GravityUp(const AuPoint& gravityN, const AuMatrix4x4& stopM) const
	{
		Q_UNUSED(stopM);
		return gravityN;
	}

	virtual quint16 GroundVehicleParkingSpots() const
	{
		return 0;
	}

	inline const QString& GroupName() const
	{
		return m_Group;
	}

	static const char* GroupNameHull();

	static const char* GroupNameFencePost();

	static const char* GroupNameFenceSection();

	// Test IsGuardPost, IsCitizenPost, or IsLivestockPost first.
	virtual const AuPoint GuardLoc() const
	{
		return AuPoint(0,0);
	}

	inline bool HasGroupName() const
	{
		return !m_Group.isEmpty();
	}

	// Some parts have a halo of glow effect as if they are a light, without actually emitting light. e.g. nav light, strobe.
	virtual bool HasLightGlow() const
	{
		return false;
	}

	// Some parts get rendered in a separate pass, where each part can have it's own texture, namely screens, icons and sketches.
	virtual bool HasOwnTexture() const
	{
		return false;
	}

	// State information is used by doors, landing gear, elevators, etc.
	// Returns the current state value.
	virtual bool HasState(quint8 state) const
	{
		Q_UNUSED(state);
		return false;
	}

	// Returns the distance from 'loc' down to the nearest hard floor, in the direction of gravity.
	// 'gravityN' is the environmental gravity up vector.
	// 'stopM' is used to transform the location of this part, for lift stops.
	// 'floorN' is a return value, set to the surface normal of the floor.
	virtual double HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM, AuPoint& floorN) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(gravityN);
		Q_UNUSED(stopM);
		Q_UNUSED(floorN);
		return 0;
	}

	// Returns the distance from 'loc' up to the nearest hard ceiling, against the direction of gravity.
	// 'stopM' is used to transform the location of this part, for lift stops.
	virtual double HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(gravityN);
		Q_UNUSED(stopM);
		return 0;
	}

	// Returns the access to the hold when someone is in a room.
	virtual HOLD_ACCESS HoldAccess() const
	{
		return HOLD_Inaccessible;
	}

	virtual bool HullExtents(AuPoint& min, AuPoint& max) const
	{
		Q_UNUSED(min);
		Q_UNUSED(max);
		return false;
	}

	// Gets the extents of hull and landing gear in its current state.
	// 'partId' is the id of this part.
	virtual bool HullExtents(quint16 partId, AuPoint& min, AuPoint& max, const AuLandingGearStateHash& landingGearStates) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(min);
		Q_UNUSED(max);
		Q_UNUSED(landingGearStates);
		return false;
	}
	
	// IsHullMesh parts return their hull volume in cubic meters.
	// 'parts' are passed so the part can exclude portions of itself that are inside parts that have a lower part id.
	// 'partId' is my id in 'parts'.
	virtual quint32 HullVolume(quint16 partId, const AuScParts& parts) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		return 0;
	}

	// Add to 'voxels' wherever this part contributes to the hull volume, when subdivided into 'voxelSize' cubes.
	// 'voxels' key is voxel location, value is not used.
	virtual quint32 HullVolumeVoxels(quint16 partId, const AuScParts& parts, QMap<AuPoint, bool>& voxels, double voxelSize) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(voxels);
		Q_UNUSED(voxelSize);
		return 0;
	}

	// Returns true if the 2D location of 'loc' falls within this site mesh.
	virtual bool InBuildingSite(const AuPoint& loc) const
	{
		Q_UNUSED(loc);
		return false;
	}

	// 'hideGrass' can be 0=no, 1=yes, 2=mow, 3=lawn
	virtual bool InBuildingSite(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(siteGround);
		Q_UNUSED(hideGrass);
		return false;
	}

	// Returns true if the 3D location of 'loc' falls ABOVE any of my build zone meshes.
	virtual bool InBuildZone(const AuPoint& loc) const
	{
		Q_UNUSED(loc);
		return false;
	}

	// Returns true if the 3D location of 'loc' falls ABOVE any of my build zone meshes, 'withinHeight' distance above it.
	virtual bool InBuildZone(const AuPoint& loc, double withinHeight) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(withinHeight);
		return false;
	}

	virtual bool InteriorIsSunlit() const
	{
		return true;
	}

	virtual INTERIOR_LIGHT InteriorLight() const
	{
		return INTLIGHT_Dark;
	}

	// Return true if any part of this part falls within the view rectangle 'pixelRect', in pixels.
	// 'mvp' is the model view projection.
	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const = 0;

	// Return true if any part of this part falls within the view rectangle 'pixelRect', in pixels.
	// 'mvp' is the model view projection.
	// 'selInfo' is filled with the specific vertex ids that fall inside 'pixelRect'.
	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const = 0;

	// 'personnelAccess' is a combination of PERSONNEL_ACCESS bits representing who is requesting access.
	virtual bool IsAccessGranted(quint8 personnelAccess) const
	{
		Q_UNUSED(personnelAccess);
		return false;
	}

	virtual bool IsAutomaticDoor() const
	{
		return false;
	}

	// Returns true for solid volumes that obstruct movement.
	virtual bool IsBarrier() const
	{
		return false;
	}

	virtual bool IsBerth() const
	{
		return false;
	}

	virtual bool IsBuildZone() const
	{
		return false;
	}

	virtual bool IsCamera() const
	{
		return false;
	}

	virtual bool IsCitizenPost() const
	{
		return false;
	}

	virtual bool IsCommandStation() const
	{
		return false;
	}

	// Returns true for old style consoles and command chairs.
	virtual bool IsConsole() const
	{
		return false;
	}

	virtual bool IsContrailEmitter() const
	{
		return false;
	}

	virtual bool IsControlStation() const
	{
		return false;
	}

	// Returns true for parts whose faces must be blended on after solid faces are painted. 
	// These get painted in the same pass as the solids.
	virtual bool IsDecal() const
	{
		return false;
	}

	virtual bool IsDecalJig() const
	{
		return false;
	}

	virtual bool IsDoor() const
	{
		return false;
	}

	// Returns true for jigs that cut doors.
	virtual bool IsDoorJig() const
	{
		return false;
	}

	virtual bool IsEnvironmentVoid() const
	{
		return false;
	}

	virtual bool IsFence() const
	{
		return false;
	}

	virtual bool IsGameTable() const
	{
		return false;
	}

	virtual bool IsGuardPost() const
	{
		return false;
	}

	virtual bool IsHelm() const
	{
		return false;
	}

	inline bool IsHidden() const
	{
		return m_Bits & BIT_Hide;
	}

	virtual bool IsHullDoor() const
	{
		return false;
	}

	// Returns true for exterior hull meshes.
	virtual bool IsHullMesh() const
	{
		return false;
	}

	virtual bool IsHullVoid() const
	{
		return false;
	}

	// Returns true for doors that are initially locked when a design is made.
	virtual bool IsInitiallyLocked() const
	{
		return false;
	}

	virtual bool IsInsideEnvironmentVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(loc);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		Q_UNUSED(liftStops);
		return false;
	}

	// Returns true if 'loc' is inside my obstruction model. Does not consider voids.
	virtual bool IsInsideObstructionModel(const AuPoint& loc) const
	{
		Q_UNUSED(loc);
		return false;
	}

	virtual bool IsInsideObstructionModel(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		return IsInsideObstructionModel(loc);
	}

	// Checks to see if 'loc' is inside a void space created by this part.
	// 'parts' are needed by voids that depend on the state of an associated part.
	virtual bool IsInsideRoomVoid(const AuPoint& loc, const AuScParts& parts) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(parts);
		return false;
	}

	virtual bool IsInsideRoomVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(loc);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		Q_UNUSED(liftStops);
		return false;
	}

	// Checks to see if 'loc' is inside a void space created by this part.
	// 'loc' is elevated by 'elevate' with respect to gravity in each void, when making the inside room test.
	// This is used to find the room that contains a transporter console, which could be placed exactly on the floor.
	// 'parts' are needed by voids that depend on the state of an associated part.
	virtual bool IsInsideRoomVoid(const AuPoint& loc, double elevate, const AuScParts& parts) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(elevate);
		Q_UNUSED(parts);
		return false;
	}

	virtual bool IsInsideRoomVoid(quint16 partId, const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(loc);
		Q_UNUSED(elevate);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		Q_UNUSED(liftStops);
		return false;
	}

	// Checks to see if 'loc' is inside a void space created by this part.
	virtual bool IsInsideVoid(const AuPoint& loc) const
	{
		Q_UNUSED(loc);
		return false;
	}

	// Checks to see if 'loc' is inside a void space created by this part.
	// 'parts' are used by voids that depend on the state of an associated part.
	virtual bool IsInsideVoid(const AuPoint& loc, const AuScParts& parts) const
	{
		Q_UNUSED(loc);
		Q_UNUSED(parts);
		return false;
	}

	virtual bool IsInsideVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(loc);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		Q_UNUSED(liftStops);
		return false;
	}

	// Checks to see if 'loc' is ever inside a void space created by this part, regardless of this part's current state or stop.
	virtual bool IsInsideVoidEver(const AuPoint& loc) const
	{
		Q_UNUSED(loc);
		return false;
	}

	// Returns true for all jigs.
	virtual bool IsJig() const
	{
		return false;
	}

	virtual bool IsLadderPath() const
	{
		return false;
	}

	virtual bool IsLandingGear() const
	{
		return false;
	}

	// Returns true for jigs that cut launch tubes.
	virtual bool IsLaunchTubeJig() const
	{
		return false;
	}

	// Returns true if this part emits light. Must override Light functions.
	virtual bool IsLight() const
	{
		return false;
	}

	// Returns true for light lens whose light color property can be changed.
	virtual bool IsLightLens() const
	{
		return false;
	}

	virtual bool IsLivestockPost() const
	{
		return false;
	}

	virtual bool IsLocked() const
	{
		return m_Bits & BIT_Lock;
	}

	// Returns true for anything that only needs to have one vertex inside the hull obstruction model, i.e. light lenses.
	virtual bool IsLooselyAttachedToHull() const
	{
		return false;
	}

	virtual bool IsMesh() const
	{
		return false;
	}

	virtual bool IsOmitted() const
	{
		return m_Bits & BIT_Omit;
	}

	// Return true for parts that may only have one instance per design.
	virtual bool IsOnePerDesign() const
	{
		return false;
	}

	// Returns true if this part contributes to the pathing data.
	virtual bool IsPath() const
	{
		return false;
	}

	virtual bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
	{
		Q_UNUSED(fromLoc);
		Q_UNUSED(toLoc);
		Q_UNUSED(hitLoc);
		Q_UNUSED(hitNormal);
		return false;
	}

	// 'partId' is my part id, for looking myself up in the states hashes.
	virtual bool IsPathBlocked(quint16 partId, const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		Q_UNUSED(liftStops);
		return IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal);
	}

	// Returns true for doors that are controlled using remote door switches in spacecraft.
	virtual bool IsRemoteSpacecraftDoor() const
	{
		return false;
	}

	// Returns true for doors that are controlled using remote door switches in vehicles.
	virtual bool IsRemoteVehicleDoor() const
	{
		return false;
	}

	// Returns true for interior room meshes that can be jig cut.
	virtual bool IsRoomMesh() const
	{
		return false;
	}

	virtual bool IsRoomVoid() const
	{
		return false;
	}

	virtual bool IsSealed() const
	{
		return m_Bits & BIT_Seal;
	}

	virtual bool IsSensorStation() const
	{
		return false;
	}

	virtual bool IsServicePanel() const
	{
		return false;
	}

	virtual bool IsShieldStation() const
	{
		return false;
	}

	virtual bool IsSite() const
	{
		return false;
	}

	virtual bool IsStatusDisplay() const
	{
		return false;
	}

	virtual bool IsTurboLiftDoor() const
	{
		return false;
	}

	// Returns true for turbo lift meshes.
	virtual bool IsTurboLiftMesh() const
	{
		return false;
	}

	virtual bool IsTurret() const
	{
		return false;
	}

	// Returns true if the properties of this part are ok to be in a design.
	virtual bool IsValid() const = 0;

	// Return false for parts that are not rendered in the primary model collection pass of the final version of this model.
	virtual bool IsVisibleInFinalModel() const
	{
		return true;
	}

	// Returns true for void spaces.
	virtual bool IsVoid() const
	{
		return false;
	}

	// Returns true for windows.
	virtual bool IsWindow() const
	{
		return false;
	}

	// Returns true for jigs that cut windows.
	virtual bool IsWindowJig() const
	{
		return false;
	}

	// Gets faces to be cut by a jig.
	virtual void JigCutFaces(AuScFaceList& faces) const
	{
		Faces(faces);
	}

	// Gets cutting faces from a jig.
	virtual void JigFaces(AuScFaceList& /*jigFaces*/) const
	{
	}

	// Returns false if this is not a landing system.
	virtual bool LandingSystemPosition(AuMatrix4x4& landerPosM) const
	{
		Q_UNUSED(landerPosM);
		return false;
	}

	virtual bool LaunchPosition(AuMatrix4x4& launchPadM, AuPoint& launchExit) const
	{
		Q_UNUSED(launchPadM);
		Q_UNUSED(launchExit);
		return false;
	}

	virtual QColor LightColor() const
	{
		return QColor(255,255,255);
	}

	virtual float LightCosInnerCone() const
	{
		return -1;
	}

	virtual float LightCosOuterCone() const
	{
		return -1;
	}

	virtual AuPoint LightDir() const
	{
		return AuPoint::Null();
	}

	virtual float LightkL() const
	{
		return 0;
	}

	virtual float LightkQ() const
	{
		return 0;
	}

	virtual AuPoint LightLoc() const
	{
		return AuPoint::Null();
	}

	virtual const AuPoint& LightRotationAxis() const
	{
		return AuPoint::Null();
	}

	// 'streamline' is an AuScDesign::STREAMLINE enum.
	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor,
			COMMODITYID hullMaterial) const = 0;

	virtual quint64 MemoryUsage() const;

	// Returns the extents of every vertex of the model, including all states of parts that have them.
	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const = 0;

	// Returns the module associated with a part. This implicitly creates a system if the module is a system module.
	// FTL Drives and Weapons use this when allocating hull volume.
	virtual COMMODITYID Module() const
	{
		return CID_Invalid;
	}

	// Returns the system associated with a part type if it can have a module installed.
	inline SYSTEM ModuleSystem() const
	{
		return ModuleSystem(Type());
	}

	static SYSTEM ModuleSystem(PART_TYPE cid);

	// Returns the number added to the design analysis.
	virtual quint16 OfficerBerths() const
	{
		return 0;
	}

	// Returns the opacity of this part, from 0 to 1, where 0 is invisible and 1 is solid. Alpha value when blended.
	// This is separate from the Opacity of a face, intended for parts with overall opacity, such as a window or sketch.
	virtual float Opacity() const
	{
		return 1;
	}

	// Returns operator's location for consoles and stations.
	virtual AuPoint OperatorLoc() const
	{
		return AuPoint(0,0);
	}

	// Returns the station type and operator positioning for the operator of this station.
	// 'gravityUp' is the up direction in the void space containing this station.
	virtual AuStation OperatorStation(const AuPoint& gravityUp) const
	{
		Q_UNUSED(gravityUp);
		return AuStation();
	}

	virtual GLuint OwnTexture(AuGLEffects& /*re*/) const
	{
		return 0;
	}

	// Packs this part's type and data into a byte array using operator>>. Use Unpack to restore.
	QByteArray Pack() const;

	// Fills 'padLocs' with transporter pad locations, which should be on the floor.
	virtual void PadLocs(AuPointArray& padLocs) const
	{
		Q_UNUSED(padLocs);
	}

	virtual void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
	{
		Q_UNUSED(selInfo);
		Q_UNUSED(paint);
	}

	virtual void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
	{
		Q_UNUSED(selInfo);
		Q_UNUSED(theme);
		Q_UNUSED(opacity);
	}

	virtual void PaintInUse(QList<QColor>& paintInUse) const
	{
		Q_UNUSED(paintInUse);
	}

	virtual QString PartName() const
	{
		return TypeName();
	}

	// Returns the number added to the design analysis.
	virtual quint16 PassengerBerths() const
	{
		return 0;
	}

	virtual const AuScPathSeg& PathSegment(int segmentIndex) const
	{
		Q_UNUSED(segmentIndex);
		return AuScPathSeg::Null();
	}

	virtual int PathSegments() const
	{
		return 0;
	}

	// Returns patient's location for surgery consoles and stations.
	virtual const AuPoint PatientLoc() const
	{
		return AuPoint(0,0);
	}

	virtual PATIENT_POSITION PatientPos() const
	{
		return Patient_Stand;
	}

	// Returns the station type and operator positioning for the patient at this station.
	// 'opLoc' is the current location of the operator. Stations with multiple positions return the closest one.
	// 'gravityUp' is the up direction in the void space containing this station.
	virtual AuStation PatientStation(const AuPoint& gravityUp) const
	{
		Q_UNUSED(gravityUp);
		return AuStation();
	}

	// Returns a bitwise combination of PERSONNEL_ACCESS bits.
	virtual quint8 PersonnelAccess() const
	{
		return PERSONNEL_Everyone;
	}

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
	{
		Q_UNUSED(selInfo);
		Q_UNUSED(shine);
		Q_UNUSED(reflectivity);
	}

	// Used for placing control console models and game tables according to their icon rotation.
	// Also used for placing vehicles on their parking spots.
	// Also used for placing people in their berths.
	virtual AuMatrix4x4 PositionMatrix() const
	{
		return AuMatrix4x4();
	}

	// Called on each part at the end of operator<<, to fix things that need fixing. It originated because of AuScMesh losing its
	// part association when group names were added. The AuScMesh stores its associated room id as its group name. This function
	// will use it to find the part and get its group name.
	virtual void PostReadCleanup(AuScParts& parts)
	{
		Q_UNUSED(parts);
	}

	virtual bool PunchFaces(const AuPointArray& cutBnd) = 0;

	virtual bool RandomBuildZoneLocation(AuPoint& zoneLoc) const
	{
		Q_UNUSED(zoneLoc);
		return false;
	}

	virtual bool RandomSiteLocation(const AuScParts& parts, AuPoint& siteLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const
	{
		Q_UNUSED(parts);
		Q_UNUSED(siteLoc);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		return false;
	}

	virtual bool RandomVoidLocation(quint16 partId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(voidLoc);
		Q_UNUSED(doorStates);
		Q_UNUSED(landingGearStates);
		Q_UNUSED(liftStops);
		return false;
	}

	// Returns version read from data stream.
	// Returns 0 if I was not read from a data stream.
	quint8 ReadVersion() const;

	// 'partId' is this part's id in 'parts'.
	virtual void RefreshObstructionsCache(quint16 partId, const AuScParts& parts)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
	}

	// 'partId' is this part's id in 'parts'.
	virtual bool RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		return false;
	}

	virtual bool RenameStop(const QString& oldStopName, const QString& nuStopName)
	{
		Q_UNUSED(oldStopName);
		Q_UNUSED(nuStopName);
		return false;
	}

	// Returns a short name of this part when it is mentioned on an analysis report.
	virtual QString ReportDesc() const
	{
		return TypeName();
	}

	// Fills 'tris' with the triangles needed to build the VAO to render the part. OpenGL context not needed.
	// This base class implementation does nothing.
	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	// Used when getting triangles for a flag decal, to scale the result according to the aspect ratio of the flag image.
	virtual void RenderTrianglesScaled(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count], float scaleX, float scaleY) const
	{
		Q_UNUSED(opaqueTris);
		Q_UNUSED(transparentTris);
		Q_UNUSED(themeColors);
		Q_UNUSED(scaleX);
		Q_UNUSED(scaleY);
	}

	// Fills 'tris' with the triangles needed to build the VAO to render a specific state of the part. OpenGL context not needed.
	// Parts that do not have states should not override this.
	// This base class implementation does nothing.
	virtual void RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count], quint8 state) const;

	virtual void ReverseFaces()
	{
	}

	virtual void ReverseFaces(const AuScSelectInfo& selInfo)
	{
		Q_UNUSED(selInfo);
	}

	virtual void ReverseVertexNormals()
	{
	}

	virtual quint16 RocketParkingSpots() const
	{
		return 0;
	}

	// Returns true on success.
	virtual bool ScreenQuad(AuQuad& screen) const
	{
		Q_UNUSED(screen);
		return false;
	}

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const = 0;

	// Returns description of part when it is selected with the mouse.
	virtual QString SelectedDesc(const AuScParts& parts) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const = 0;

	// Selects only one face.
	// Returns true if 'selInfo' is modified.
	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const = 0;

	// Selects all faces whose grip is at 'atLoc'.
	// Returns true if 'selInfo' is modified.
	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const = 0;

	// Returns true if 'selInfo' is modified.
	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const = 0;

	inline void SetArchitect(const AuId& id, const QString& name)
	{
		m_ArchId = id;
		m_ArchName = name;
	}

	// Sets the architect id and name if current architect is null. This is used to preserve ids of
	// architects of parts that were not created by 'id'. See corresponding ClearArchitectForFileSave function.
	inline void SetArchitectAfterFileLoad(const AuId& id, const QString& name)
	{
		if(ArchId().IsNull())
			SetArchitect(id, name);
	}

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		Q_UNUSED(partId);
		Q_UNUSED(partState);
	}

	// Switch locations can be AuPoint::Invalid to restore them to the center of the closed door geometry.
	virtual void SetDoorOpLocs(const AuPoint& switch1, const AuPoint& switch2)
	{
		Q_UNUSED(switch1);
		Q_UNUSED(switch2);
	}

	virtual void SetDoorProperties(Qt::CheckState isAutomatic, Qt::CheckState initLocked, Qt::CheckState remoteSpacecraft,
			Qt::CheckState remoteVehicle)
	{
		Q_UNUSED(isAutomatic);
		Q_UNUSED(initLocked);
		Q_UNUSED(remoteSpacecraft);
		Q_UNUSED(remoteVehicle);
	}

	virtual void SetEmitLoc(const AuPoint& emitLoc)
	{
		Q_UNUSED(emitLoc);
	}

	virtual void SetFaceOpacity(float opacity) = 0;

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity) = 0;

	virtual void SetFaces(const AuScFaceList& /*faces*/)
	{
	}

	virtual void SetFaceSelfIllumination(float selfIllum) = 0;

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum) = 0;

	virtual void SetGravity(GRAVITY g, const AuPoint& up)
	{
		Q_UNUSED(g);
		Q_UNUSED(up);
	}

	// Caller must validate that 'name' is either blank or passes the IsNameValid test. I need this to accept invalid names
	// when fixing groups on old version meshes.
	inline void SetGroupName(const QString& name)
	{
		m_Group = name;
	}

	inline void SetHidden(bool hide)
	{
		if(hide)
			m_Bits |= BIT_Hide;			// Turn the bit on.
		else
			m_Bits &= ~BIT_Hide;		// Turn the bit off.
	}

	virtual void SetHoldAccess(HOLD_ACCESS holdAccess)
	{
		Q_UNUSED(holdAccess);
	}

	virtual void SetHullDoor(bool hullDoor)
	{
		Q_UNUSED(hullDoor);
	}

	virtual void SetInteriorLighting(bool isSunlit, INTERIOR_LIGHT initLight)
	{
		Q_UNUSED(isSunlit);
		Q_UNUSED(initLight);
	}

	// Set faces with result of me getting cut by a jig.
	virtual void SetJigCutFaces(const AuScFaceList& faces)
	{
		SetFaces(faces);
	}

	inline void SetLocked(bool lock)
	{
		if(lock)
			m_Bits |= BIT_Lock;			// Turn the bit on.
		else
			m_Bits &= ~BIT_Lock;		// Turn the bit off.
	}

	virtual void SetLight(const QColor& color, float kL, float kQ, float cosOuterCone, float cosInnerCone)
	{
		Q_UNUSED(color);
		Q_UNUSED(kL);
		Q_UNUSED(kQ);
		Q_UNUSED(cosOuterCone);
		Q_UNUSED(cosInnerCone);
	}

	virtual void SetLightColor(const QColor& color)
	{
		Q_UNUSED(color);
	}

	virtual void SetModule(COMMODITYID cid)
	{
		Q_UNUSED(cid);
	}

	inline void SetOmitted(bool omit)
	{
		if(omit)
			m_Bits |= BIT_Omit;			// Turn the bit on.
		else
			m_Bits &= ~BIT_Omit;		// Turn the bit off.
	}

	virtual void SetOpacity(float o)
	{
		Q_UNUSED(o);
	}

	// 'personnelAccess' is a bitwise combination of PERSONNEL_ACCESS values.
	virtual void SetPersonnelAccess(quint8 personnelAccess)
	{
		Q_UNUSED(personnelAccess);
	}

	// Only the architect is permitted to change the state of the seal bit.
	inline void SetSealed(bool seal)
	{
		if(seal)
			m_Bits |= BIT_Seal;			// Turn the bit on.
		else
			m_Bits &= ~BIT_Seal;		// Turn the bit off.
	}

	virtual void SetSiteProperties(quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs, quint8 terBlend,
			quint8 trees)
	{
		Q_UNUSED(grade);
		Q_UNUSED(grass);
		Q_UNUSED(pavement);
		Q_UNUSED(rocks);
		Q_UNUSED(shrubs);
		Q_UNUSED(terBlend);
		Q_UNUSED(trees);
	}

	// State information is used by doors, landing gear, elevators, etc.
	virtual void SetState(quint8 state)
	{
		Q_UNUSED(state);
	}

	// Sets the current stop position.
	// The empty stop position is the natural model position for editing. We only edit it in that position.
	virtual void SetStop(const QString& stop)
	{
		Q_UNUSED(stop);
	}

	// Changes the stop name at a door.
	virtual void SetStopName(const QString& name)
	{
		Q_UNUSED(name);
	}

	virtual void SetPadLocs(const AuPointArray& padLocs)
	{
		Q_UNUSED(padLocs);
	}

	virtual void SetWhichEffect(int which)
	{
		Q_UNUSED(which);
	}

	// Check sight segment from 'eyeLoc' to 'sightLoc'.
	// 'partId' is the id of this part, needed for checking door and landing gear states.
	// Returns true if the sight segment hits me.
	// If I am hit, 'sightLoc' is adjusted closer to 'eyeLoc', to the point that was hit, and 'sightSurfaceNormal' is set.
	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const = 0;

	virtual bool SightBuildZone(const AuPoint& eyeLoc, AuPoint& sightLoc) const
	{
		Q_UNUSED(eyeLoc);
		Q_UNUSED(sightLoc);
		return false;
	}

	// Returns the system represented by this part when it is sighted.
	virtual AuSysId SightedSystem(quint16 partId) const = 0;

	virtual bool SiteExtents(AuPoint& min, AuPoint& max) const
	{
		Q_UNUSED(min);
		Q_UNUSED(max);
		return false;
	}

	virtual void SmoothVertices()
	{
	}

	virtual void SmoothVertices(const AuScSelectInfo& selInfo)
	{
		Q_UNUSED(selInfo);
	}

	virtual PART_SOUND Sound() const
	{
		return SOUND_Silent;
	}

	virtual AuPoint SoundEmitLoc() const
	{
		return AuPoint(0,0);
	}

	virtual quint16 SpacecraftParkingSpots() const
	{
		return 0;
	}

	virtual quint16 SpaceVehicleParkingSpots() const
	{
		return 0;
	}

	// State information is used by doors, landing gear, elevators, etc.
	// Returns the current state value.
	virtual quint8 State() const
	{
		return 0;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::TYPE_Count;
	}

	// Returns the stop matrix for a stop named 'stopName'.
	virtual const AuMatrix4x4& StopMatrixAt(const QString& stopName) const
	{
		Q_UNUSED(stopName);
		return AuMatrix4x4::IdentityMatrix();
	}

	// Returns the stop matrix for a lift. 'partId' is this part's id. 'liftStops' is the current stops for all lifts.
	virtual const AuMatrix4x4& StopMatrixAt(quint16 partId, const AuLiftHash& liftStops) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(liftStops);
		return AuMatrix4x4::IdentityMatrix();
	}

	// Returns the stop matrix for a lift based on it's design stop setting. Used in design environment.
	virtual const AuMatrix4x4& StopMatrixDesign() const
	{
		return AuMatrix4x4::IdentityMatrix();
	}

	// Returns the stop name of a turbo lift door.
	virtual QString StopName() const
	{
		return "";
	}

	// Returns the name of the current state value.
	virtual QString StateName() const
	{
		return StateName(State());
	}

	virtual QString StateName(quint8 state) const
	{
		Q_UNUSED(state);
		return "";
	}

	// Returns a map of all possible states (keys) and names (values).
	virtual QMap<quint8, QString> StateNames() const
	{
		return QMap<quint8, QString>();
	}

	// Returns a map of all possible states (keys) and names (values).
	virtual QStringList StopNames() const
	{
		return QStringList();
	}

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex) = 0;

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex) = 0;

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane) = 0;

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane) = 0;

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane) = 0;

	// Returns SC_TEXTURE_INDEX common to all selected faces.
	// Returns SCTEXI_Count if they differ.
	// Returns SCTEXI_Count+1 if none, i.e. no faces on this part selected.
	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const = 0;

	// Moves texture coordinates on selected vertices by 'vec'.
	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec) = 0;

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle) = 0;

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY) = 0;

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane) = 0;

	// Part mode. Applies to the whole part.
	virtual void Transform(const AuMatrix4x4& t);

	// Object mode. Applies to selected parts of this part.
	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	// Object mode. Applies to selected parts of this part.
	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	// Part mode. Applies only to the current stop of a lift.
	virtual void TransformLiftStop(const AuMatrix4x4& t)
	{
		Q_UNUSED(t);
	}

	// Returns the number added to the design analysis.
	virtual quint16 TroopBerths() const
	{
		return 0;
	}

	// Sets 'm' to the eye view matrix at the gun barrel, for aiming by NPCs.
	// Project (0,0) through 'm' to get the current barrel pivot location, based on 'baseRot' and 'gunRot'.
	// Project vector (0,0,-1) through 'm' to get its current emit direction.
	virtual void TurretAimMatrix(float baseRot, AuMatrix4x4& m) const
	{
		Q_UNUSED(m);
		Q_UNUSED(baseRot);
	}

	// Sets 'm' to the model matrix for the base of a turret, at 'baseRot' rotation.
	virtual void TurretBaseMatrix(float baseRot, AuMatrix4x4& m) const
	{
		Q_UNUSED(m);
		Q_UNUSED(baseRot);
	}

	// Sets 'm' to the eye view matrix at the gun sight.
	// Project (0,0) through 'm' to get the current emit location, based on 'baseRot' and 'gunRot'.
	// Project vector (0,0,-1) through 'm' to get its current emit direction.
	virtual void TurretSightMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const
	{
		Q_UNUSED(m);
		Q_UNUSED(baseRot);
		Q_UNUSED(gunRot);
	}

	// Sets 'm' to the model matrix for the gun of a turret, at 'baseRot' and 'gunRot' rotations.
	virtual void TurretGunMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const
	{
		Q_UNUSED(m);
		Q_UNUSED(baseRot);
		Q_UNUSED(gunRot);
	}

	// Sets 'm' to the eye view matrix at the gun muzzle.
	// Project (0,0) through 'm' to get the current emit location, based on 'baseRot' and 'gunRot'.
	// Project vector (0,0,1) through 'm' to get its current emit direction.
	virtual void TurretMuzzleMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const
	{
		Q_UNUSED(m);
		Q_UNUSED(baseRot);
		Q_UNUSED(gunRot);
	}

	// Returns the number added to the design analysis.
	virtual quint16 Turrets() const
	{
		return 0;
	}

	// Each derived class must return a unique type for identifying the object in a data stream.
	virtual PART_TYPE Type() const = 0;

	virtual QString TypeIcon() const = 0;

	virtual QString TypeName() const = 0;

	// Restores a part from a byte array that was stored using Pack.
	// The returned pointer is allocated on the heap and becomes the property of the caller.
	static AuScPart* Unpack(const QByteArray& ba);

	virtual void Update(const AuScPart& c)
	{
		m_ArchId = c.m_ArchId;
		m_ArchName = c.m_ArchName;
		m_Bits = c.m_Bits;
		m_Group = c.m_Group;
		m_StreamVersion = c.m_StreamVersion;
	}

	virtual const AuPoint VehicleExitLoc() const
	{
		return AuPoint(0,0);
	}

	virtual const AuPoint VehicleParkLoc() const
	{
		return AuPoint(0,0);
	}

	// Return the distance in pixels to this part from a 'pixel' location.
	// 'mvp' is the model view projection.
	// 'vertexIndex' is set to the vertex id of the closest vertex.
	// 'snapLoc' may be 0. If not 0, the vertex location on the part is returned there.
	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const = 0;

	// Fills 'vertexIds' with the ids of all my vertexes that can be selected.
	virtual void VertexIds(QSet<quint32>& vertexIds) const = 0;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const = 0;

	// 'selVerts' contains the ids of the selected vertices.
	// 'selLocs' is filled with the vertex locations that are selected.
	// 'unselLocs' is filled with the vertex locations that are not selected.
	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const = 0;

	virtual AuPoint VLSEmitLoc() const
	{
		return AuPoint(0,0);
	}

	virtual AuPoint VLSEmitVec() const
	{
		return AuPoint(0,0);
	}

	virtual quint16 WaterVehicleParkingSpots() const
	{
		return 0;
	}

	// Bay weapons and turrets.
	virtual AuPoint WeaponEmitLoc() const
	{
		return AuPoint(0,0);
	}

	virtual int WhichEffect() const
	{
		return 0;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScPart& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScPart& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScMeshBase : public AuScPart
{
protected:
	AuScFaceList						m_Faces;

public:	// Constructors
	AuScMeshBase();
	AuScMeshBase(const AuScMeshBase& c);
	AuScMeshBase(const AuScMeshBase& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void AddFaces(const AuScFaceList& faces);

	virtual bool CanFracture() const
	{
		return true;
	}

	virtual bool CanFuse() const
	{
		return true;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// A simple mesh can be joined to any other mesh.
		return dynamic_cast<const AuScMeshBase*>(joinTarget);
	}

	virtual bool CanMakeIntoOtherPart() const
	{
		return true;
	}

	virtual AuPoint Center() const;

	virtual bool CutFaces(const AuPointArray& cutLine);

	virtual void DeleteFaces(const QSet<quint32>& faceIds);

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions);

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeIds(QSet<quint32>& edgeIds) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const;

	virtual void EffectFaces(SURFACE_EFFECT effect);

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual void EffectHullFTL(double texSize, bool shipHull);

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual void FaceIds(QSet<quint32>& faceIds) const;

	virtual void Faces(AuScFaceList& faces) const;

	virtual void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	virtual void FlattenVertices();

	virtual void FlattenVertices(const AuScSelectInfo& selInfo);

	virtual bool FractureFaces(const AuScSelectInfo& selInfo);

	virtual bool FuseFaces(const AuScSelectInfo& selInfo);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	virtual bool IsMesh() const
	{
		return true;
	}

	virtual bool IsValid() const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	// 'extVec' is relative to the gridPlane.
	void MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
			bool closeEnds);

	// Makes a lat/long type sphere.
	void MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps, const AuMatrix4x4& gridPlane);

	// Makes an evenly tessellated sphere.
	void MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
			const AuMatrix4x4& gridPlane);

	// 'profile' is drawn in the XY gridPlane. It spins around the X axis. Drawn from -X to +X, the triangles face outward.
	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane);

	void MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides);

	// 'shape' is expected to be drawn in 2D. The origin is the guide along which 'shape' follows 'path'.
	void MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides);

	virtual quint64 MemoryUsage() const;

	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint);

	virtual void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity);

	virtual void PaintInUse(QList<QColor>& paintInUse) const;

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	virtual bool PunchFaces(const AuPointArray& cutBnd);

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void ReverseFaces();

	virtual void ReverseFaces(const AuScSelectInfo& selInfo);

	virtual void ReverseVertexNormals();

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void SetFaceOpacity(float opacity);

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity);

	virtual void SetFaces(const AuScFaceList& faces)
	{
		m_Faces = faces;
	}

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void SmoothVertices();

	virtual void SmoothVertices(const AuScSelectInfo& selInfo);

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex);

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex);

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const;

	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScMeshBase& c = dynamic_cast<const AuScMeshBase&>(part);

			m_Faces = c.m_Faces;

			AuScPart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexIds(QSet<quint32>& vertexIds) const;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScMesh : public AuScMeshBase
{
public:	// Constructors
	AuScMesh();
	AuScMesh(const AuScMesh& c);
	AuScMesh(const AuScMesh& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool IsValid() const;

	// Generic mesh that is associated with a room will be collected by the room itself.
	virtual bool IsVisibleInFinalModel() const
	{
		return !AssociatedPartId();
	}

	virtual quint64 MemoryUsage() const;

	virtual void PostReadCleanup(AuScParts& parts);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Mesh;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScMesh& c = dynamic_cast<const AuScMesh&>(part);
			Q_UNUSED(c);
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScDoorJig : public AuScMeshBase
{
public:	// Constructors
	AuScDoorJig();
	AuScDoorJig(const AuScDoorJig& c);
	AuScDoorJig(const AuScDoorJig& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	// Jigs are not needed in the final design.
	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return false;
	}

	virtual bool IsDoorJig() const
	{
		return true;
	}

	// Returns true for all jigs.
	virtual bool IsJig() const
	{
		return true;
	}

	// Jigs are always omitted from the finalized design.
	virtual bool IsOmitted() const
	{
		return true;
	}

	// Jigs aren't even present in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	// Jig can't be jig cut.
	virtual void JigCutFaces(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	virtual void JigFaces(AuScFaceList& jigFaces) const
	{
		Faces(jigFaces);
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_DoorJig;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScDoorJig& c = dynamic_cast<const AuScDoorJig&>(part);

			Q_UNUSED(c);

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScWindowJig : public AuScMeshBase
{
public:	// Constructors
	AuScWindowJig();
	AuScWindowJig(const AuScWindowJig& c);
	AuScWindowJig(const AuScWindowJig& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	// Jigs are not needed in the final design.
	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return false;
	}

	// Returns true for all jigs.
	virtual bool IsJig() const
	{
		return true;
	}

	// Jigs are always omitted from the finalized design.
	virtual bool IsOmitted() const
	{
		return true;
	}

	// Jigs aren't even present in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	virtual bool IsWindowJig() const
	{
		return true;
	}

	// Jig can't be jig cut.
	virtual void JigCutFaces(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	virtual void JigFaces(AuScFaceList& jigFaces) const
	{
		Faces(jigFaces);
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_WindowJig;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScWindowJig& c = dynamic_cast<const AuScWindowJig&>(part);

			Q_UNUSED(c);

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScHull : public AuScMeshBase
{
	AuScSilhouette3d					m_Silhouette;			// Used by IsPathBlocked test.

public:	// Constructors
	AuScHull();
	AuScHull(const AuScHull& c);
	AuScHull(const AuScHull& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingConstructionSegments(AuSegmentArray& edges) const;

	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// Hulls can only be joined to a hull mesh.
		return dynamic_cast<const AuScHull*>(joinTarget);
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool HullExtents(AuPoint& min, AuPoint& max) const;

	virtual bool HullExtents(quint16 partId, AuPoint& min, AuPoint& max, const AuLandingGearStateHash& landingGearStates) const;

	virtual quint32 HullVolume(quint16 partId, const AuScParts& parts) const;

	virtual quint32 HullVolumeVoxels(quint16 partId, const AuScParts& parts, QMap<AuPoint, bool>& voxels, double voxelSize) const;

	virtual bool IsHullMesh() const
	{
		return true;
	}

	virtual bool IsInsideObstructionModel(const AuPoint& loc) const;

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual void RefreshObstructionsCache(quint16 partId, const AuScParts& parts);

	virtual bool RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Hull;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScHull& c = dynamic_cast<const AuScHull&>(part);

			m_Silhouette = c.m_Silhouette;

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScRoom : public AuScMeshBase
{
private:
	quint8								m_InitLight;			// INTERIOR_LIGHT enum value.
	bool								m_IsSunlit;

public:	// Constructors
	AuScRoom();
	AuScRoom(const AuScRoom& c);
	AuScRoom(const AuScRoom& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// Rooms can only be joined to a room mesh.
		return dynamic_cast<const AuScRoom*>(joinTarget);
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void FacesInHullSilhouette(AuScFaceList& faces) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual bool InteriorIsSunlit() const
	{
		return m_IsSunlit;
	}

	virtual INTERIOR_LIGHT InteriorLight() const
	{
		return (INTERIOR_LIGHT)m_InitLight;
	}

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual bool IsRoomMesh() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	void MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps);

	void MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps, const AuMatrix4x4& gridPlane);

	void MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
			const AuMatrix4x4& gridPlane);

	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane);

	void MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides);

	void MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides);

	virtual quint64 MemoryUsage() const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual void SetInteriorLighting(bool isSunlit, INTERIOR_LIGHT initLight)
	{
		m_InitLight = initLight;
		m_IsSunlit = isSunlit;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Room;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScRoom& c = dynamic_cast<const AuScRoom&>(part);
			m_InitLight = c.m_InitLight;
			m_IsSunlit = c.m_IsSunlit;
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStatePart : public AuScPart
{
protected:
	quint8								m_State;				// Current state.
	QHash<quint8, AuScFaceList>			m_StateMeshes;			// A different mesh for each state.

public:	// Constructors
	AuScStatePart();
	AuScStatePart(const AuScStatePart& c);
	AuScStatePart(const AuScStatePart& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void AddFaces(const AuScFaceList& faces);

	virtual bool CanChangeState() const
	{
		return true;
	}

	virtual bool CanFracture() const
	{
		return true;
	}

	virtual bool CanFuse() const
	{
		return true;
	}

	virtual bool CanMakeIntoOtherPart() const
	{
		return false;
	}

	virtual AuPoint Center() const;

	virtual quint32 CountFaces() const;

	virtual quint32 CountTriangles() const;

	virtual bool CutFaces(const AuPointArray& cutLine);

	virtual void DeleteFaces(const QSet<quint32>& faceIds);

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions);

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeIds(QSet<quint32>& edgeIds) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const;

	virtual void EffectFaces(SURFACE_EFFECT effect);

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual void EffectHullFTL(double texSize, bool shipHull);

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual void FaceIds(QSet<quint32>& faceIds) const;

	virtual void Faces(AuScFaceList& faces) const;

	virtual void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	virtual void FlattenVertices();

	virtual void FlattenVertices(const AuScSelectInfo& selInfo);

	virtual bool FractureFaces(const AuScSelectInfo& selInfo);

	virtual bool FuseFaces(const AuScSelectInfo& selInfo);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool HasState(quint8 state) const
	{
		return m_StateMeshes.contains(state);
	}

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	virtual bool IsValid() const;

	// 'extVec' is relative to the gridPlane.
	void MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
			bool closeEnds);

	// Makes a lat/long type sphere.
	void MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps, const AuMatrix4x4& gridPlane);

	// Makes an evenly tessellated sphere.
	void MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
			const AuMatrix4x4& gridPlane);

	// 'profile' is drawn in the XY gridPlane. It spins around the X axis. Drawn from -X to +X, the triangles face outward.
	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane);

	void MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides);

	// 'shape' is expected to be drawn in 2D. The origin is the guide along which 'shape' follows 'path'.
	void MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides);

	virtual quint64 MemoryUsage() const;

	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint);

	virtual void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity);

	virtual void PaintInUse(QList<QColor>& paintInUse) const;

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	virtual bool PunchFaces(const AuPointArray& cutBnd);

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void ReverseFaces();

	virtual void ReverseFaces(const AuScSelectInfo& selInfo);

	virtual void ReverseVertexNormals();

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void SetFaceOpacity(float opacity);

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity);

	virtual void SetFaces(const AuScFaceList& faces);

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	virtual void SetState(quint8 state)
	{
		m_State = state;
	}

	virtual void SmoothVertices();

	virtual void SmoothVertices(const AuScSelectInfo& selInfo);

	virtual quint8 State() const
	{
		return m_State;
	}

	inline quint8 StateCount() const
	{
		return m_StateMeshes.size();
	}

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex);

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex);

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const;

	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStatePart& c = dynamic_cast<const AuScStatePart&>(part);

			m_State = c.m_State;
			m_StateMeshes = c.m_StateMeshes;

			AuScPart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexIds(QSet<quint32>& vertexIds) const;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScDoor : public AuScStatePart
{
public:
	enum DOOR_ACCESS
	{
		DOOR_Automatic,					// Automatic doors open when someone is near them and they close when clear.
		DOOR_Standard,					// Standard doors do not open or close automatically.
		DOOR_Lift,						// Lift doors access a lift stop.
		DOOR_HullLift,					// Hull doors connect the inside to the outside. This one accesses a lift stop.
		DOOR_Hull,						// Hull doors connect the inside to the outside.
	};

	enum DOOR_BIT
	{
		DOORBIT_InitLocked = 0x01,		// Door is initially locked when design is built. n/a for hull and lift doors.
		DOORBIT_RemoteSpacecraft = 0x02,	// Door responds to remote door request from spacecraft.
		DOORBIT_RemoteVehicle = 0x04,	// Door responds to remote door request from vehicles.
	};

	enum STATES
	{
		Closed,							// Door completely closed.
		Open10,							// Door 10% open.
		Open20,							// Door 20% open.
		Open30,							// Door 30% open.
		Open40,							// Door 40% open.
		Open50,							// Door 50% open.
		Open60,							// Door 60% open.
		Open70,							// Door 70% open.
		Open80,							// Door 80% open.
		Open90,							// Door 90% open.
		Open,							// Door completely open.
		StuckClosed,					// Door closed and stuck. Easily unstuck by physical open by player, not remote switch.
		StuckOpen,						// Door open and stuck. Easily unstuck by physical close by player, not remote switch.
		LockedClosed,					// Door completely closed and locked.
		LockedOpen,						// Door completely open and locked. Doesn't stay open. Will automatically close.
		STATES_Count					// LAST ITEM
	};

private:
	quint8								m_Access;				// DOOR_ACCESS enum.
	quint8								m_DoorBits;				// DOOR_BITs or'd together.
	AuPoint								m_OpLoc1;				// Location to operate the door.
	AuPoint								m_OpLoc2;				// Location to operate the door.
	AuScSilhouette3d					m_Silhouette;			// Only used for the closed state. Used by IsPathBlocked test.
	QString								m_StopName;				// Doors are named for their turbo lift stop.

public:	// Constructors
	AuScDoor();
	AuScDoor(const AuScDoor& c);
	AuScDoor(const AuScDoor& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual QMap<quint8, QString> AddableStates() const;

	// Adds a new empty state mesh if it does not already exist.
	// Returns true if any changes are made.
	virtual bool AddState(quint8 state);

	virtual bool CanAddState() const
	{
		return StateCount() < STATES_Count;
	}

	virtual bool CanDeleteState() const
	{
		// We don't permit state 0 to be deleted.
		return StateCount() > 1;
	}

	virtual bool CanChangeStopName() const;

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// Doors cannot be joined to anything.
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual QMap<quint8, QString> DeletableStates() const;

	virtual bool DeleteState(quint8 state);

	virtual void DoorOpLocs(AuPoint& opLoc1, AuPoint& opLoc2) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual void FacesInHullSilhouette(AuScFaceList& faces) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool IsAutomaticDoor() const
	{
		return m_Access == DOOR_Automatic;
	}

	inline bool IsClosed() const
	{
		return m_State <= Open40 || m_State == StuckClosed || m_State == LockedClosed;
	}

	virtual bool IsDoor() const
	{
		return true;
	}

	virtual bool IsHullDoor() const
	{
		return m_Access == DOOR_Hull || m_Access == DOOR_HullLift;
	}

	virtual bool IsInitiallyLocked() const
	{
		return m_DoorBits & DOORBIT_InitLocked;
	}

	virtual bool IsInsideObstructionModel(const AuPoint& loc) const;

	virtual bool IsInsideObstructionModel(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	virtual bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	virtual bool IsPathBlocked(quint16 partId, const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Returns true for doors that are controlled using remote door switches in spacecraft.
	virtual bool IsRemoteSpacecraftDoor() const
	{
		return m_DoorBits & DOORBIT_RemoteSpacecraft;
	}

	// Returns true for doors that are controlled using remote door switches in vehicles.
	virtual bool IsRemoteVehicleDoor() const
	{
		return m_DoorBits & DOORBIT_RemoteVehicle;
	}

	// A door with a name is implicitly a turbo lift door.
	virtual bool IsTurboLiftDoor() const
	{
		return !m_StopName.isEmpty();
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual QString PartName() const;

	virtual void RefreshObstructionsCache(quint16 partId, const AuScParts& parts);

	virtual bool RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts);

	virtual void RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count], quint8 state) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual void SetDoorOpLocs(const AuPoint& switch1, const AuPoint& switch2);

	virtual void SetDoorProperties(Qt::CheckState isAutomatic, Qt::CheckState initLocked, Qt::CheckState remoteSpacecraft,
			Qt::CheckState remoteVehicle);

	virtual void SetHullDoor(bool hullDoor);

	virtual void SetStopName(const QString& name);

	virtual void SetState(quint8 state)
	{
		if(state < STATES_Count)
			AuScStatePart::SetState(state);
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual QString StateName(quint8 state) const;

	virtual QMap<quint8, QString> StateNames() const;

	virtual QString StopName() const
	{
		return m_StopName;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Door;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScDoor& c = dynamic_cast<const AuScDoor&>(part);
			m_Access = c.m_Access;
			m_DoorBits = c.m_DoorBits;
			m_OpLoc1 = c.m_OpLoc1;
			m_OpLoc2 = c.m_OpLoc2;
			m_Silhouette = c.m_Silhouette;
			m_StopName = c.m_StopName;
			AuScStatePart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScWindow : public AuScMeshBase
{
	AuScSilhouette3d					m_Silhouette;			// Used by IsPathBlocked test.

public:	// Constructors
	AuScWindow();
	AuScWindow(const AuScWindow& c);
	AuScWindow(const AuScWindow& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// Windows can only be joined to other windows.
		return dynamic_cast<const AuScWindow*>(joinTarget);
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void FacesInHullSilhouette(AuScFaceList& faces) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool IsInsideObstructionModel(const AuPoint& loc) const;

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual bool IsWindow() const
	{
		return true;
	}

	virtual bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual float Opacity() const;

	virtual void RefreshObstructionsCache(quint16 partId, const AuScParts& parts);

	virtual bool RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts);

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual void SetOpacity(float o);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Window;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScWindow& c = dynamic_cast<const AuScWindow&>(part);
			m_Silhouette = c.m_Silhouette;
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScLandingGear : public AuScStatePart
{
public:
	enum STATES
	{
		Extended,						// Gear fully extended.
		Retracted10,					// Gear 10% retracted.
		Retracted20,					// Gear 20% retracted.
		Retracted30,					// Gear 30% retracted.
		Retracted40,					// Gear 40% retracted.
		Retracted50,					// Gear 50% retracted.
		Retracted60,					// Gear 60% retracted.
		Retracted70,					// Gear 70% retracted.
		Retracted80,					// Gear 80% retracted.
		Retracted90,					// Gear 90% retracted.
		Retracted,						// Gear fully retracted.
		StuckExtended,					// Gear extended and stuck.
		StuckRetracted,					// Gear retracted and stuck.
		STATES_Count					// LAST ITEM
	};

private:
	QHash<quint8, AuScSilhouette3d>		m_StateSilhouettes;		// Used by IsPathBlocked test.

public:	// Constructors
	AuScLandingGear();
	AuScLandingGear(const AuScLandingGear& c);
	AuScLandingGear(const AuScLandingGear& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual QMap<quint8, QString> AddableStates() const;

	// Adds a new empty state mesh if it does not already exist.
	// Returns true if any changes are made.
	virtual bool AddState(quint8 state);

	virtual bool CanAddState() const
	{
		return StateCount() < STATES_Count;
	}

	virtual bool CanDeleteState() const
	{
		// We don't permit state 0 to be deleted.
		return StateCount() > 1;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// Landing gear cannot be joined to anything.
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual QMap<quint8, QString> DeletableStates() const;

	virtual bool DeleteState(quint8 state);

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool HullExtents(quint16 partId, AuPoint& min, AuPoint& max, const AuLandingGearStateHash& landingGearStates) const;

	inline static bool IsExtended(STATES state)
	{
		switch(state)
		{
		case Extended:
		case StuckExtended:
			return true;
		case Retracted10:
		case Retracted20:
		case Retracted30:
		case Retracted40:
		case Retracted50:
		case Retracted60:
		case Retracted70:
		case Retracted80:
		case Retracted90:
		case Retracted:
		case StuckRetracted:
		case STATES_Count:
			break;
		}
		return false;
	}

	virtual bool IsInsideObstructionModel(const AuPoint& loc) const;

	virtual bool IsInsideObstructionModel(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	virtual bool IsLandingGear() const
	{
		return true;
	}

	virtual bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	virtual bool IsPathBlocked(quint16 partId, const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	// Landing gear is assembled separately.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual void RefreshObstructionsCache(quint16 partId, const AuScParts& parts);

	virtual bool RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts);

	virtual void RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count], quint8 state) const;

	virtual void SetState(quint8 state)
	{
		if(state < STATES_Count)
			AuScStatePart::SetState(state);
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual QString StateName(quint8 state) const;

	virtual QMap<quint8, QString> StateNames() const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_LandingGear;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScLandingGear& c = dynamic_cast<const AuScLandingGear&>(part);

			m_StateSilhouettes = c.m_StateSilhouettes;

			AuScStatePart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScLift : public AuScStatePart
{
public:
	enum STATES
	{
		Car,							// Car mesh.
		DoorJig,						// Door jig mesh.
		CarOriginal,					// Needed for jig cutting this thing more than once. Hidden from the user.
		RoomVoid,						// Room void inside the car mesh.
		STATES_Count					// LAST ITEM
	};

	quint8								m_GravityType;			// GRAVITY enum value.
	AuPoint								m_GravityUp;			// Gravity up vector when gravity is provided artificially.
	AuScPathSegList						m_PathSegments;			// Created when finalized. Interconnects all stops.
	QString								m_Stop;					// Empty is the editable state, at its natural position.
	QMap<QString, AuMatrix4x4>			m_Stops;				// Destinations for the car.

public:	// Constructors
	AuScLift();
	AuScLift(const AuScLift& c);
	AuScLift(const AuScLift& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual QMap<quint8, QString> AddableStates() const;

	virtual void AddFaces(const AuScFaceList& faces);

	virtual bool AddState(quint8 state);

	virtual bool AddStop(const QString& stop);

	virtual bool CanAddState() const
	{
		return StateCount() < STATES_Count;
	}

	virtual bool CanAddStop() const;

	virtual bool CanChangeStop() const;

	virtual bool CanDeleteStop() const;

	virtual bool CanDeleteState() const
	{
		// We don't permit state 0 to be deleted.
		return StateCount() > 1;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// Lifts can only be joined to a lift mesh.
		return dynamic_cast<const AuScLift*>(joinTarget);
	}

	virtual bool CheckVoidFacesPlanar() const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool CutFaces(const AuPointArray& cutLine);

	virtual QMap<quint8, QString> DeletableStates() const;

	virtual QStringList DeletableStops() const;

	virtual bool DeleteState(quint8 state);

	virtual bool DeleteStop(const QString& stop);

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual AuPoint FaceLoc(const AuPoint& loc, const AuPoint& vec, AuPoint& terN, const AuMatrix4x4& stopM) const;

	virtual void Faces(AuScFaceList& faces) const;

	virtual void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const;

	virtual void FacesInHullSilhouette(AuScFaceList& faces) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual GRAVITY GravityType() const
	{
		return (GRAVITY)m_GravityType;
	}

	virtual AuPoint GravityUp(const AuMatrix4x4& stopM) const;

	virtual AuPoint GravityUp(const AuPoint& gravityN, const AuMatrix4x4& stopM) const;

	virtual double HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM, AuPoint& floorN) const;

	virtual double HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	inline bool IsAtDesignStop() const
	{
		return m_Stop.isEmpty() || !m_Stops.contains(m_Stop);
	}

	// Returns true for all jigs.
	virtual bool IsJig() const
	{
		// A lift is only a jig if it has a door jig and if it has a current stop.
		return HasState(DoorJig) && !IsAtDesignStop();
	}

	virtual bool IsInsideRoomVoid(const AuPoint& loc, const AuScParts& parts) const;

	virtual bool IsInsideRoomVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool IsInsideRoomVoid(const AuPoint& loc, double elevate, const AuScParts& parts) const;

	virtual bool IsInsideRoomVoid(quint16 partId, const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool IsInsideVoid(const AuPoint& loc) const;

	virtual bool IsInsideVoid(const AuPoint& loc, const AuScParts& parts) const;

	virtual bool IsInsideVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool IsInsideVoidEver(const AuPoint& loc) const;

	virtual bool IsRoomVoid() const
	{
		return HasState(RoomVoid);
	}

	virtual bool IsTurboLiftMesh() const;

	/* This is the wrong answer for all the cases IsVoid is currently called.
	virtual bool IsVoid() const
	{
		return HasState(RoomVoid);
	}
	*/

	virtual void JigCutFaces(AuScFaceList& faces) const;

	virtual void JigFaces(AuScFaceList& jigFaces) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	void MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps);

	void MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps, const AuMatrix4x4& gridPlane);

	void MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
			const AuMatrix4x4& gridPlane);

	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane);

	void MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides);

	void MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides);

	virtual quint64 MemoryUsage() const;

	virtual const AuScPathSeg& PathSegment(int segmentIndex) const
	{
		if(segmentIndex >= m_PathSegments.size())
			return AuScPathSeg::Null();
		return m_PathSegments.at(segmentIndex);
	}

	virtual int PathSegments() const
	{
		return m_PathSegments.size();
	}

	virtual bool PunchFaces(const AuPointArray& cutBnd);

	virtual bool RenameStop(const QString& oldStopName, const QString& nuStopName);

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual void SetFaces(const AuScFaceList& faces);

	virtual void SetGravity(GRAVITY g, const AuPoint& up)
	{
		m_GravityType = g;
		m_GravityUp = up;
	}

	virtual void SetJigCutFaces(const AuScFaceList& faces);

	virtual void SetState(quint8 state)
	{
		if(state < STATES_Count)
			AuScStatePart::SetState(state);
	}

	// Sets the current stop position.
	// The empty stop position is the natural model position for editing. We only edit it in that position.
	virtual void SetStop(const QString& stop);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual QString StateName(quint8 state) const;

	virtual QMap<quint8, QString> StateNames() const;

	virtual const AuMatrix4x4& StopMatrixAt(const QString& stopName) const
	{
		QMap<QString, AuMatrix4x4>::const_iterator ti = m_Stops.find(stopName);
		if(ti == m_Stops.cend())
			return AuMatrix4x4::IdentityMatrix();
		return *ti;
	}

	virtual const AuMatrix4x4& StopMatrixAt(quint16 partId, const AuLiftHash& liftStops) const;

	virtual const AuMatrix4x4& StopMatrixDesign() const
	{
		return StopMatrixAt(m_Stop);
	}

	virtual QStringList StopNames() const;

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformLiftStop(const AuMatrix4x4& t);

	virtual QString AtStop() const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Lift;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScLift& c = dynamic_cast<const AuScLift&>(part);

			m_GravityType = c.m_GravityType;
			m_GravityUp = c.m_GravityUp;
			m_PathSegments = c.m_PathSegments;
			m_Stop = c.m_Stop;
			m_Stops = c.m_Stops;

			AuScStatePart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScScreen : public AuScPart
{
protected:
	AuScFaceQuad						m_Face;

public:	// Constructors
	AuScScreen();
	AuScScreen(const AuScScreen& c);
	AuScScreen(const AuScScreen& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void AddFaces(const AuScFaceList& faces);

	virtual bool CanFracture() const
	{
		return false;
	}

	virtual bool CanFuse() const
	{
		return false;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// A screen cannot be joined to any other mesh.
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual bool CanMakeIntoOtherPart() const
	{
		return false;
	}

	virtual bool CutFaces(const AuPointArray& cutLine)
	{
		Q_UNUSED(cutLine);
		return false;
	}

	virtual AuPoint Center() const;

	virtual void DeleteFaces(const QSet<quint32>& faceIds);

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeIds(QSet<quint32>& edgeIds) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const;

	virtual void EffectFaces(SURFACE_EFFECT effect);

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual void FaceIds(QSet<quint32>& faceIds) const;

	virtual void Faces(AuScFaceList& faces) const;

	virtual void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual void FlattenVertices();

	virtual void FlattenVertices(const AuScSelectInfo& selInfo);

	virtual bool HasOwnTexture() const
	{
		return true;
	}

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	virtual bool IsCommandStation() const
	{
		return AuStation::IsCommand(StationType());
	}

	virtual bool IsControlStation() const
	{
		return AuStation::IsControl(StationType());
	}

	virtual bool IsDecal() const
	{
		return true;
	}

	virtual bool IsValid() const;

	// Command and control stations are handled separately from the main model.
	virtual bool IsVisibleInFinalModel() const
	{
		return !IsCommandStation() && !IsControlStation();
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	void MakeScreen(const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane);

	virtual quint64 MemoryUsage() const;

	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual bool PunchFaces(const AuPointArray& cutBnd)
	{
		Q_UNUSED(cutBnd);
		return false;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void ReverseFaces();

	virtual void ReverseFaces(const AuScSelectInfo& selInfo);

	virtual void ReverseVertexNormals();

	virtual bool ScreenQuad(AuQuad& screen) const;

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void SetFaceOpacity(float opacity);

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity);

	virtual void SetFaces(const AuScFaceList& faces);

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void SmoothVertices();

	virtual void SmoothVertices(const AuScSelectInfo& selInfo);

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex);

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex);

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const;

	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScScreen& c = dynamic_cast<const AuScScreen&>(part);

			m_Face = c.m_Face;

			AuScPart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexIds(QSet<quint32>& vertexIds) const;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStatusAlert : public AuScScreen
{
public:	// Constructors
	AuScStatusAlert();
	AuScStatusAlert(const AuScStatusAlert& c);
	AuScStatusAlert(const AuScStatusAlert& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual PART_SOUND Sound() const
	{
		return SOUND_Siren;
	}

	virtual AuPoint SoundEmitLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StatusAlert;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStatusAlert& c = dynamic_cast<const AuScStatusAlert&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStatusLifeSupport : public AuScScreen
{
	quint16								m_PartId;				// Associated room void.

public:	// Constructors
	AuScStatusLifeSupport();
	AuScStatusLifeSupport(const AuScStatusLifeSupport& c);
	AuScStatusLifeSupport(const AuScStatusLifeSupport& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual quint16 AssociatedPartId() const
	{
		return m_PartId;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		Q_UNUSED(partState);
		m_PartId = partId;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StatusLifeSupport;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStatusLifeSupport& c = dynamic_cast<const AuScStatusLifeSupport&>(part);

			m_PartId = c.m_PartId;

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStatusMission : public AuScScreen
{
public:	// Constructors
	AuScStatusMission();
	AuScStatusMission(const AuScStatusMission& c);
	AuScStatusMission(const AuScStatusMission& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StatusMission;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStatusMission& c = dynamic_cast<const AuScStatusMission&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelCapacitor : public AuScScreen
{
public:	// Constructors
	AuScPanelCapacitor();
	AuScPanelCapacitor(const AuScPanelCapacitor& c);
	AuScPanelCapacitor(const AuScPanelCapacitor& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelCapacitor;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelCapacitor& c = dynamic_cast<const AuScPanelCapacitor&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelFTLDrive : public AuScScreen
{
	quint16								m_PartId;				// Associated ftl drive station.

public:	// Constructors
	AuScPanelFTLDrive();
	AuScPanelFTLDrive(const AuScPanelFTLDrive& c);
	AuScPanelFTLDrive(const AuScPanelFTLDrive& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual quint16 AssociatedPartId() const
	{
		return m_PartId;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		Q_UNUSED(partState);
		m_PartId = partId;
	}

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_SOUND Sound() const
	{
		return SOUND_FTLDrive;
	}

	virtual AuPoint SoundEmitLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelFTLDrive;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelFTLDrive& c = dynamic_cast<const AuScPanelFTLDrive&>(part);

			m_PartId = c.m_PartId;

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelLifeSupport : public AuScScreen
{
public:	// Constructors
	AuScPanelLifeSupport();
	AuScPanelLifeSupport(const AuScPanelLifeSupport& c);
	AuScPanelLifeSupport(const AuScPanelLifeSupport& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelLifeSupport;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelLifeSupport& c = dynamic_cast<const AuScPanelLifeSupport&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelManeuverDrive : public AuScScreen
{
public:	// Constructors
	AuScPanelManeuverDrive();
	AuScPanelManeuverDrive(const AuScPanelManeuverDrive& c);
	AuScPanelManeuverDrive(const AuScPanelManeuverDrive& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_SOUND Sound() const
	{
		return SOUND_ManeuverDrive;
	}

	virtual AuPoint SoundEmitLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelManeuverDrive;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelManeuverDrive& c = dynamic_cast<const AuScPanelManeuverDrive&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelPowerPlant : public AuScScreen
{
public:	// Constructors
	AuScPanelPowerPlant();
	AuScPanelPowerPlant(const AuScPanelPowerPlant& c);
	AuScPanelPowerPlant(const AuScPanelPowerPlant& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_SOUND Sound() const
	{
		return SOUND_PowerPlant;
	}

	virtual AuPoint SoundEmitLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelPowerPlant;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelPowerPlant& c = dynamic_cast<const AuScPanelPowerPlant&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelSensor : public AuScScreen
{
public:	// Constructors
	AuScPanelSensor();
	AuScPanelSensor(const AuScPanelSensor& c);
	AuScPanelSensor(const AuScPanelSensor& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_SOUND Sound() const
	{
		return SOUND_Sensor;
	}

	virtual AuPoint SoundEmitLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelSensor;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelSensor& c = dynamic_cast<const AuScPanelSensor&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelShield : public AuScScreen
{
public:	// Constructors
	AuScPanelShield();
	AuScPanelShield(const AuScPanelShield& c);
	AuScPanelShield(const AuScPanelShield& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelShield;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelShield& c = dynamic_cast<const AuScPanelShield&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPanelWeapon : public AuScScreen
{
	quint16								m_PartId;				// Associated ftl drive station.

public:	// Constructors
	AuScPanelWeapon();
	AuScPanelWeapon(const AuScPanelWeapon& c);
	AuScPanelWeapon(const AuScPanelWeapon& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual quint16 AssociatedPartId() const
	{
		return m_PartId;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsServicePanel() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		Q_UNUSED(partState);
		m_PartId = partId;
	}

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PanelWeapon;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPanelWeapon& c = dynamic_cast<const AuScPanelWeapon&>(part);

			m_PartId = c.m_PartId;

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStation : public AuScScreen
{
public:
	enum OPERATOR_POSITION
	{
		Operator_Sit,					// Operator can turn in Z, and look up/down, e.g. swivel chair or old console/screen combo.
		Operator_SitSwivel3D,			// Operator's body turns in all 3D directions, perhaps a turret position.
		Operator_SitStationary,			// Operator cannot turn body but can turn head and look up/down.
		Operator_Stand,					// Operator stands, can turn in Z and look up/down..
		Operator_StandStationary,		// Operator stands, cannot turn body but can turn head and look up/down..
	};

private:
	AuPoint								m_Location;				// Operator location.
	quint16								m_Module;				// COMMODITYID of associated module, if any.
	AuPoint								m_OperatorFacing;		// Location used to determine operator rotation. Faces screen if
																// operator facing equals operator location.
	quint8								m_Position;				// OPERATOR_POSITION

public:	// Constructors
	AuScStation();
	AuScStation(const AuScStation& c);
	AuScStation(const AuScStation& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void EdgeSegments(AuPointArray& edges) const;

	void MakeStation(OPERATOR_POSITION pos, const AuPoint& loc, const AuPoint& opFacing, const AuPoint& corner1,
			const AuPoint& corner2, const AuMatrix4x4& gridPlane);

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	virtual AuPoint OperatorLoc() const
	{
		return m_Location;
	}

	virtual AuStation OperatorStation(const AuPoint& gravityUp) const;

	inline OPERATOR_POSITION OperatorPos() const
	{
		return (OPERATOR_POSITION)m_Position;
	}

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual QString ReportDesc() const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual void SetModule(COMMODITYID cid)
	{
		m_Module = cid;
	}

	// Pass 'opFacing' == 'loc' to face the screen.
	inline void SetOperator(OPERATOR_POSITION pos, const AuPoint& loc, const AuPoint& opFacing)
	{
		m_Location = loc;
		m_OperatorFacing = opFacing;
		m_Position = pos;
	}

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStation& c = dynamic_cast<const AuScStation&>(part);

			m_Module = c.m_Module;
			m_Position = c.m_Position;
			m_Location = c.m_Location;
			m_OperatorFacing = c.m_OperatorFacing;

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationCommand : public AuScStation
{
public:	// Constructors
	AuScStationCommand();
	AuScStationCommand(const AuScStationCommand& c);
	AuScStationCommand(const AuScStationCommand& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Command;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationCommand;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationCommand& c = dynamic_cast<const AuScStationCommand&>(part);

			Q_UNUSED(c);

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationDesigner : public AuScStation
{
public:	// Constructors
	AuScStationDesigner();
	AuScStationDesigner(const AuScStationDesigner& c);
	AuScStationDesigner(const AuScStationDesigner& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Designer;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationDesigner;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationDesigner& c = dynamic_cast<const AuScStationDesigner&>(part);

			Q_UNUSED(c);

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationEngineer : public AuScStation
{
	quint8								m_WhichEffect;			// Which of the appropriate EFF_ surface effects to use.

public:	// Constructors
	AuScStationEngineer();
	AuScStationEngineer(const AuScStationEngineer& c);
	AuScStationEngineer(const AuScStationEngineer& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual QString PartName() const;

	virtual void SetWhichEffect(int which)
	{
		if(which < 0 || which > 3)
			return;
		m_WhichEffect = which;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Engine;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationEngineer;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationEngineer& c = dynamic_cast<const AuScStationEngineer&>(part);
			m_WhichEffect = c.m_WhichEffect;
			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual int WhichEffect() const
	{
		return m_WhichEffect;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationFireControl : public AuScStation
{
	AuPoint								m_EmitLoc;				// Location for omnidirectional fire emission of weapon.
	quint8								m_WhichEffect;			// Which of the appropriate EFF_ surface effects to use.

public:	// Constructors
	AuScStationFireControl();
	AuScStationFireControl(const AuScStationFireControl& c);
	AuScStationFireControl(const AuScStationFireControl& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	inline const AuPoint& EmitLoc() const
	{
		return m_EmitLoc;
	}

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual QString PartName() const;

	virtual void SetEmitLoc(const AuPoint& emitLoc)
	{
		m_EmitLoc = emitLoc;
	}

	virtual void SetWhichEffect(int which)
	{
		if(which < 0 || which > 9)
			return;
		m_WhichEffect = which;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_FireControl;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationFireControl;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationFireControl& c = dynamic_cast<const AuScStationFireControl&>(part);
			m_EmitLoc = c.m_EmitLoc;
			m_WhichEffect = c.m_WhichEffect;
			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual AuPoint WeaponEmitLoc() const
	{
		return m_EmitLoc;
	}

	virtual int WhichEffect() const
	{
		return m_WhichEffect;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationHelm : public AuScStation
{
public:	// Constructors
	AuScStationHelm();
	AuScStationHelm(const AuScStationHelm& c);
	AuScStationHelm(const AuScStationHelm& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsHelm() const
	{
		return true;
	}

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Helm;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationHelm;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationHelm& c = dynamic_cast<const AuScStationHelm&>(part);

			Q_UNUSED(c);

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationMedic : public AuScStation
{
private:
	AuPoint								m_PatientFacing;		// Location used to determine patient rotation.
	AuPoint								m_PatientLoc;
	quint8								m_PatientPos;

public:	// Constructors
	AuScStationMedic();
	AuScStationMedic(const AuScStationMedic& c);
	AuScStationMedic(const AuScStationMedic& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation ClosestStation(const AuPoint& opLoc, const AuPoint& gravityUp) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual const AuPoint PatientLoc() const
	{
		return m_PatientLoc;
	}

	virtual PATIENT_POSITION PatientPos() const
	{
		return (PATIENT_POSITION)m_PatientPos;
	}

	virtual AuStation PatientStation(const AuPoint& gravityUp) const;

	inline void SetPatient(PATIENT_POSITION pos, const AuPoint& loc, const AuPoint& facing)
	{
		m_PatientFacing = facing;
		m_PatientLoc = loc;
		m_PatientPos = pos;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Medic;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationMedic;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationMedic& c = dynamic_cast<const AuScStationMedic&>(part);

			m_PatientFacing = c.m_PatientFacing;
			m_PatientLoc = c.m_PatientLoc;
			m_PatientPos = c.m_PatientPos;

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationNavigator : public AuScStation
{
public:	// Constructors
	AuScStationNavigator();
	AuScStationNavigator(const AuScStationNavigator& c);
	AuScStationNavigator(const AuScStationNavigator& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Navigation;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationNavigator;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationNavigator& c = dynamic_cast<const AuScStationNavigator&>(part);

			Q_UNUSED(c);

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationPowerRelay : public AuScStation
{
public:	// Constructors
	AuScStationPowerRelay();
	AuScStationPowerRelay(const AuScStationPowerRelay& c);
	AuScStationPowerRelay(const AuScStationPowerRelay& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Power;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationPowerRelay;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationPowerRelay& c = dynamic_cast<const AuScStationPowerRelay&>(part);

			Q_UNUSED(c);

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationSensor : public AuScStation
{
public:	// Constructors
	AuScStationSensor();
	AuScStationSensor(const AuScStationSensor& c);
	AuScStationSensor(const AuScStationSensor& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsSensorStation() const
	{
		return true;
	}

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual PART_SOUND Sound() const
	{
		return SOUND_Sensor;
	}

	virtual AuPoint SoundEmitLoc() const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Sensor;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationSensor;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationSensor& c = dynamic_cast<const AuScStationSensor&>(part);

			Q_UNUSED(c);

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationShield : public AuScStation
{
public:	// Constructors
	AuScStationShield();
	AuScStationShield(const AuScStationShield& c);
	AuScStationShield(const AuScStationShield& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsShieldStation() const
	{
		return true;
	}

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Shield;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationShield;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationShield& c = dynamic_cast<const AuScStationShield&>(part);

			Q_UNUSED(c);

			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStationTransporter : public AuScStation
{
private:
	AuPointArray						m_PadLocs;
	quint8								m_WhichEffect;			// Which of the appropriate EFF_ surface effects to use.

public:	// Constructors
	AuScStationTransporter();
	AuScStationTransporter(const AuScStationTransporter& c);
	AuScStationTransporter(const AuScStationTransporter& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual QString PartName() const;

	virtual void PadLocs(AuPointArray& padLocs) const
	{
		padLocs = m_PadLocs;
	}

	virtual void SetPadLocs(const AuPointArray& padLocs)
	{
		m_PadLocs = padLocs;

		// Insure we don't exceed 255 for our stream operators.
		if(m_PadLocs.size() > 255)
			m_PadLocs.resize(255);
	}

	virtual void SetWhichEffect(int which)
	{
		if(which < 0 || which > 9)
			return;
		m_WhichEffect = which;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Integrated_Transporter;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StationTransporter;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStationTransporter& c = dynamic_cast<const AuScStationTransporter&>(part);
			m_PadLocs = c.m_PadLocs;
			m_WhichEffect = c.m_WhichEffect;
			AuScStation::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual int WhichEffect() const
	{
		return m_WhichEffect;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Location represented by an icon.

class AUDATA_DLL AuScIcon : public AuScScreen
{
public:	// Constructors
	AuScIcon();
	AuScIcon(const AuScIcon& c);
	AuScIcon(const AuScIcon& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	// Bottom = -Y.
	virtual double IconBack() const = 0;

	// Fore = +Y.
	virtual double IconFore() const = 0;

	// Left = -X.
	virtual double IconLeft() const = 0;

	// Right = +X.
	virtual double IconRight() const = 0;

	// Most icons are not visible in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	// Icon is created centered at the origin of 'atPlane', and rotated to 'atPlane'.
	// Forward on the icon is considered to be its +Y axis edge. It will be the second edge in m_Face.
	void MakeIcon(const AuMatrix4x4& atPlane);

	// Icon is created centered at 'loc' and parallel to 'gridPlane'. The icon is rotated 'rot' in Z in its plane.
	void MakeIcon(const AuPoint& loc, float rot, const AuMatrix4x4& gridPlane);

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScIcon& c = dynamic_cast<const AuScIcon&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Console represents the point location of an old style swiveling chair/screen combo.
// The "screen" is only depicted as an icon to show it's location as it is created using a 3DS model.

class AUDATA_DLL AuScConsole : public AuScIcon
{
	quint16								m_Module;				// COMMODITYID of associated module, if any.

public:	// Constructors
	AuScConsole();
	AuScConsole(const AuScConsole& c);
	AuScConsole(const AuScConsole& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool IsConsole() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	virtual AuPoint OperatorLoc() const;

	virtual AuStation OperatorStation(const AuPoint& gravityUp) const;

	virtual AuMatrix4x4 PositionMatrix() const;

	virtual QString ReportDesc() const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual void SetModule(COMMODITYID cid)
	{
		m_Module = cid;
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsole& c = dynamic_cast<const AuScConsole&>(part);

			m_Module = c.m_Module;

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleCommand : public AuScConsole
{
public:	// Constructors
	AuScConsoleCommand();
	AuScConsoleCommand(const AuScConsoleCommand& c);
	AuScConsoleCommand(const AuScConsoleCommand& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Command;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleCommand;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleCommand& c = dynamic_cast<const AuScConsoleCommand&>(part);

			Q_UNUSED(c);

			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleEngineer : public AuScConsole
{
	quint8								m_WhichEffect;			// Which of the appropriate EFF_ surface effects to use.

public:	// Constructors
	AuScConsoleEngineer();
	AuScConsoleEngineer(const AuScConsoleEngineer& c);
	AuScConsoleEngineer(const AuScConsoleEngineer& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual quint64 MemoryUsage() const;

	virtual QString PartName() const;

	virtual void SetWhichEffect(int which)
	{
		if(which < 0 || which > 3)
			return;
		m_WhichEffect = which;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Engine;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleEngineer;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleEngineer& c = dynamic_cast<const AuScConsoleEngineer&>(part);
			m_WhichEffect = c.m_WhichEffect;
			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual int WhichEffect() const
	{
		return m_WhichEffect;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleFireControl : public AuScConsole
{
	AuPoint								m_EmitLoc;				// Location for omnidirectional fire emission of weapon.
	quint8								m_WhichEffect;			// Which of the appropriate EFF_ surface effects to use.


public:	// Constructors
	AuScConsoleFireControl();
	AuScConsoleFireControl(const AuScConsoleFireControl& c);
	AuScConsoleFireControl(const AuScConsoleFireControl& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	inline const AuPoint& EmitLoc() const
	{
		return m_EmitLoc;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString PartName() const;

	virtual void SetEmitLoc(const AuPoint& emitLoc)
	{
		m_EmitLoc = emitLoc;
	}

	virtual void SetWhichEffect(int which)
	{
		if(which < 0 || which > 9)
			return;
		m_WhichEffect = which;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_FireControl;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleFireControl;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleFireControl& c = dynamic_cast<const AuScConsoleFireControl&>(part);
			m_EmitLoc = c.m_EmitLoc;
			m_WhichEffect = c.m_WhichEffect;
			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual AuPoint WeaponEmitLoc() const
	{
		return m_EmitLoc;
	}

	virtual int WhichEffect() const
	{
		return m_WhichEffect;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleHelm : public AuScConsole
{
public:	// Constructors
	AuScConsoleHelm();
	AuScConsoleHelm(const AuScConsoleHelm& c);
	AuScConsoleHelm(const AuScConsoleHelm& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsHelm() const
	{
		return true;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Helm;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleHelm;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleHelm& c = dynamic_cast<const AuScConsoleHelm&>(part);

			Q_UNUSED(c);

			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleMedic : public AuScConsole
{
private:
	AuPoint								m_PatientFacing;		// Location used to determine patient rotation.
	AuPoint								m_PatientLoc;
	quint8								m_PatientPos;

public:	// Constructors
	AuScConsoleMedic();
	AuScConsoleMedic(const AuScConsoleMedic& c);
	AuScConsoleMedic(const AuScConsoleMedic& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation ClosestStation(const AuPoint& opLoc, const AuPoint& gravityUp) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual const AuPoint PatientLoc() const
	{
		return m_PatientLoc;
	}

	virtual PATIENT_POSITION PatientPos() const
	{
		return (PATIENT_POSITION)m_PatientPos;
	}

	virtual AuStation PatientStation(const AuPoint& gravityUp) const;

	inline void SetPatient(PATIENT_POSITION pos, const AuPoint& loc, const AuPoint& facing)
	{
		m_PatientFacing = facing;
		m_PatientLoc = loc;
		m_PatientPos = pos;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Medic;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleMedic;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleMedic& c = dynamic_cast<const AuScConsoleMedic&>(part);

			m_PatientFacing = c.m_PatientFacing;
			m_PatientLoc = c.m_PatientLoc;
			m_PatientPos = c.m_PatientPos;

			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleNavigator : public AuScConsole
{
public:	// Constructors
	AuScConsoleNavigator();
	AuScConsoleNavigator(const AuScConsoleNavigator& c);
	AuScConsoleNavigator(const AuScConsoleNavigator& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Navigation;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleNavigator;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleNavigator& c = dynamic_cast<const AuScConsoleNavigator&>(part);

			Q_UNUSED(c);

			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsolePowerRelay : public AuScConsole
{
public:	// Constructors
	AuScConsolePowerRelay();
	AuScConsolePowerRelay(const AuScConsolePowerRelay& c);
	AuScConsolePowerRelay(const AuScConsolePowerRelay& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Power;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsolePowerRelay;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsolePowerRelay& c = dynamic_cast<const AuScConsolePowerRelay&>(part);

			Q_UNUSED(c);

			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleSensor : public AuScConsole
{
public:	// Constructors
	AuScConsoleSensor();
	AuScConsoleSensor(const AuScConsoleSensor& c);
	AuScConsoleSensor(const AuScConsoleSensor& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsSensorStation() const
	{
		return true;
	}

	virtual PART_SOUND Sound() const
	{
		return SOUND_Sensor;
	}

	virtual AuPoint SoundEmitLoc() const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Sensor;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleSensor;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleSensor& c = dynamic_cast<const AuScConsoleSensor&>(part);

			Q_UNUSED(c);

			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleShield : public AuScConsole
{
public:	// Constructors
	AuScConsoleShield();
	AuScConsoleShield(const AuScConsoleShield& c);
	AuScConsoleShield(const AuScConsoleShield& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsShieldStation() const
	{
		return true;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Shield;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleShield;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleShield& c = dynamic_cast<const AuScConsoleShield&>(part);

			Q_UNUSED(c);

			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScConsoleTransporter : public AuScConsole
{
private:
	AuPointArray						m_PadLocs;
	quint8								m_WhichEffect;			// Which of the appropriate EFF_ surface effects to use.

public:	// Constructors
	AuScConsoleTransporter();
	AuScConsoleTransporter(const AuScConsoleTransporter& c);
	AuScConsoleTransporter(const AuScConsoleTransporter& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual void PadLocs(AuPointArray& padLocs) const
	{
		padLocs = m_PadLocs;
	}

	virtual QString PartName() const;

	virtual void SetPadLocs(const AuPointArray& padLocs)
	{
		m_PadLocs = padLocs;

		// Insure we don't exceed 255 for our stream operators.
		if(m_PadLocs.size() > 255)
			m_PadLocs.resize(255);
	}

	virtual void SetWhichEffect(int which)
	{
		if(which < 0 || which > 9)
			return;
		m_WhichEffect = which;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Console_Transporter;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ConsoleTransporter;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScConsoleTransporter& c = dynamic_cast<const AuScConsoleTransporter&>(part);
			m_PadLocs = c.m_PadLocs;
			m_WhichEffect = c.m_WhichEffect;
			AuScConsole::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual int WhichEffect() const
	{
		return m_WhichEffect;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A post is a place for a person to occupy. It iconically represents the location and body position required.

class AUDATA_DLL AuScPost : public AuScIcon
{
public:
	enum OCCUPANT_POSITION
	{
		Occupant_Stand,
		Occupant_Sit,
		Occupant_StandStationary,
		Occupant_SitStationary,
		Occupant_Prone,
	};

private:
	quint8								m_Position;

public:	// Constructors
	AuScPost();
	AuScPost(const AuScPost& c);
	AuScPost(const AuScPost& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual quint64 MemoryUsage() const;

	virtual AuPoint OperatorLoc() const;

	virtual AuStation OperatorStation(const AuPoint& gravityUp) const;

	inline OCCUPANT_POSITION Position() const
	{
		return (OCCUPANT_POSITION)m_Position;
	}

	inline void SetPosition(OCCUPANT_POSITION pos)
	{
		m_Position = pos;
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPost& c = dynamic_cast<const AuScPost&>(part);

			m_Position = c.m_Position;

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

/*
class AUDATA_DLL AuScSurgeryUnit : public AuScPost
{
public:	// Constructors
	AuScSurgeryUnit();
	AuScSurgeryUnit(const AuScSurgeryUnit& c);
	AuScSurgeryUnit(const AuScSurgeryUnit& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_SurgeryUnit_NeverUsed;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScSurgeryUnit& c = dynamic_cast<const AuScSurgeryUnit&>(part);

			Q_UNUSED(c);

			AuScPost::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScTransporterPad : public AuScPost
{
public:	// Constructors
	AuScTransporterPad();
	AuScTransporterPad(const AuScTransporterPad& c);
	AuScTransporterPad(const AuScTransporterPad& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_TransporterPad_NeverUsed;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScTransporterPad& c = dynamic_cast<const AuScTransporterPad&>(part);

			Q_UNUSED(c);

			AuScPost::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};
*/

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScTroopPost : public AuScPost
{
public:	// Constructors
	AuScTroopPost();
	AuScTroopPost(const AuScTroopPost& c);
	AuScTroopPost(const AuScTroopPost& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsGuardPost() const
	{
		return true;
	}

	virtual const AuPoint GuardLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Troop;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_TroopPost;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScTroopPost& c = dynamic_cast<const AuScTroopPost&>(part);

			Q_UNUSED(c);

			AuScPost::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScCitizenPost : public AuScPost
{
public:	// Constructors
	AuScCitizenPost();
	AuScCitizenPost(const AuScCitizenPost& c);
	AuScCitizenPost(const AuScCitizenPost& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsCitizenPost() const
	{
		return true;
	}

	virtual const AuPoint GuardLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Citizen;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_CitizenPost;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScCitizenPost& c = dynamic_cast<const AuScCitizenPost&>(part);

			Q_UNUSED(c);

			AuScPost::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScLivestockPost : public AuScPost
{
public:	// Constructors
	AuScLivestockPost();
	AuScLivestockPost(const AuScLivestockPost& c);
	AuScLivestockPost(const AuScLivestockPost& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool IsLivestockPost() const
	{
		return true;
	}

	virtual const AuPoint GuardLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Livestock;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_LivestockPost;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScLivestockPost& c = dynamic_cast<const AuScLivestockPost&>(part);

			Q_UNUSED(c);

			AuScPost::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScGroundVehicleParkingSpot : public AuScIcon
{
public:	// Constructors
	AuScGroundVehicleParkingSpot();
	AuScGroundVehicleParkingSpot(const AuScGroundVehicleParkingSpot& c);
	AuScGroundVehicleParkingSpot(const AuScGroundVehicleParkingSpot& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual quint16 GroundVehicleParkingSpots() const
	{
		return 1;
	}

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual AuMatrix4x4 PositionMatrix() const;

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_GroundVehicleParkingSpot;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScGroundVehicleParkingSpot& c = dynamic_cast<const AuScGroundVehicleParkingSpot&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScSpaceVehicleParkingSpot : public AuScIcon
{
public:	// Constructors
	AuScSpaceVehicleParkingSpot();
	AuScSpaceVehicleParkingSpot(const AuScSpaceVehicleParkingSpot& c);
	AuScSpaceVehicleParkingSpot(const AuScSpaceVehicleParkingSpot& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual AuMatrix4x4 PositionMatrix() const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual quint16 SpaceVehicleParkingSpots() const
	{
		return 1;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_SpaceVehicleParkingSpot;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScSpaceVehicleParkingSpot& c = dynamic_cast<const AuScSpaceVehicleParkingSpot&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScSpaceVehicleLauncher : public AuScIcon
{
	AuPoint								m_ExitLoc;				// Point the launcher exits the hull.

public:	// Constructors
	AuScSpaceVehicleLauncher();
	AuScSpaceVehicleLauncher(const AuScSpaceVehicleLauncher& c);
	AuScSpaceVehicleLauncher(const AuScSpaceVehicleLauncher& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool LaunchPosition(AuMatrix4x4& launchPadM, AuPoint& launchExit) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	inline void SetExitLoc(const AuPoint& loc)
	{
		m_ExitLoc = loc;
	}

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_SpaceVehicleLauncher;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScSpaceVehicleLauncher& c = dynamic_cast<const AuScSpaceVehicleLauncher&>(part);

			m_ExitLoc = c.m_ExitLoc;

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual const AuPoint VehicleExitLoc() const
	{
		return m_ExitLoc;
	}

	virtual const AuPoint VehicleParkLoc() const
	{
		return m_Face.FaceLocation();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScBerth : public AuScIcon
{
public:	// Constructors
	AuScBerth();
	AuScBerth(const AuScBerth& c);
	AuScBerth(const AuScBerth& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuPoint BerthLocation() const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool IsBerth() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual AuStation OperatorStation(const AuPoint& gravityUp) const;

	virtual AuSysId SightedSystem(quint16 partId) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScBerthCapt : public AuScBerth
{
public:	// Constructors
	AuScBerthCapt();
	AuScBerthCapt(const AuScBerthCapt& c);
	AuScBerthCapt(const AuScBerthCapt& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual quint16 CaptBerths() const
	{
		return 1;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsOnePerDesign() const
	{
		return true;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Berth_Captain;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_BerthCapt;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBerthCapt& c = dynamic_cast<const AuScBerthCapt&>(part);

			Q_UNUSED(c);

			AuScBerth::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScBerthCrew : public AuScBerth
{
public:	// Constructors
	AuScBerthCrew();
	AuScBerthCrew(const AuScBerthCrew& c);
	AuScBerthCrew(const AuScBerthCrew& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual quint16 CrewBerths() const
	{
		return 1;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Berth_Crew;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_BerthCrew;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBerthCrew& c = dynamic_cast<const AuScBerthCrew&>(part);

			Q_UNUSED(c);

			AuScBerth::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScBerthOfficer : public AuScBerth
{
public:	// Constructors
	AuScBerthOfficer();
	AuScBerthOfficer(const AuScBerthOfficer& c);
	AuScBerthOfficer(const AuScBerthOfficer& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual quint16 OfficerBerths() const
	{
		return 1;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Berth_Officer;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_BerthOfficer;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBerthOfficer& c = dynamic_cast<const AuScBerthOfficer&>(part);

			Q_UNUSED(c);

			AuScBerth::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScBerthPassenger : public AuScBerth
{
public:	// Constructors
	AuScBerthPassenger();
	AuScBerthPassenger(const AuScBerthPassenger& c);
	AuScBerthPassenger(const AuScBerthPassenger& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual quint16 PassengerBerths() const
	{
		return 1;
	}

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Berth_Passenger;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_BerthPassenger;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBerthPassenger& c = dynamic_cast<const AuScBerthPassenger&>(part);

			Q_UNUSED(c);

			AuScBerth::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScBerthTroop : public AuScBerth
{
public:	// Constructors
	AuScBerthTroop();
	AuScBerthTroop(const AuScBerthTroop& c);
	AuScBerthTroop(const AuScBerthTroop& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Berth_Troop;
	}

	virtual quint16 TroopBerths() const
	{
		return 1;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_BerthTroop;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBerthTroop& c = dynamic_cast<const AuScBerthTroop&>(part);

			Q_UNUSED(c);

			AuScBerth::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScGameTable : public AuScIcon
{
public:	// Constructors
	AuScGameTable();
	AuScGameTable(const AuScGameTable& c);
	AuScGameTable(const AuScGameTable& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual AuStation ClosestStation(const AuPoint& opLoc, const AuPoint& gravityUp) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool IsGameTable() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	// Returns the position of seat 0.
	virtual AuStation OperatorStation(const AuPoint& gravityUp) const;

	virtual AuMatrix4x4 PositionMatrix() const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Table_Game;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_GameTable;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScGameTable& c = dynamic_cast<const AuScGameTable&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScContrailEmitter : public AuScIcon
{
public:	// Constructors
	AuScContrailEmitter();
	AuScContrailEmitter(const AuScContrailEmitter& c);
	AuScContrailEmitter(const AuScContrailEmitter& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual float EmitterDiameter() const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool IsContrailEmitter() const
	{
		return true;
	}

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual PART_SOUND Sound() const
	{
		return SOUND_ManeuverDrive;
	}

	virtual AuPoint SoundEmitLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ContrailEmitter;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScContrailEmitter& c = dynamic_cast<const AuScContrailEmitter&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// The mesh is the lens of the light.

class AUDATA_DLL AuScHeadlightLens : public AuScMeshBase
{
public:	// Constructors
	AuScHeadlightLens();
	AuScHeadlightLens(const AuScHeadlightLens& c);
	AuScHeadlightLens(const AuScHeadlightLens& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual bool HasLightGlow() const
	{
		return true;
	}

	virtual bool IsLooselyAttachedToHull() const
	{
		return true;
	}

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual QColor LightColor() const;

	virtual AuPoint LightLoc() const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_HeadlightLens_Deprecated;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScHeadlightLens& c = dynamic_cast<const AuScHeadlightLens&>(part);

			Q_UNUSED(c);

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// The mesh is the lens of the light.

class AUDATA_DLL AuScLightLens : public AuScMeshBase
{
	QColor								m_Color;				// Color of the light behind the lens. Glow color.

public:	// Constructors
	AuScLightLens();
	AuScLightLens(const AuScLightLens& c);
	AuScLightLens(const AuScLightLens& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual bool HasLightGlow() const
	{
		return true;
	}

	// Returns true for light lens whose light color property can be changed.
	virtual bool IsLightLens() const
	{
		return true;
	}

	virtual bool IsLooselyAttachedToHull() const
	{
		return true;
	}

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual QColor LightColor() const;

	virtual AuPoint LightLoc() const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual void SetLightColor(const QColor& color)
	{
		m_Color = color;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_LightLens;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScLightLens& c = dynamic_cast<const AuScLightLens&>(part);
			m_Color = c.m_Color;
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// The mesh is the lens of the light.

class AUDATA_DLL AuScNavLightLens : public AuScMeshBase
{
private:
	QColor								m_Color;

public:	// Constructors
	AuScNavLightLens();
	AuScNavLightLens(const AuScNavLightLens& c);
	AuScNavLightLens(const AuScNavLightLens& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	inline const QColor& Color() const
	{
		return m_Color;
	}

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual bool HasLightGlow() const
	{
		return true;
	}

	virtual bool IsLooselyAttachedToHull() const
	{
		return true;
	}

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual QColor LightColor() const
	{
		return m_Color;
	}

	virtual AuPoint LightLoc() const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	inline void SetColor(const QColor& col)
	{
		m_Color = col;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_NavLightLens;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScNavLightLens& c = dynamic_cast<const AuScNavLightLens&>(part);
			m_Color = c.m_Color;
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// The mesh is the lens of the light.

class AUDATA_DLL AuScRotatingBeaconLens : public AuScMeshBase
{
private:
	QColor								m_Color;
	AuPoint								m_RotationAxis;

public:	// Constructors
	AuScRotatingBeaconLens();
	AuScRotatingBeaconLens(const AuScRotatingBeaconLens& c);
	AuScRotatingBeaconLens(const AuScRotatingBeaconLens& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	inline const QColor& Color() const
	{
		return m_Color;
	}

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual bool HasLightGlow() const
	{
		return true;
	}

	virtual bool IsLooselyAttachedToHull() const
	{
		return true;
	}

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual QColor LightColor() const
	{
		return m_Color;
	}

	virtual AuPoint LightLoc() const;

	virtual const AuPoint& LightRotationAxis() const
	{
		return m_RotationAxis;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	inline void SetColor(const QColor& col)
	{
		m_Color = col;
	}

	inline void SetLightRotationAxis(const AuPoint& axis)
	{
		// Insure we get a valid rotation axis.
		if(axis.IsZero())
			m_RotationAxis = AuPoint(0,0,1);
		else
		{
			m_RotationAxis = axis;
			m_RotationAxis.Unit();
		}
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_RotatingBeaconLens;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScRotatingBeaconLens& c = dynamic_cast<const AuScRotatingBeaconLens&>(part);
			m_Color = c.m_Color;
			m_RotationAxis = c.m_RotationAxis;
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// The mesh is the lens of the light.

class AUDATA_DLL AuScStrobeLens : public AuScMeshBase
{
private:
	QColor								m_Color;

public:	// Constructors
	AuScStrobeLens();
	AuScStrobeLens(const AuScStrobeLens& c);
	AuScStrobeLens(const AuScStrobeLens& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	inline const QColor& Color() const
	{
		return m_Color;
	}

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual bool HasLightGlow() const
	{
		return true;
	}

	virtual bool IsLooselyAttachedToHull() const
	{
		return true;
	}

	virtual bool IsOmitted() const
	{
		return AuScPart::IsOmitted();
	}

	virtual QColor LightColor() const
	{
		return m_Color;
	}

	virtual AuPoint LightLoc() const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	inline void SetColor(const QColor& col)
	{
		m_Color = col;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StrobeLens;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStrobeLens& c = dynamic_cast<const AuScStrobeLens&>(part);
			m_Color = c.m_Color;
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScTurret : public AuScStatePart
{
public:
	enum GUNNER_POSITION
	{
		Gunner_Sit,						// Gunner can turn in Z, and look up/down, e.g. swivel chair or old console/screen combo.
		Gunner_SitSwivel3D,				// Gunner's body turns in all 3D directions, perhaps a turret position.
		Gunner_SitStationary,			// Gunner cannot turn body but can turn head and look up/down.
		Gunner_Stand,					// Gunner stands, can turn in Z and look up/down..
		Gunner_StandStationary,			// Gunner stands, cannot turn body but can turn head and look up/down..
	};

	enum STATES
	{
		Base,							// The swiveling turret base.
		Gun,							// The gun that can be elevated.
		GunnerScreen,					// The screen for the gunner. Always only a quad.
		GunFire1,						// First frame of gun firing.
		GunFire2,						// 2nd frame of gun firing.
		GunFire3,						// 3rd frame of gun firing.
		GunFire4,						// 4th frame of gun firing.
		GunFire5,						// 5th frame of gun firing.
		GunFire6,						// 6th frame of gun firing.
		GunFire7,						// 7th frame of gun firing.
		GunFire8,						// 8th frame of gun firing.
		GunFire9,						// 9th frame of gun firing.
		GunFire10,						// Last frame of gun firing.
		GunReload1,						// First frame of gun reloading.
		GunReload2,						// 2nd frame of gun reloading.
		GunReload3,						// 3rd frame of gun reloading.
		GunReload4,						// 4th frame of gun reloading.
		GunReload5,						// 5th frame of gun reloading.
		GunReload6,						// 6th frame of gun reloading.
		GunReload7,						// 7th frame of gun reloading.
		GunReload8,						// 8th frame of gun reloading.
		GunReload9,						// 9th frame of gun reloading.
		GunReload10,					// Last frame of gun reloading.
		STATES_Count,
	};

protected:
	AuPoint								m_BaseAxis;				// Axis vector for pivot. Base pivots around the axis vector.
	AuPoint								m_BasePivot;			// Pivot point for turret.
	float								m_BasePivotMax;			// Max pivot angle around the axis vector.
	float								m_BasePivotMin;			// Min pivot angle around the axis vector.
	AuPoint								m_GunAxis;				// Axis vector for gun. Gun pivots around the axis vector.
	AuPoint								m_GunEmitLoc;			// Emit location for gun fire effect.
	AuPoint								m_GunEmitVec;			// Emit direction for gun fire effect.
	quint8								m_GunnerPosition;
	AuPoint								m_GunnerFacing;
	AuPoint								m_GunnerLocation;
	AuPoint								m_GunPivot;				// Pivot point for gun.
	float								m_GunPivotMax;			// Max pivot angle around the axis vector.
	float								m_GunPivotMin;			// Min pivot angle around the axis vector.
	AuPoint								m_GunSightLoc;			// Camera location for gun sight view.
	quint16								m_Module;				// COMMODITYID of associated module, if any.

public:	// Constructors
	AuScTurret();
	AuScTurret(const AuScTurret& c);
	AuScTurret(const AuScTurret& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual QMap<quint8, QString> AddableStates() const;

	// Adds a new empty state mesh if it does not already exist.
	// Returns true if any changes are made.
	virtual bool AddState(quint8 state);

	virtual const AuPoint& BaseAxis() const
	{
		return m_BaseAxis;
	}

	inline const AuPoint& BasePivot() const
	{
		return m_BasePivot;
	}

	inline float BasePivotMax() const
	{
		return m_BasePivotMax;
	}

	inline float BasePivotMin() const
	{
		return m_BasePivotMin;
	}

	virtual bool CanAddState() const
	{
		return StateCount() < STATES_Count;
	}

	// We don't permit the base to be deleted.
	virtual bool CanDeleteState() const;

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// Doors cannot be joined to anything.
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual QMap<quint8, QString> DeletableStates() const;

	virtual bool DeleteState(quint8 state);

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	inline const AuPoint& GunAxis() const
	{
		return m_GunAxis;
	}

	inline const AuPoint& GunEmitLoc() const
	{
		return m_GunEmitLoc;
	}

	inline const AuPoint& GunEmitVec() const
	{
		return m_GunEmitVec;
	}

	inline GUNNER_POSITION GunnerPos() const
	{
		return (GUNNER_POSITION)m_GunnerPosition;
	}

	inline const AuPoint& GunPivot() const
	{
		return m_GunPivot;
	}

	inline float GunPivotMax() const
	{
		return m_GunPivotMax;
	}

	inline float GunPivotMin() const
	{
		return m_GunPivotMin;
	}

	inline const AuPoint& GunSightLoc() const
	{
		return m_GunSightLoc;
	}

	virtual bool IsTurret() const
	{
		return true;
	}

	// Turrets are assembled separately.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	void MakeScreen(const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane);

	virtual quint64 MemoryUsage() const;

	virtual COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	virtual AuPoint OperatorLoc() const
	{
		return m_GunnerLocation;
	}

	virtual AuStation OperatorStation(const AuPoint& gravityUp) const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void RenderTrianglesState(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count], quint8 state) const;

	virtual QString ReportDesc() const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	inline void SetBaseAxis(const AuPoint& baseAxis)
	{
		m_BaseAxis = baseAxis;
	}

	inline void SetBasePivot(const AuPoint& basePivot)
	{
		m_BasePivot = basePivot;
	}

	inline void SetBasePivotMax(float basePivotMax)
	{
		m_BasePivotMax = basePivotMax;
	}

	inline void SetBasePivotMin(float basePivotMin)
	{
		m_BasePivotMin = basePivotMin;
	}

	inline void SetGunAxis(const AuPoint& gunAxis)
	{
		m_GunAxis = gunAxis;
	}

	inline void SetGunEmitLoc(const AuPoint& gunEmitLoc)
	{
		m_GunEmitLoc = gunEmitLoc;
	}

	inline void SetGunEmitVec(const AuPoint& gunEmitVec)
	{
		m_GunEmitVec = gunEmitVec;
	}

	inline void SetGunner(GUNNER_POSITION pos, const AuPoint& loc, const AuPoint& facing)
	{
		m_GunnerFacing = facing;
		m_GunnerLocation = loc;
		m_GunnerPosition = pos;
	}

	inline void SetGunPivot(const AuPoint& gunPivot)
	{
		m_GunPivot = gunPivot;
	}

	inline void SetGunPivotMax(float gunPivotMax)
	{
		m_GunPivotMax = gunPivotMax;
	}

	inline void SetGunPivotMin(float gunPivotMin)
	{
		m_GunPivotMin = gunPivotMin;
	}

	inline void SetGunSightLoc(const AuPoint& gunSightLoc)
	{
		m_GunSightLoc = gunSightLoc;
	}

	virtual void SetModule(COMMODITYID cid)
	{
		m_Module = cid;
	}

	virtual void SetState(quint8 state)
	{
		if(state < STATES_Count)
			AuScStatePart::SetState(state);
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual QString StateName() const;

	virtual QString StateName(quint8 state) const;

	virtual QMap<quint8, QString> StateNames() const;

	virtual AuStation::TYPE StationType() const
	{
		return AuStation::Turret_Integrated;
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual void TurretAimMatrix(float baseRot, AuMatrix4x4& m) const;

	virtual void TurretBaseMatrix(float baseRot, AuMatrix4x4& m) const;

	virtual void TurretSightMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const;

	virtual void TurretGunMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const;

	virtual void TurretMuzzleMatrix(float baseRot, float gunRot, AuMatrix4x4& m) const;

	virtual quint16 Turrets() const
	{
		return 1;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Turret;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScTurret& c = dynamic_cast<const AuScTurret&>(part);

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

			AuScStatePart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual AuPoint WeaponEmitLoc() const
	{
		return m_GunEmitLoc;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPodBay : public AuScIcon
{
public:	// Constructors
	AuScPodBay();
	AuScPodBay(const AuScPodBay& c);
	AuScPodBay(const AuScPodBay& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PodBay;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPodBay& c = dynamic_cast<const AuScPodBay&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Reference pictures display images for tracing in the designer. Not part of the final design.

class AUDATA_DLL AuScSketch : public AuScScreen
{
	QString								m_FilePath;				// On designer's computer.

public:	// Constructors
	AuScSketch();
	AuScSketch(const AuScSketch& c);
	AuScSketch(const AuScSketch& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	inline const QString& FilePath() const
	{
		return m_FilePath;
	}

	virtual bool IsOmitted() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual float Opacity() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	inline void SetFilePath(const QString& filePath)
	{
		m_FilePath = filePath;
	}

	virtual void SetOpacity(float o);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Sketch;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScSketch& c = dynamic_cast<const AuScSketch&>(part);

			m_FilePath = c.m_FilePath;

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScLaunchTubeJig : public AuScMeshBase
{
public:	// Constructors
	AuScLaunchTubeJig();
	AuScLaunchTubeJig(const AuScLaunchTubeJig& c);
	AuScLaunchTubeJig(const AuScLaunchTubeJig& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	// Jigs are not needed in the final design.
	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return false;
	}

	// Returns true for all jigs.
	virtual bool IsJig() const
	{
		return true;
	}

	virtual bool IsLaunchTubeJig() const
	{
		return true;
	}

	// Jigs are always omitted from the finalized design.
	virtual bool IsOmitted() const
	{
		return true;
	}

	// Jigs aren't even present in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	// Jig can't be jig cut.
	virtual void JigCutFaces(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	virtual void JigFaces(AuScFaceList& jigFaces) const
	{
		Faces(jigFaces);
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_LaunchTubeJig;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScLaunchTubeJig& c = dynamic_cast<const AuScLaunchTubeJig&>(part);

			Q_UNUSED(c);

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A cutting jig is used as a generic cutter. It only cuts and does nothing with the result.

class AUDATA_DLL AuScCuttingJig : public AuScMeshBase
{
public:	// Constructors
	AuScCuttingJig();
	AuScCuttingJig(const AuScCuttingJig& c);
	AuScCuttingJig(const AuScCuttingJig& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	// Jigs are not needed in the final design.
	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return false;
	}

	virtual bool IsCuttingJig() const
	{
		return true;
	}

	// Returns true for all jigs.
	virtual bool IsJig() const
	{
		return true;
	}

	// Jigs are always omitted from the finalized design.
	virtual bool IsOmitted() const
	{
		return true;
	}

	// Jigs aren't even present in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	// Jig can't be jig cut.
	virtual void JigCutFaces(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	virtual void JigFaces(AuScFaceList& jigFaces) const
	{
		Faces(jigFaces);
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_CuttingJig;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScCuttingJig& c = dynamic_cast<const AuScCuttingJig&>(part);

			Q_UNUSED(c);

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScSpaceVehicleLandingSystem : public AuScIcon
{
public:	// Constructors
	AuScSpaceVehicleLandingSystem();
	AuScSpaceVehicleLandingSystem(const AuScSpaceVehicleLandingSystem& c);
	AuScSpaceVehicleLandingSystem(const AuScSpaceVehicleLandingSystem& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool LandingSystemPosition(AuMatrix4x4& landerPosM) const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_SpaceVehicleLandingSystem;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScSpaceVehicleLandingSystem& c = dynamic_cast<const AuScSpaceVehicleLandingSystem&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual AuPoint VLSEmitLoc() const
	{
		return m_Face.FaceLocation();
	}

	virtual AuPoint VLSEmitVec() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScLightBulb : public AuScIcon
{
	QColor								m_Color;
	float								m_kL;
	float								m_kQ;
	float								m_CosInnerCone;
	float								m_CosOuterCone;

public:	// Constructors
	AuScLightBulb();
	AuScLightBulb(const AuScLightBulb& c);
	AuScLightBulb(const AuScLightBulb& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeLight() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool IsLight() const
	{
		return true;
	}

	virtual bool IsOnePerDesign() const
	{
		return true;
	}

	inline bool IsSpotLight() const
	{
		return m_CosInnerCone > -1 || m_CosOuterCone > -1;
	}

	virtual QColor LightColor() const
	{
		return m_Color;
	}

	virtual float LightCosInnerCone() const
	{
		return m_CosInnerCone;
	}

	virtual float LightCosOuterCone() const
	{
		return m_CosOuterCone;
	}

	virtual AuPoint LightDir() const;

	virtual float LightkL() const
	{
		return m_kL;
	}

	virtual float LightkQ() const
	{
		return m_kQ;
	}

	virtual AuPoint LightLoc() const;

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual void SetLight(const QColor& color, float kL, float kQ, float cosOuterCone, float cosInnerCone);

	virtual void SetLightColor(const QColor& color)
	{
		m_Color = color;
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_LightBulb;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScLightBulb& c = dynamic_cast<const AuScLightBulb&>(part);

			m_Color = c.m_Color;
			m_kL = c.m_kL;
			m_kQ = c.m_kQ;
			m_CosInnerCone = c.m_CosInnerCone;
			m_CosOuterCone = c.m_CosOuterCone;

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScStatusWorldMap : public AuScScreen
{
public:	// Constructors
	AuScStatusWorldMap();
	AuScStatusWorldMap(const AuScStatusWorldMap& c);
	AuScStatusWorldMap(const AuScStatusWorldMap& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_StatusWorldMap;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScStatusWorldMap& c = dynamic_cast<const AuScStatusWorldMap&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScDecal : public AuScMeshBase
{
public:	// Constructors
	AuScDecal();
	AuScDecal(const AuScDecal& c);
	AuScDecal(const AuScDecal& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual bool IsDecal() const
	{
		return true;
	}

	virtual float Opacity() const;

	virtual void SetOpacity(float o);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Decal;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScDecal& c = dynamic_cast<const AuScDecal&>(part);

			Q_UNUSED(c);

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScEmpireFlag : public AuScScreen
{
public:	// Constructors
	AuScEmpireFlag();
	AuScEmpireFlag(const AuScEmpireFlag& c);
	AuScEmpireFlag(const AuScEmpireFlag& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual float Opacity() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	virtual void RenderTrianglesScaled(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count], float scaleX, float scaleY) const;

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetOpacity(float o);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_EmpireFlag;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScEmpireFlag& c = dynamic_cast<const AuScEmpireFlag&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScEmpireName : public AuScScreen
{
public:	// Constructors
	AuScEmpireName();
	AuScEmpireName(const AuScEmpireName& c);
	AuScEmpireName(const AuScEmpireName& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual float Opacity() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint);

	virtual void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity);

	virtual void PaintInUse(QList<QColor>& paintInUse) const;

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetOpacity(float o);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_EmpireName;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScEmpireName& c = dynamic_cast<const AuScEmpireName&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScShipName : public AuScScreen
{
public:	// Constructors
	AuScShipName();
	AuScShipName(const AuScShipName& c);
	AuScShipName(const AuScShipName& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual float Opacity() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint);

	virtual void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity);

	virtual void PaintInUse(QList<QColor>& paintInUse) const;

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetOpacity(float o);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_ShipName;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScShipName& c = dynamic_cast<const AuScShipName&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScCaptName : public AuScScreen
{
public:	// Constructors
	AuScCaptName();
	AuScCaptName(const AuScCaptName& c);
	AuScCaptName(const AuScCaptName& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual float Opacity() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint);

	virtual void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity);

	virtual void PaintInUse(QList<QColor>& paintInUse) const;

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetOpacity(float o);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_CaptName;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScCaptName& c = dynamic_cast<const AuScCaptName&>(part);

			Q_UNUSED(c);

			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScDecalJig : public AuScMeshBase
{
public:	// Constructors
	AuScDecalJig();
	AuScDecalJig(const AuScDecalJig& c);
	AuScDecalJig(const AuScDecalJig& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	// Jigs are not needed in the final design.
	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return false;
	}

	virtual bool IsDecalJig() const
	{
		return true;
	}

	// Returns true for all jigs.
	virtual bool IsJig() const
	{
		return true;
	}

	// Jigs are always omitted from the finalized design.
	virtual bool IsOmitted() const
	{
		return true;
	}

	// Jigs aren't even present in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	// Jig can't be jig cut.
	virtual void JigCutFaces(AuScFaceList& faces) const
	{
		Q_UNUSED(faces);
	}

	virtual void JigFaces(AuScFaceList& jigFaces) const
	{
		Faces(jigFaces);
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_DecalJig;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScDecalJig& c = dynamic_cast<const AuScDecalJig&>(part);

			Q_UNUSED(c);

			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScVoid : public AuScPart
{
protected:
	AuScFaceList						m_Faces;
	quint16								m_PartId;				// Associated part, e.g. turbo lift, door.
	quint8								m_PartState;			// Associated part state, e.g. turbo lift, door.

public:	// Constructors
	AuScVoid();
	AuScVoid(const AuScVoid& c);
	AuScVoid(const AuScVoid& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void AddFaces(const AuScFaceList& faces);

	virtual quint16 AssociatedPartId() const
	{
		return m_PartId;
	}

	virtual quint8 AssociatedPartState() const
	{
		return m_PartState;
	}

	virtual bool CanFracture() const
	{
		return true;
	}

	virtual bool CanFuse() const
	{
		return false;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual bool CanMakeIntoOtherPart() const
	{
		return true;
	}

	virtual AuPoint Center() const;

	virtual bool CheckVoidFacesPlanar() const;

	virtual bool CutFaces(const AuPointArray& cutLine);

	virtual void DeleteFaces(const QSet<quint32>& faceIds);

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions);

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeIds(QSet<quint32>& edgeIds) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const;

	virtual void EffectFaces(SURFACE_EFFECT effect);

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual void FaceIds(QSet<quint32>& faceIds) const;

	virtual AuPoint FaceLoc(const AuPoint& loc, const AuPoint& vec, AuPoint& terN, const AuMatrix4x4& stopM) const;

	virtual void Faces(AuScFaceList& faces) const;

	virtual void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual void FlattenVertices();

	virtual void FlattenVertices(const AuScSelectInfo& selInfo);

	virtual bool FractureFaces(const AuScSelectInfo& selInfo);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	virtual bool IsInsideVoid(const AuPoint& loc) const;

	virtual bool IsInsideVoid(const AuPoint& loc, const AuScParts& parts) const;

	virtual bool IsInsideVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool IsInsideVoidEver(const AuPoint& loc) const
	{
		return IsInsideVoid(loc);
	}

	virtual bool IsValid() const;

	// Voids are not visible in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	virtual bool IsVoid() const
	{
		return true;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	// 'extVec' is relative to the gridPlane.
	void MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
			bool closeEnds);

	// Makes a lat/long type sphere.
	void MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps, const AuMatrix4x4& gridPlane);

	// Makes an evenly tessellated sphere.
	void MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
			const AuMatrix4x4& gridPlane);

	// 'profile' is drawn in the XY gridPlane. It spins around the X axis. Drawn from -X to +X, the triangles face outward.
	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane);

	void MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides);

	// 'shape' is expected to be drawn in 2D. The origin is the guide along which 'shape' follows 'path'.
	void MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides);

	virtual quint64 MemoryUsage() const;

	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual bool PunchFaces(const AuPointArray& cutBnd);

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void ReverseFaces();

	virtual void ReverseFaces(const AuScSelectInfo& selInfo);

	virtual void ReverseVertexNormals();

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		m_PartId = partId;
		m_PartState = partState;
	}

	virtual void SetFaceOpacity(float opacity);

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity);

	virtual void SetFaces(const AuScFaceList& faces)
	{
		m_Faces = faces;
	}

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex);

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex);

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const;

	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScVoid& c = dynamic_cast<const AuScVoid&>(part);

			m_Faces = c.m_Faces;
			m_PartId = c.m_PartId;
			m_PartState = c.m_PartState;

			AuScPart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexIds(QSet<quint32>& vertexIds) const;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScHullVoid : public AuScVoid
{
public:	// Constructors
	AuScHullVoid();
	AuScHullVoid(const AuScVoid& c);
	AuScHullVoid(const AuScVoid& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool IsHullVoid() const
	{
		return true;
	}

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_HullVoid;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScHullVoid& c = dynamic_cast<const AuScHullVoid&>(part);

			Q_UNUSED(c);

			AuScVoid::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScRoomVoid : public AuScVoid
{
private:
	quint8								m_Access;				// HOLD_ACCESS in bottom nibble. PERSONNEL_ACCESS in top nibble.
	quint8								m_GravityType;			// GRAVITY enum value.
	AuPoint								m_GravityUp;			// Gravity up vector when gravity is provided artificially.

public:	// Constructors
	AuScRoomVoid();
	AuScRoomVoid(const AuScRoomVoid& c);
	AuScRoomVoid(const AuScRoomVoid& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual void FacesInHullSilhouette(AuScFaceList& faces) const;

	virtual GRAVITY GravityType() const
	{
		return (GRAVITY)m_GravityType;
	}

	virtual AuPoint GravityUp(const AuMatrix4x4& stopM) const;

	virtual AuPoint GravityUp(const AuPoint& gravityN, const AuMatrix4x4& stopM) const;

	virtual double HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM, AuPoint& floorN) const;

	virtual double HeightOfCeiling(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM) const;

	virtual HOLD_ACCESS HoldAccess() const
	{
		return (HOLD_ACCESS)(m_Access & 0x0f);
	}

	virtual bool IsAccessGranted(quint8 personnelAccess) const;

	virtual bool IsInsideRoomVoid(const AuPoint& loc, const AuScParts& parts) const;

	virtual bool IsInsideRoomVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool IsInsideRoomVoid(const AuPoint& loc, double elevate, const AuScParts& parts) const;

	virtual bool IsInsideRoomVoid(quint16 partId, const AuPoint& loc, double elevate, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool IsRoomVoid() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	// Returns a bitwise combination of PERSONNEL_ACCESS bits.
	virtual quint8 PersonnelAccess() const
	{
		quint8 personnelAccess = m_Access >> 4;
		if(!personnelAccess)
			return PERSONNEL_Everyone;
		return personnelAccess;
	}

	virtual bool RandomVoidLocation(quint16 partId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual QString ReportDesc() const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual void SetGravity(GRAVITY g, const AuPoint& up)
	{
		m_GravityType = g;
		m_GravityUp = up;
	}

	virtual void SetHoldAccess(HOLD_ACCESS holdAccess)
	{
		m_Access &= 0xf0;				// Turn off current bottom nibble bits.
		m_Access |= holdAccess & 0x0f;	// Turn on new bottom nibble bits.
	}

	virtual void SetPersonnelAccess(quint8 personnelAccess)
	{
		m_Access &= 0x0f;				// Turn off current top nibble bits.
		if((personnelAccess & 0x0f) != PERSONNEL_Everyone)
			m_Access |= personnelAccess << 4;	// Turn on new top nibble bits, if access is not everyone.
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_RoomVoid;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScRoomVoid& c = dynamic_cast<const AuScRoomVoid&>(part);
			m_Access = c.m_Access;
			m_GravityType = c.m_GravityType;
			m_GravityUp = c.m_GravityUp;
			AuScVoid::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPath : public AuScPart
{
protected:
	quint16								m_PartId;				// Associated part, e.g. door, landing gear.
	quint8								m_PartState;			// Associated part state, e.g. door, landing gear.
	AuScPathSeg							m_Segment;

public:	// Constructors
	AuScPath();
	AuScPath(const AuScPath& c);
	AuScPath(const AuScPath& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual quint16 AssociatedPartId() const
	{
		return m_PartId;
	}

	virtual quint8 AssociatedPartState() const
	{
		return m_PartState;
	}

	virtual bool CanFracture() const
	{
		return false;
	}

	virtual bool CanFuse() const
	{
		return false;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// A screen cannot be joined to any other mesh.
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual bool CanMakeIntoOtherPart() const
	{
		return false;
	}

	virtual AuPoint Center() const;

	virtual bool CutFaces(const AuPointArray& cutLine)
	{
		Q_UNUSED(cutLine);
		return false;
	}

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeIds(QSet<quint32>& edgeIds) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const;

	virtual void EffectFaces(SURFACE_EFFECT effect);

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual void FaceIds(QSet<quint32>& faceIds) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	virtual bool IsPath() const
	{
		return true;
	}

	virtual bool IsValid() const;

	// Paths are not visible in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	void MakePath(const AuPoint& start, const AuPoint& end);

	virtual quint64 MemoryUsage() const;

	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual const AuScPathSeg& PathSegment(int segmentIndex) const
	{
		Q_UNUSED(segmentIndex);
		return m_Segment;
	}

	virtual int PathSegments() const
	{
		return 1;
	}

	virtual bool PunchFaces(const AuPointArray& cutBnd)
	{
		Q_UNUSED(cutBnd);
		return false;
	}

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		m_PartId = partId;
		m_PartState = partState;
	}

	virtual void SetFaceOpacity(float opacity);

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity);

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex);

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex);

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const;

	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPath& c = dynamic_cast<const AuScPath&>(part);

			m_PartId = c.m_PartId;
			m_PartState = c.m_PartState;
			m_Segment = c.m_Segment;

			AuScPart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexIds(QSet<quint32>& vertexIds) const;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPathWalk : public AuScPath
{
public:	// Constructors
	AuScPathWalk();
	AuScPathWalk(const AuScPathWalk& c);
	AuScPathWalk(const AuScPathWalk& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PathWalk;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPathWalk& c = dynamic_cast<const AuScPathWalk&>(part);

			Q_UNUSED(c);

			AuScPath::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScPathLadder : public AuScPath
{
	AuPoint								m_Facing;				// Vector pointing in the facing direction while on the ladder path.

public:	// Constructors
	AuScPathLadder();
	AuScPathLadder(const AuScPathLadder& c);
	AuScPathLadder(const AuScPathLadder& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	inline const AuPoint& Facing() const
	{
		return m_Facing;
	}

	virtual bool FindLadder(const AuPoint& fromLoc, AuMatrix4x4& mLadder, double& dist);

	virtual bool IsLadderPath() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	inline void SetFacing(const AuPoint& dir)
	{
		m_Facing = dir;
		m_Facing.Unit();
	}

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_PathLadder;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScPathLadder& c = dynamic_cast<const AuScPathLadder&>(part);

			m_Facing = c.m_Facing;

			AuScPath::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScWaterVehicleParkingSpot : public AuScIcon
{
public:	// Constructors
	AuScWaterVehicleParkingSpot();
	AuScWaterVehicleParkingSpot(const AuScWaterVehicleParkingSpot& c);
	AuScWaterVehicleParkingSpot(const AuScWaterVehicleParkingSpot& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual AuMatrix4x4 PositionMatrix() const;

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_WaterVehicleParkingSpot;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScWaterVehicleParkingSpot& c = dynamic_cast<const AuScWaterVehicleParkingSpot&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual quint16 WaterVehicleParkingSpots() const
	{
		return 1;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScBarrier : public AuScPart
{
protected:
	AuScFaceList						m_Faces;
	quint16								m_PartId;				// Associated part, e.g. turbo lift, door.
	quint8								m_PartState;			// Associated part state, e.g. turbo lift, door.
	AuScSilhouette3d					m_Silhouette;			// Used by IsPathBlocked test.

public:	// Constructors
	AuScBarrier();
	AuScBarrier(const AuScBarrier& c);
	AuScBarrier(const AuScBarrier& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void AddFaces(const AuScFaceList& faces);

	virtual quint16 AssociatedPartId() const
	{
		return m_PartId;
	}

	virtual quint8 AssociatedPartState() const
	{
		return m_PartState;
	}

	virtual bool BlockSegment(const AuPoint& fromLoc, AuPoint& toLoc, AuPoint& floorN) const;

	virtual bool CanFracture() const
	{
		return false;
	}

	virtual bool CanFuse() const
	{
		return false;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual bool CanMakeIntoOtherPart() const
	{
		return true;
	}

	virtual AuPoint Center() const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool CutFaces(const AuPointArray& cutLine);

	virtual void DeleteFaces(const QSet<quint32>& faceIds);

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions);

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeIds(QSet<quint32>& edgeIds) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const;

	virtual void EffectFaces(SURFACE_EFFECT effect);

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual void FaceIds(QSet<quint32>& faceIds) const;

	virtual AuPoint FaceLoc(const AuPoint& loc, const AuPoint& vec, AuPoint& terN, const AuMatrix4x4& stopM) const;

	virtual void Faces(AuScFaceList& faces) const;

	virtual void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const;

	virtual void FacesInHullSilhouette(AuScFaceList& faces) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull)
	{
		Q_UNUSED(partId);
		Q_UNUSED(parts);
		Q_UNUSED(shipHull);
		return !IsOmitted();
	}

	virtual void FlattenVertices();

	virtual void FlattenVertices(const AuScSelectInfo& selInfo);

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	virtual bool IsBarrier() const
	{
		return true;
	}

	virtual bool IsInsideObstructionModel(const AuPoint& loc) const;

	virtual bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	virtual bool IsPathBlocked(quint16 partId, const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal,
			const AuDoorStateHash& doorStates, const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool IsValid() const;

	// Barriers are not visible in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	// 'extVec' is relative to the gridPlane.
	void MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
			bool closeEnds);

	// Makes a lat/long type sphere.
	void MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps, const AuMatrix4x4& gridPlane);

	// Makes an evenly tessellated sphere.
	void MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
			const AuMatrix4x4& gridPlane);

	// 'profile' is drawn in the XY gridPlane. It spins around the X axis. Drawn from -X to +X, the triangles face outward.
	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane);

	void MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides);

	// 'shape' is expected to be drawn in 2D. The origin is the guide along which 'shape' follows 'path'.
	void MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides);

	virtual quint64 MemoryUsage() const;

	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual bool PunchFaces(const AuPointArray& cutBnd);

	virtual void RefreshObstructionsCache(quint16 partId, const AuScParts& parts);

	virtual bool RefreshObstructionsCacheIfNeeded(quint16 partId, const AuScParts& parts);

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual QString ReportDesc() const;

	virtual void ReverseFaces();

	virtual void ReverseFaces(const AuScSelectInfo& selInfo);

	virtual void ReverseVertexNormals();

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		m_PartId = partId;
		m_PartState = partState;
	}

	virtual void SetFaceOpacity(float opacity);

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity);

	virtual void SetFaces(const AuScFaceList& faces)
	{
		m_Faces = faces;
	}

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex);

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex);

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const;

	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Barrier;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBarrier& c = dynamic_cast<const AuScBarrier&>(part);

			m_Faces = c.m_Faces;
			m_PartId = c.m_PartId;
			m_PartState = c.m_PartState;
			m_Silhouette = c.m_Silhouette;

			AuScPart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexIds(QSet<quint32>& vertexIds) const;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScRocketParkingSpot : public AuScIcon
{
public:	// Constructors
	AuScRocketParkingSpot();
	AuScRocketParkingSpot(const AuScRocketParkingSpot& c);
	AuScRocketParkingSpot(const AuScRocketParkingSpot& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual AuMatrix4x4 PositionMatrix() const;

	virtual quint16 RocketParkingSpots() const
	{
		return 1;
	}

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_RocketParkingSpot;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScRocketParkingSpot& c = dynamic_cast<const AuScRocketParkingSpot&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScSite : public AuScMeshBase
{
public:
	enum GRADE
	{
		Terrain,						// Terrain elevation is unmodified.
		Level,							// Terrain that lands in the site is raised or lowered as necessary to site elevation.
		Fill,							// Terrain that lands in the site is raised to site elevation, if it is below the site.
		Cut,							// Terrain that lands in the site is lowered to site elevation, if it is above the site.
	};

private:
	quint8								m_Grade;				// GRADE enum
	quint8								m_Grass;				// Grass: 0=remove it, 1=leave it, 2=mow it, 3=grassy lawn
	quint8								m_Pavement;				// Pavement type for texture selection. WORLDTEX_TYPE
	quint8								m_Rocks;				// Natural rocks: 0=remove them, 1=leave them.
	quint8								m_Shrubs;				// Natural shrubs: 0=remove them, 1=leave them.
	quint8								m_TerBlend;				// How to blend when faces have glow. 0=linear, 1=cosine
	quint8								m_Trees;				// Natural trees: 0=remove them, 1=leave them.

public:	// Constructors
	AuScSite();
	AuScSite(const AuScSite& c);
	AuScSite(const AuScSite& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual void BuildingCursorSiteSegments(AuPointArray& edges) const;

	// Returns a number from 0 to 1. 0=no influence, 1=full influence.
	virtual float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const;

	virtual float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
			bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
			const AuZoneResource& res) const;

	virtual void BuildZoneCheckLocs(AuPointArray& locs) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double FieldArea() const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual void GlowVertexLocs(const AuPointArray& vertLocs, const QColor& glow);

	inline GRADE Grade() const
	{
		return (GRADE)m_Grade;
	}

	inline quint8 Grass() const
	{
		return m_Grass;
	}

	virtual bool InBuildingSite(const AuPoint& loc) const;

	virtual bool InBuildingSite(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const;

	virtual bool IsSite() const
	{
		return true;
	}

	virtual bool IsValid() const;

	// Site meshes are not visible in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	void MakeSite(const AuPointArray& bnd, const AuMatrix4x4& gridPlane);

	virtual quint64 MemoryUsage() const;

	inline WORLDTEX_TYPE Pavement() const
	{
		return (WORLDTEX_TYPE)m_Pavement;
	}

	virtual bool RandomSiteLocation(const AuScParts& parts, AuPoint& siteLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates) const;

	inline quint8 Rocks() const
	{
		return m_Rocks;
	}

	virtual QString SelectedDesc(const AuScParts& parts) const;

	inline void SetGrade(GRADE grade)
	{
		m_Grade = grade;
	}

	inline void SetGrass(quint8 grass)
	{
		m_Grass = grass;
	}

	inline void SetPavement(quint8 pave)
	{
		m_Pavement = pave;
	}

	inline void SetRocks(quint8 rocks)
	{
		m_Rocks = rocks;
	}

	inline void SetShrubs(quint8 shrubs)
	{
		m_Shrubs = shrubs;
	}

	virtual void SetSiteProperties(quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs, quint8 terBlend,
			quint8 trees);

	inline void SetTerrainBlend(quint8 terBlend)
	{
		m_TerBlend = terBlend;
	}

	inline void SetTrees(quint8 trees)
	{
		m_Trees = trees;
	}

	inline quint8 Shrubs() const
	{
		return m_Shrubs;
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual bool SiteExtents(AuPoint& min, AuPoint& max) const;

	inline quint8 TerrainBlend() const
	{
		return m_TerBlend;
	}

	inline quint8 Trees() const
	{
		return m_Trees;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Site;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScSite& c = dynamic_cast<const AuScSite&>(part);
			m_Grade = c.m_Grade;
			m_Grass = c.m_Grass;
			m_Pavement = c.m_Pavement;
			m_Rocks = c.m_Rocks;
			m_Shrubs = c.m_Shrubs;
			m_TerBlend = c.m_TerBlend;
			m_Trees = c.m_Trees;
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScTree : public AuScIcon
{
private:
	quint16								m_PlantCID;				// Commodity id of the plant resource, Random if CID_Invalid.

public:	// Constructors
	AuScTree();
	AuScTree(const AuScTree& c);
	AuScTree(const AuScTree& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
			bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
			const AuZoneResource& res) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	inline COMMODITYID PlantCID() const
	{
		return (COMMODITYID)m_PlantCID;
	}

	virtual QString SelectedDesc(const AuScParts& parts) const;

	inline void SetPlant(COMMODITYID cid)
	{
		m_PlantCID = cid;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Tree;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScTree& c = dynamic_cast<const AuScTree&>(part);
			m_PlantCID = c.m_PlantCID;
			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScShrub : public AuScIcon
{
private:
	quint16								m_PlantCID;				// Commodity id of the plant resource, Random if CID_Invalid.

public:	// Constructors
	AuScShrub();
	AuScShrub(const AuScShrub& c);
	AuScShrub(const AuScShrub& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual float BuildingSiteInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
			bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend,
			const AuZoneResource& res) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	inline COMMODITYID PlantCID() const
	{
		return (COMMODITYID)m_PlantCID;
	}

	virtual QString SelectedDesc(const AuScParts& parts) const;

	inline void SetPlant(COMMODITYID cid)
	{
		m_PlantCID = cid;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Shrub;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScShrub& c = dynamic_cast<const AuScShrub&>(part);
			m_PlantCID = c.m_PlantCID;
			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScFence : public AuScPart
{
public:
	enum STYLE
	{
		Posts,
		PostsT,
		Wire,
		WireT,
		Range,
		RangeT,
		ChainLink,
		ChainLinkRazorWire,
		SplitRail,
		WoodYard,
		SandBagWall,
		BrickWall,
		StoneWall,
		Custom,
		STYLE_Count	// If you add a style, add an entry to the StyleName function.
	};

protected:
	quint32								m_DesignId;				// Design id.
	AuSegment							m_Segment;
	quint8								m_Style;				// STYLE enum

public:	// Constructors
	AuScFence();
	AuScFence(const AuScFence& c);
	AuScFence(const AuScFence& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual bool CanFracture() const
	{
		return false;
	}

	virtual bool CanFuse() const
	{
		return false;
	}

	virtual bool CanJoinFaces(const AuScPart* joinTarget) const
	{
		// A screen cannot be joined to any other mesh.
		Q_UNUSED(joinTarget);
		return false;
	}

	virtual bool CanMakeIntoOtherPart() const
	{
		return false;
	}

	virtual AuPoint Center() const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool CutFaces(const AuPointArray& cutLine)
	{
		Q_UNUSED(cutLine);
		return false;
	}

	virtual bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const;

	virtual double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const;

	virtual void EdgeIds(QSet<quint32>& edgeIds) const;

	virtual void EdgeSegments(AuPointArray& edges) const;

	virtual SURFACE_EFFECT Effect(const AuScSelectInfo& selInfo) const;

	virtual void EffectFaces(SURFACE_EFFECT effect);

	virtual void EffectFaces(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	virtual void EffectFacesCylindrical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectFacesProject(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void EffectFacesSpherical(const AuScSelectInfo& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void EffectRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void EffectStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void EffectUnwrap(const AuScSelectInfo& sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual quint32 FenceDesignId() const
	{
		return m_DesignId;
	}

	virtual double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	virtual void FaceIds(QSet<quint32>& faceIds) const;

	virtual void FaceSpriteLocs(const QSet<quint32>& selFaces, QList<AuPointAlpha>& selLocs, QList<AuPointAlpha>& unselLocs) const;

	static void FenceSectionBarrierFaces(STYLE fenceStyle, AuScFaceList& faces);

	// 'fenceStyle' of STYLE_Count retrieves the under construction faces.
	static void FenceSectionFaces(STYLE fenceStyle, AuScFaceList& faces);

	static void FencePostBarrierFaces(STYLE fenceStyle, AuScFaceList& faces);

	// 'fenceStyle' of STYLE_Count retrieves the under construction faces.
	static void FencePostFaces(STYLE fenceStyle, AuScFaceList& faces);

	virtual void FencePostFaces(AuScFaceList& faces) const;

	static double FencePostSpacing(STYLE fenceStyle);

	virtual void FenceSegments(AuSegmentArray& segs) const
	{
		segs.append(m_Segment);
	}

	virtual int FenceStyle() const
	{
		return m_Style;
	}

	virtual double FenceVolume() const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const;

	virtual bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	virtual bool IsFence() const
	{
		return true;
	}

	virtual bool IsValid() const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	void MakeFence(STYLE style, quint32 customDesignId, const AuPoint& start, const AuPoint& end);

	virtual quint64 MemoryUsage() const;

	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual bool PunchFaces(const AuPointArray& cutBnd)
	{
		Q_UNUSED(cutBnd);
		return false;
	}

	inline const AuSegment& Segment() const
	{
		return m_Segment;
	}

	virtual bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const;

	virtual bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	virtual void SetFaceOpacity(float opacity);

	virtual void SetFaceOpacity(const AuScSelectInfo& selInfo, float opacity);

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetFaceSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	inline void SetStyle(STYLE fenceStyle, quint32 designId)
	{
		m_DesignId = designId;
		m_Style = fenceStyle;
	}

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual AuSysId SightedSystem(quint16 partId) const;

	virtual bool SiteExtents(AuPoint& min, AuPoint& max) const;

	inline STYLE Style() const
	{
		return (STYLE)m_Style;
	}

	static QString StyleIcon(STYLE fenceStyle);

	static QString StyleName(STYLE fenceStyle);

	virtual void TextureFaces(SC_TEXTURE_INDEX texIndex);

	virtual void TextureFaces(const AuScSelectInfo& selInfo, SC_TEXTURE_INDEX texIndex);

	virtual void TextureFacesCylindrical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureFacesProject(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	virtual void TextureFacesSpherical(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	virtual SC_TEXTURE_INDEX TextureIndex(const AuScSelectInfo& selInfo) const;

	virtual void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	virtual void TextureRotate(const AuScSelectInfo& sel, const QPointF& pivot, double angle);

	virtual void TextureStretch(const AuScSelectInfo& sel, const QPointF& base, double scaleX, double scaleY);

	virtual void TextureUnwrap(const AuScSelectInfo& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t);

	virtual void Transform(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Fence;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScFence& c = dynamic_cast<const AuScFence&>(part);
			m_DesignId = c.m_DesignId;
			m_Segment = c.m_Segment;
			m_Style = c.m_Style;
			AuScPart::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}

	virtual double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	virtual void VertexIds(QSet<quint32>& vertexIds) const;

	virtual AuScPoint VertexLoc(quint32 vertexIndex) const;

	virtual void VertexSpriteLocs(const QSet<quint32>& selVerts, QList<AuPointAlpha>& selLocs,
			QList<AuPointAlpha>& unselLocs) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScSpacecraftParkingSpot : public AuScIcon
{
public:	// Constructors
	AuScSpacecraftParkingSpot();
	AuScSpacecraftParkingSpot(const AuScSpacecraftParkingSpot& c);
	AuScSpacecraftParkingSpot(const AuScSpacecraftParkingSpot& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual AuMatrix4x4 PositionMatrix() const;

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual quint16 SpacecraftParkingSpots() const
	{
		return 1;
	}

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_SpacecraftParkingSpot;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScSpacecraftParkingSpot& c = dynamic_cast<const AuScSpacecraftParkingSpot&>(part);

			Q_UNUSED(c);

			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScCamera : public AuScIcon
{
public:	// Constructors
	AuScCamera();
	AuScCamera(const AuScCamera& c);
	AuScCamera(const AuScCamera& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double IconBack() const;

	virtual double IconFore() const;

	virtual double IconLeft() const;

	virtual double IconRight() const;

	virtual bool IsCamera() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_Camera;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScCamera& c = dynamic_cast<const AuScCamera&>(part);
			Q_UNUSED(c);
			AuScIcon::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Berth name decal shows the name of the occupant of the associated berth.

class AUDATA_DLL AuScBerthName : public AuScScreen
{
	quint16								m_PartId;				// Associated berth id.

public:	// Constructors
	AuScBerthName();
	AuScBerthName(const AuScBerthName& c);
	AuScBerthName(const AuScBerthName& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual quint16 AssociatedPartId() const
	{
		return m_PartId;
	}

	virtual bool CanChangeOpacity() const
	{
		return true;
	}

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	virtual void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	virtual bool IsStatusDisplay() const
	{
		return true;
	}

	// Berth name decal that is not associated with any berth will be discarded in the final model.
	virtual bool IsVisibleInFinalModel() const;

	virtual void ListComponents(DeProcessComponentArray& pc, STREAMLINE streamline, float armor, COMMODITYID hullMaterial) const;

	virtual quint64 MemoryUsage() const;

	virtual float Opacity() const;

	virtual GLuint OwnTexture(AuGLEffects& re) const;

	virtual void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint);

	virtual void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity);

	virtual void PaintInUse(QList<QColor>& paintInUse) const;

	virtual void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	virtual void SetAssociatedPart(quint16 partId, quint8 partState)
	{
		Q_UNUSED(partState);
		m_PartId = partId;
	}

	virtual void SetFaceSelfIllumination(float selfIllum);

	virtual void SetOpacity(float o);

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_BerthName;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBerthName& c = dynamic_cast<const AuScBerthName&>(part);
			m_PartId = c.m_PartId;
			AuScScreen::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Environment void is a hull void that contains a breathable environment. Used in enclosures (biodomes and platforms) to define
// the natural environment space.

class AUDATA_DLL AuScEnvironmentVoid : public AuScHullVoid
{
public:	// Constructors
	AuScEnvironmentVoid();
	AuScEnvironmentVoid(const AuScEnvironmentVoid& c);
	AuScEnvironmentVoid(const AuScEnvironmentVoid& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual double HeightAboveFloor(const AuPoint& loc, const AuPoint& gravityN, const AuMatrix4x4& stopM, AuPoint& floorN) const;

	virtual bool IsAccessGranted(quint8 personnelAccess) const
	{
		Q_UNUSED(personnelAccess);
		return true;
	}

	virtual bool IsEnvironmentVoid() const
	{
		return true;
	}

	virtual bool IsInsideEnvironmentVoid(quint16 partId, const AuPoint& loc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual bool RandomVoidLocation(quint16 partId, AuPoint& voidLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_EnvironmentVoid;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScEnvironmentVoid& c = dynamic_cast<const AuScEnvironmentVoid&>(part);
			Q_UNUSED(c);
			AuScHullVoid::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A build zone is used to delineate the areas of a platform where buildings can be constructed.

class AUDATA_DLL AuScBuildZone : public AuScMeshBase
{
public:	// Constructors
	AuScBuildZone();
	AuScBuildZone(const AuScBuildZone& c);
	AuScBuildZone(const AuScBuildZone& c, const AuScSelectInfo& selInfo);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScPart& c) const;

public:	// Methods
	virtual void BuildingConstructionSegments(AuSegmentArray& edges) const;

	virtual void BuildingCursorSegments(AuPointArray& edges) const;

	virtual void BuildingCursorSiteSegments(AuPointArray& edges) const;

	virtual AuScPart* Clone() const;

	virtual AuScPart* CloneParts(const AuScSelectInfo& selInfo) const;

	virtual bool Finalize(quint16 partId, const AuScParts& parts, bool shipHull);

	virtual bool InBuildZone(const AuPoint& loc) const;

	virtual bool InBuildZone(const AuPoint& loc, double withinHeight) const;

	virtual bool IsBuildZone() const
	{
		return true;
	}

	virtual bool IsValid() const;

	// Site meshes are not visible in the final model.
	virtual bool IsVisibleInFinalModel() const
	{
		return false;
	}

	void MakeBuildZone(const AuPointArray& bnd, const AuMatrix4x4& gridPlane);

	virtual quint64 MemoryUsage() const;

	virtual bool RandomBuildZoneLocation(AuPoint& zoneLoc) const;

	virtual QString SelectedDesc(const AuScParts& parts) const;

	virtual bool Sight(quint16 partId, const AuPoint& eyeLoc, AuPoint& sightLoc, const AuDoorStateHash& doorStates,
			const AuLandingGearStateHash& landingGearStates, const AuLiftHash& liftStops, AuPoint& sightSurfaceNormal) const;

	virtual bool SightBuildZone(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual PART_TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeIcon() const;

	virtual QString TypeName() const;

	inline static PART_TYPE TypeStatic()
	{
		return Part_BuildZone;
	}

	virtual void Update(const AuScPart& part)
	{
		try
		{
			const AuScBuildZone& c = dynamic_cast<const AuScBuildZone&>(part);
			Q_UNUSED(c);
			AuScMeshBase::Update(part);
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuScParts_H
