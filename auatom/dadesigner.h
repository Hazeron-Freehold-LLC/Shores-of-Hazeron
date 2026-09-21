//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dadesigner.h,v 1.144 2022/05/17 16:01:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaDesigner_H
#define DaDesigner_H

#include "dadesignbase.h"
#include "dpconsoleempty.h"

class AuDesign;
class AuDesignPosition;
class AuDesignTex;
class AuScPart;
class AuScParts;
class AuSpecies;
class QStringList;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuDesignUndoRedoItem is one discrete step of an AuDesignUndoRedoTransaction.

class AuDesignUndoRedoItem
{
public:
	enum TYPE
	{
		Add,
		Change,
		Delete
	};

public:
	AuScParts*							m_Parts;				// Parts affected by the undo/redo transaction.
	quint8								m_Type;					// TYPE enum value

public:
	AuDesignUndoRedoItem(TYPE type);

	AuDesignUndoRedoItem();

	AuDesignUndoRedoItem(const AuDesignUndoRedoItem& c);

	~AuDesignUndoRedoItem();

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint64 MemoryUsage() const;

	AuScParts& Parts() const;

	inline TYPE Type() const
	{
		return (TYPE)m_Type;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuDesignUndoRedoItem& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDesignUndoRedoItem& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuDesignUndoRedoTransaction contains one or more AuDesignUndoRedoItems to encapsulate a single complex transaction that can be
// undone/redone.

class AuDesignUndoRedoTransaction : public QList<AuDesignUndoRedoItem>
{
public:
	QString								m_Desc;					// Brief description of the action.

public:
	inline AuDesignUndoRedoTransaction(const QString& desc)
	{
		m_Desc = desc;
	}

	inline AuDesignUndoRedoTransaction()
	{
	}

	inline AuDesignUndoRedoTransaction(const AuDesignUndoRedoTransaction& c) : QList<AuDesignUndoRedoItem>(c)
	{
		m_Desc = c.m_Desc;
	}


public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	// Returns true if this item can replace 'trans'.
	bool CanReplace(const AuDesignUndoRedoTransaction& trans) const;

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuDesignUndoRedoTransaction& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDesignUndoRedoTransaction& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDesignUndoRedoList : private QList<AuDesignUndoRedoTransaction>
{
private:
	int									m_NextUndo;				// Index of next undo transaction. Next redo is the one after this.

public:
	inline AuDesignUndoRedoList()
	{
		m_NextUndo = -1;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_NextUndo;
		out << (quint16)size();
		for(const_iterator ti = cbegin(); ti != cend(); ti++)
			out << *ti;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_NextUndo;
		clear();
		quint16 cnt;
		in >> cnt;
		while(cnt--)
		{
			AuDesignUndoRedoTransaction t;
			in >> t;
			append(t);
		}
	}

public:	// Methods
	// Adds a new undo transaction after the NextUndo index.
	// Truncates redo's that are now invalid.
	// The new undo will be the last one in this list of transactions after this call. NextUndo will be pointing at it.
	inline AuDesignUndoRedoTransaction& AddUndo(const AuDesignUndoRedoTransaction& undo)
	{
		// Truncate the transactions back to the next undo index.
		while(m_NextUndo < count()-1)
			removeLast();

		// Append the new undo transaction.
		append(undo);
		m_NextUndo = count()-1;

		// Return the address of the newly added transaction.
		return last();
	}

	// Returns true if there is at least one redo transaction after the NextUndo index.
	inline bool CanRedo() const
	{
		return m_NextUndo < count()-1;
	}

	// Returns true if there is a least one undo transaction at the NextUndo index.
	inline bool CanUndo() const
	{
		return m_NextUndo >= 0 && m_NextUndo < count();
	}

	// Returns the current/last undo/redo transaction.
	inline AuDesignUndoRedoTransaction& Last()
	{
		Q_ASSERT(!isEmpty());									// Failed to call StartUndo.
		Q_ASSERT(m_NextUndo == count()-1);						// Failed to call StartUndo.
		return last();
	}

	quint64 MemoryUsage() const;

	// Removes the second-to-last transaction if the last transaction is a replacement for it. Only applies to Change transactions.
	void MergeLast();

	// Returns 0 if no redo transaction is pending, i.e. if CanRedo returns false.
	// Returns the pending redo transaction and increments the NextUndo index.
	// NextUndo index is left pointing at the redo transaction returned.
	inline AuDesignUndoRedoTransaction* Redo()
	{
		// Insure there is a pending redo transaction.
		if(!CanRedo())
			return 0;

		// Increment NextUndo to the pending redo transaction and return it. The NextUndo transaction then becomes the pending undo
		// transaction.
		m_NextUndo++;
		AuDesignUndoRedoTransaction& redo = operator[](m_NextUndo);
		return &redo;
	}

	// Returns 0 if no undo transaction is pending, i.e. if CanUndo returns false.
	// Returns the pending undo transaction and decrements the NextUndo index.
	// NextUndo index is left pointing at the transaction prior to the undo transaction returned, which might be non-existent.
	inline AuDesignUndoRedoTransaction* Undo()
	{
		// Insure there is a pending undo transaction.
		if(!CanUndo())
			return 0;

		// The pending undo transaction is the one currently pointed at by NextUndo. We will return it and decrement the NextUndo
		// index to the previous pending undo transaction. NextUndo will drop below 0 when no more undo transactions remain.
		AuDesignUndoRedoTransaction& undo = operator[](m_NextUndo);
		m_NextUndo--;
		return &undo;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuDesignUndoRedoList& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDesignUndoRedoList& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaDesigner : public DaDesignBase  
{
private:
	mutable bool						m_DesignModified;		// true if design has been modified
	bool								m_ViewOnly;				// true to prohibit copy to clipboard and save.

private:
	mutable QReadWriteLock				m_EnvLock;
	QString								m_Architect;			// name of architect of loaded design. Not necessarily the person
																// viewing or editing it.
	AuId								m_idArchitect;			// id of architect of loaded design. Not necessarily the person
																// viewing or editing it.
	AuId								m_idOwner;				// id of owner of this design studio instance.
	AuDesign*							m_Design;				// The design we are modifying.
	QHash<quint16, AuElapsedTimer>		m_DesignChangeTimes;	// Key is part id, 0=the design record. Value is last change time.
	AuDesign*							m_Preview;				// The design we are previewing.
	quint32								m_PreviewGenSeed;
	bool								m_PreviewHasPower;
	QHash<AuId, AuDesignUndoRedoList>	m_UndoRedo;				// Undo/redo transactions for all architects present.

private:
	// OpenGL
	// Most normal faces are in the opaque and transparent VAOs.
	GLuint								m_EffectCur[EFF_Count];
	int									m_EffectFrame;			// Last frame of sequential effect animation, 0-5, once per sec.
	int									m_EffectRandFrame;		// Last frame of random effect animation, 0-5, once per sec.
	GLuint								m_EffectTex[EFF_Count+6][6];	// Six frames of animation for each kind of surface effect.
																// Extras for low power trans, grav drive, ftl drive and ftl hull.
	QHash<quint16, AuScVAO>				m_OpaqueVAOs;			// Opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_TransparentVAOs;		// Transparent faces. Key is partId.

	// These faces require their own texture, such as a screens, icons and sketches.
	QHash<quint16, AuScVAO>				m_OwnOpaqueVAOs;		// Faces that need their own textures, e.g. screens, icons.
	QHash<quint16, GLuint>				m_OwnTextures;			// One per part that needs its own texture.
	QHash<quint16, AuScVAO>				m_OwnTransparentVAOs;	// Transparent faces that need their own textures, e.g. sketches.

	// Decals get blended in after opaque faces but before transparent faces, in the opaque pass.
	QHash<quint16, AuScVAO>				m_DecalOpaqueVAOs;		// Opaque faces of decals. Key is partId.
	QHash<quint16, AuScVAO>				m_DecalTransparentVAOs;	// Transparent faces of decals. Key is partId.
	QHash<quint16, AuScVAO>				m_OwnDecalOpaqueVAOs;	// Opaque faces of decals with own texture. Key is partId.
	QHash<quint16, AuScVAO>				m_OwnDecalTransparentVAOs;	// Transparent faces of decals with own texture. Key is partId.

	// These faces are from windows, which change based on lighting conditions.
	QHash<quint16, AuScVAO>				m_WindowTranslucentVAOs;	// Translucent faces of windows.
	QHash<quint16, AuScVAO>				m_WindowTransparentVAOs;	// Transparent faces of windows.

	// Empty command chair and consoles. These are the old style 3ds model-based stations.
	DpConsoleEmpty						m_CmdEmpty;
	DpConsoleEmpty						m_EngineEmpty;
	DpConsoleEmpty						m_FireEmpty;
	DpConsoleEmpty						m_NavEmpty;
	DpConsoleEmpty						m_PilotEmpty;
	DpConsoleEmpty						m_PowerEmpty;
	DpConsoleEmpty						m_SensorEmpty;
	DpConsoleEmpty						m_ShieldEmpty;
	DpConsoleEmpty						m_SurgeryEmpty;
	DpConsoleEmpty						m_TransEmpty;

	GLuint								m_GroundTex;
	AuGLVAO								m_GroundVAO;

public:
	DaDesigner(quint16 studioId);
	DaDesigner();
	virtual ~DaDesigner();

protected:
	DaDesigner(const DaDesigner &rhs);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Adds 'partObjects' to 'partId'.
	void AddPartObjects(const AuId& archId, quint16 partId, const AuScParts& partObjects);

	void AddParts(const AuId& archId, const AuScParts& parts);

	void AddPartState(const AuId& archId, quint16 partId, quint8 state);

	void AddPartStop(const AuId& archId, quint16 partId, const QString& station);

	void AssociateParts(const AuId& archId, const AuScSelection& partIds);

	virtual DaAtom* Clone() const;

	// Returns BD_Invalid if not in building desing mode.
	BUILDING_STRUCTURE BuildingStructure() const;

	// Checks for valid parts in 'mimeData'.
	bool CheckMIME(bool objectMode, const QMimeData& mimeData) const;

	void CopyParts(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t);

	void CopyPartObjects(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t);

	void CopyPartsSymmetrical(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& gridPlane);

	// Creates a custom mime data object of the parts identified.
	// Parts are transformed using the inverse of the design plane matrix so the result is in the space of the design plane.
	// QMimeData is allocated with new.
	// 0 is returned on error or no shapes found.
	QMimeData* CreateMIME(const AuId& architectId, bool objectMode, const AuScSelection& partIds,
			const AuMatrix4x4& designPlane) const;

	void CutFaces(const AuId& archId, const AuScSelection& sel, const AuPointArray& cutLine);

	void DeletePartObjects(const AuId& archId, const AuScSelection& sel);

	void DeleteParts(const AuId& archId, const AuScSelection& sel);

	void DeletePartState(const AuId& archId, quint16 partId, quint8 state);

	void DeletePartStop(const AuId& archId, quint16 partId, const QString& station);

	// Returns true if 'sel' is changed.
	bool DeselectPartObjectsAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel, AuScSelection& sel) const;

	// Returns true if 'sel' is changed.
	bool DeselectPartTextureVertices(quint16 partId, quint32 partVertex, AuAtomRenderInfo& ari) const;

	// Called after a design is loaded to reanalyze the design.
	void DesignLoadCompleted();

	void DissociateParts(const AuId& archId, const AuScSelection& partIds);

	void DivideFaces(const AuId& archId, const AuScSelection& sel, quint8 divisions);

	// 'effect' is an AuScFace::EFFECT.
	void EffectFaces(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect);

	// 'effect' is an AuScFace::EFFECT.
	void EffectFacesCylindrical(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	// 'effect' is an AuScFace::EFFECT.
	void EffectFacesProject(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	// 'effect' is an AuScFace::EFFECT.
	void EffectFacesSpherical(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	void EffectIntensityVertices(const AuId& archId, const AuScSelection& sel, float intensity);

	// 'undoSeq' is used to distinguish one stream of texture moves from the next, for undo. It just has to be different from the
	// previous one so the undo transaction will not be absorbed into one single change undo.
	void EffectMove(const AuId& archId, const AuScSelection& sel, const QPointF& vec, quint8 undoSeq);

	// 'undoSeq' is used to distinguish one stream of texture moves from the next, for undo. It just has to be different from the
	// previous one so the undo transaction will not be absorbed into one single change undo.
	void EffectRotate(const AuId& archId, const AuScSelection& sel, const QPointF& pivot, double angle, quint8 undoSeq);

	// 'undoSeq' is used to distinguish one stream of texture moves from the next, for undo. It just has to be different from the
	// previous one so the undo transaction will not be absorbed into one single change undo.
	void EffectStretch(const AuId& archId, const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY,
			quint8 undoSeq);

	// 'effect' is an AuScFace::EFFECT.
	// 'unwrap' is a AuScFaceList::UNWRAP_METHOD
	void EffectUnwrap(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect, double texSize, quint8 unwrap,
			float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	// Ends the last undo created by 'architectId'. This is normally not needed. It consolidates the last two change transactions
	// into one if possible. This is useful for operations that change the same part repeatedly, i.e. TextureMove.
	void EndUndo(const AuId& architectId);

	virtual QString EntryPassword() const;

	void Faces(AuScFaceList& faces, const AuScSelection& sel) const;

	// Returns part id at 'pixel' by looking for an edge.
	// Returns 0 if none is found within search tolerance of 'pixel'.
	// 'snapLoc' returns the location on the part that was found.
	quint16 FindEdgeAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel, const AuScSelection& sel, bool objectMode, bool objectIsolate, AuPoint* snapLoc=0) const;

	virtual DaDesigner* FindParentDesignerInEnv();

	virtual const DaDesigner* FindParentDesignerInEnv() const;

	// Returns part id at 'pixel' by looking for vertices and edges.
	// Returns 0 if none is found within search tolerance of 'pixel'.
	quint16 FindPartAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel) const;

	// Returns part id at 'pixel' by looking for vertices, edges and faces.
	// Only looks at parts in 'partIds'.
	// Returns 0 if none is found within search tolerance of 'pixel'.
	quint16 FindPartObjectAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel, const AuScSelection& partIds) const;

	// For each part in partIds, fills 'partId's object ids with all vertices, edges and faces that are in the pixel rectangle.
	// Does not clear existing contents from 'partIds', only inserts new ids.
	void FindPartObjectsInArea(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QRectF& pixelRect, AuScSelection& partIds) const;

	// Fills 'partIds' with all parts that are in the pixel rectangle 'pixelRect', even partially.
	// Object ids are empty.
	// Does not clear existing contents from 'partIds', only inserts new ids.
	void FindPartsInArea(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QRectF& pixelRect, AuScSelection& partIds) const;

	// Returns part id at 'pixel' by looking for a vertex.
	// Returns 0 if none is found within search tolerance of 'pixel'.
	// 'snapLoc' returns the location on the part that was found.
	quint16 FindVertexAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel, const AuScSelection& sel, bool objectMode, bool objectIsolate, AuPoint* snapLoc=0) const;

	void FlattenVertices(const AuId& archId, const AuScSelection& sel);

	void FractureFaces(const AuId& archId, const AuScSelection& sel);

	void FuseFaces(const AuId& archId, const AuScSelection& sel);

	// Returns a seed value for generating preview contents.
	quint32 GenSeed() const;

	virtual void GetAtomEnvironment(AuQueryEnvironment& qe, const DaAtom* atom) const;

	void GlowVertices(const AuId& archId, const AuScSelection& sel, const QColor& glow);

	void GoToSelection(const AuId& archId, const AuScSelection& sel);

	virtual double Gravity(double cgDistance) const;

	int GridPosition(quint8 number, AuMatrix4x4& gridM) const;

	//void GridPositionLoad(const AuId& avatarId, quint8 number);

	//void GridPositionOrigin(const AuId& avatarId);

	// Returns ids of grid positions that are currently stored in the blueprint.
	QSet<quint8> GridPositions() const;

	void GridPositionSave(const AuId& avatarId, quint8 number, const AuMatrix4x4& gridM);

	// Returns true if the design contains any parts that were sealed by an architect EXCEPT 'architectId'.
	bool HasSealedParts(const AuId& architectId) const;

	// Returns true if the selection contains any parts that were sealed by an architect EXCEPT 'architectId'.
	bool HasSealedParts(const AuId& architectId, const AuScSelection& sel) const;

	void InvertPartObjectSelection(AuScSelection& sel) const;

	virtual bool IsDesignModified() const
	{
		return m_DesignModified;
	}

	// True if the design is opened with view-only terms. That means nothing can be copied to the clipboard and the design cannot
	// be saved anywhere.
	inline bool IsDesignViewOnly() const
	{
		return m_ViewOnly;
	}

	// 'hitNormal' is an output parameter. It is the normal to the surface or detail that is hit.
	// 'toWho' is the atom attempting to move so we can automatically open doors when appropriate.
	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho);

	// Returns true if we are currently previewing a design. No editing is permitted during preview.
	virtual bool IsPreview() const;

	COMMODITYID ItemCommodity() const;

	// Uses select jigs to cut unselected meshes.
	void JigCutParts(const AuId& archId, const AuScSelection& sel);

	// Joins select parts with 'partId', which may be selected or not.
	void JoinParts(const AuId& archId, quint16 partId, const AuScSelection& sel);

	// Retrieves the current spacecraft design record that is being modified by this design environment.
	// You must call unlock even if 0 is returned.
	const AuDesign* LockDesignForRead();

	// Const access to the parts of the design.
	// You must call unlock even if 0 is returned.
	const AuScParts* LockPartsForRead();

	// Attempts to find a loop around the faces of 'sel', starting at 'loc'.
	// Fills 'loop' with the result.
	// On failure, 'loop' will be empty.
	void Loop(const AuScSelection& sel, const AuPoint& loc, AuPointArray& loop) const;

	// Converts selected parts into something else.
	void MakeDecal(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	void MakeDoor(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	//void MakeHeadlightLens(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	void MakeHull(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	void MakeLandingGear(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	void MakeLightLens(const AuId& archId, const AuScSelection& sel, const QColor& lightColor);

	// Converts selected parts into something else.
	void MakeMesh(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	void MakeNavLightLens(const AuId& archId, const AuScSelection& sel, const QColor& color);

	// Converts selected parts into something else.
	void MakeRoom(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	void MakeRotatingBeaconLens(const AuId& archId, const AuScSelection& sel, const QColor& lightColor,
			const AuPoint& rotationAxis);

	// Converts selected parts into something else.
	void MakeStrobeLens(const AuId& archId, const AuScSelection& sel, const QColor& lightColor);

	// Converts selected parts into something else.
	void MakeTurboLift(const AuId& archId, const AuScSelection& sel);

	// Converts selected parts into something else.
	bool MakeTurret(const AuId& archId, const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis);

	// Converts selected parts into something else.
	void MakeWindow(const AuId& archId, const AuScSelection& sel, float opacity);

	virtual quint64 MemoryUsage() const;

	void MoveDoorSwitches(const AuId& archId, const AuScSelection& partIds, const AuPoint& switch1, const AuPoint& switch2);

	void MoveLiftStops(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t);

	void MoveParts(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t);

	void MovePartObjects(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t);

	virtual QString Name() const;

	virtual AuId Owner() const
	{
		m_EnvLock.lockForRead();
		AuId v(m_idOwner);
		m_EnvLock.unlock();
		return v;
	}

	// Alpha of 'paint' is the blend factor.
	void PaintFaces(const AuId& archId, const AuScSelection& sel, const QColor& paint);

	void PaintFaces(const AuId& archId, const AuScSelection& sel, AuScFace::COLOR_THEME theme, quint8 opacity);

	void PaintInUse(QList<QColor>& paintInUse) const;

	void PartAdd(const AuId& archId, const AuScPart& part);

	void PartAddFaces(const AuId& archId, quint16 partId, const AuScFaceList& faces);

	// Fills a boundary in object mode by adding a face to the first part in 'sel'.
	void PartFillFace(const AuId& archId, const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane);

	void PartFillQuad(const AuId& archId, const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
			const AuPoint& v4);

	void PartFillTriangle(const AuId& archId, const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3);

	// Gets a list of unique group names from the parts. List will include a blank entry if any part has no group.
	void PartGroupNames(QStringList& groupNames) const;

	void PartReplaceFaces(const AuId& archId, quint16 partId, const AuScSelection& sel, const AuScFaceList& faces);

	QString PartSelectedDesc(quint16 partId) const;

	// Retrieves the properties of the current door selection.
	void PartSelectionDoorProperties(const AuScSelection& partIds,
			Qt::CheckState& isAutomatic,
			Qt::CheckState& initLocked,
			Qt::CheckState& remoteSpacecraft,
			Qt::CheckState& remoteVehicle) const;

	// Fills 'partIds' with ids of all selectable parts.
	void PartSelectionIds(AuScSelection& partIds) const;

	// For each part in the selection, fills the edge list with all possible edge ids.
	void PartSelectionEdgeIds(AuScSelection& partIds) const;

	// For each part in the selection, fills the face list with all possible face ids.
	void PartSelectionFaceIds(AuScSelection& partIds) const;

	// Retrieves the properties of the current light bulb selection.
	void PartSelectionLightBulbProperties(const AuScSelection& partIds, QColor& color, float& kL, float& kQ, float& cosOuterCone,
			float& cosInnerCone) const;

	// Retrieves the properties of the current light lens selection.
	void PartSelectionLightLensProperties(const AuScSelection& partIds, QColor& color) const;

	// Retrieves the properties of the current door selection.
	void PartSelectionSiteProperties(const AuScSelection& partIds,
			quint8& grade,
			quint8& grass,
			quint8& pavement,
			quint8& rocks,
			quint8& shrubs,
            quint8& terBlend,
			quint8& trees) const;

	// For each part in the selection, fills the vertex list with all possible vertex ids.
	void PartSelectionVertexIds(AuScSelection& partIds) const;

	// Returns the current station name of a part.
	QString PartStop(quint16 partId) const;

	void PartStops(quint16 partId, QStringList& stations) const;

	void PartUpdate(const AuId& archId, quint16 partId, const AuScPart& part);

	void PolishFaces(const AuId& archId, const AuScSelection& sel, float shine, float reflectivity);

	void PunchFaces(const AuId& archId, const AuScSelection& sel, const AuPointArray& cutBnd);

	inline bool PreviewHasPower() const
	{
		return m_PreviewHasPower;
	}

	// Loads people in preview. Must already be in preview.
	void PreviewPeople(const AuSpecies& species);

	// Reads MIME encoded parts. Returns them in a parts list.
	void ReadMIME(const AuId& architectId, const QString& architectName, bool objectMode, const QMimeData& mimeData,
			AuScParts& parts) const;

	void Redo(const AuId& archId);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Rebuilds collision data.
	// Called only on the client at user's request.
	void RefreshObstructions();

	// Returns true if collision data changes.
	// Called only on the client at user's request.
	bool RefreshObstructionsIfNeeded();

	void RenamePartStop(const AuId& archId, quint16 partId, const QString& oldStopName, const QString& nuStopName);

	void ReverseFaces(const AuId& archId, const AuScSelection& sel);

	// Adds faces to the selection 'sel' that form a continuous surface in the same plane as faces that are already selected.
	// Depends on at least one face already being in the selection.
	// 'angleTol' is used to determine if two faces are planar enough.
	// Returns true if 'sel' is changed.
	bool SelectContiguous(double angleTol, AuScSelection& sel) const;

	// Returns true if 'sel' is changed.
	bool SelectPartObjectsAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel, AuScSelection& sel) const;

	// Returns true if 'sel' is changed.
	bool SelectPartTextureVertices(quint16 partId, quint32 partVertex, AuAtomRenderInfo& ari) const;

	inline void SetDesignModified(bool isModified=true)
	{
		m_DesignModified = isModified;
		SetModified();
	}

	inline void SetDesignViewOnly(bool viewOnly)
	{
		m_ViewOnly = viewOnly;
		SetModified();
	}

	void SetDoorProperties(const AuId& archId, const AuScSelection& partIds, Qt::CheckState isAutomatic, Qt::CheckState initLocked,
			Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle);

	void SetFaceOpacity(const AuId& archId, const AuScSelection& sel, float opacity);

	void SetFaceSelfIllumination(const AuId& archId, const AuScSelection& sel, float selfIllum);

	void SetLightBulbProperties(const AuId& archId, const AuScSelection& partIds, const QColor& color, float kL, float kQ,
			float cosOuterCone, float cosInnerCone);

	void SetLightLensProperties(const AuId& archId, const AuScSelection& partIds, const QColor& color);

	void SetPartOpacity(const AuId& archId, quint16 partId, float opacity);

	void SetPartsGroup(const AuId& archId, const AuScIdSet& partIds, const QString& groupName);

	void SetPartsHidden(const AuId& archId, const AuScIdSet& partIds, bool hide);

	void SetPartsLocked(const AuId& archId, const AuScIdSet& partIds, bool lock);

	void SetPartsOmitted(const AuId& archId, const AuScIdSet& partIds, bool omit);

	void SetPartsSealed(const AuId& archId, const AuScIdSet& partIds, bool seal);

	void SetPartState(const AuId& archId, quint16 partId, quint8 state);

	void SetPartStop(const AuId& archId, quint16 partId, const QString& station);

	void SetPartStopName(const AuId& archId, quint16 partId, const QString& name);

	// 'previewType' 0=none, 1=empty space, 2=sun, 3=sun and planet, 4=sun, planet and moon, 5=inferno, 6=frigid, 7=gas giant,
	//   8=ringworld, 9=sea bottom
	// 'design' and 'genSeed' are only needed when 'previewType' is not zero.
	void SetPreview(quint8 previewType, const AuDesign* design, quint32 genSeed);

	inline void SetPreviewHasPower(bool hasPower)
	{
		m_PreviewHasPower = hasPower;
	}

	void SetRoomProperties(const AuId& archId, const AuScSelection& partIds, bool isSunlit, INTERIOR_LIGHT initLighting);

	void SetSiteProperties(const AuId& archId, const AuScSelection& partIds, quint8 grade, quint8 grass, quint8 pavement,
			quint8 rocks, quint8 shrubs, quint8 terBlend, quint8 trees);

	// Texture palette contains AuTextureLibItem::Id's.
	void SetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count]);

	bool SetTurretBase(const AuId& archId, const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis);

	bool SetTurretGun(const AuId& archId, const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis,
			const AuPoint& gunEmitLoc, const AuPoint& gunEmitVec, const AuPoint& gunSightLoc);

	// 'gunnerPos' is a AuScTurret::GUNNER_POSITION
	bool SetTurretStation(const AuId& archId, const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc,
			const AuPoint& gunnerFacing, const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane,
			COMMODITYID module);

	void SetVoidProperties(const AuId& archId, const AuScSelection& partIds, GRAVITY gravityType, const AuPoint& gravityUp,
			HOLD_ACCESS holdAccess, quint8 personnelAccess);

	void SmoothVertices(const AuId& archId, const AuScSelection& sel);

	void SplitFaces(const AuId& archId, const AuScSelection& sel);

	// Returns a pointer to the undo/redo transaction that is started.
	AuDesignUndoRedoTransaction& StartUndo(const AuId& architectId, const QString& desc);

	void TextureFaces(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex);

	void TextureFacesCylindrical(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	void TextureFacesProject(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	void TextureFacesSpherical(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	// 'undoSeq' is used to distinguish one stream of texture moves from the next, for undo. It just has to be different from the
	// previous one so the undo transaction will not be absorbed into one single change undo.
	void TextureMove(const AuId& archId, const AuScSelection& sel, const QPointF& vec, quint8 undoSeq);

	// Retrieves the design's texture palette into 'texturePalette'.
	// Texture palette contains AuTextureLibItem::Id's.
	void TexturePalette(AuDesignTex texturePalette[SCTEXI_Count]) const;

	// 'undoSeq' is used to distinguish one stream of texture moves from the next, for undo. It just has to be different from the
	// previous one so the undo transaction will not be absorbed into one single change undo.
	void TextureRotate(const AuId& archId, const AuScSelection& sel, const QPointF& pivot, double angle, quint8 undoSeq);

	// 'undoSeq' is used to distinguish one stream of texture moves from the next, for undo. It just has to be different from the
	// previous one so the undo transaction will not be absorbed into one single change undo.
	void TextureStretch(const AuId& archId, const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY,
			quint8 undoSeq);

	// 'unwrap' is a AuScFaceList::UNWRAP_METHOD
	void TextureUnwrap(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize, quint8 unwrap,
			float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	void TogglePartObjectSelection(const AuMatrix4x4& projection, const AuMatrix4x4& modelView, const DaPerson* sceneAtom,
			AuScSelection& sel, const QRectF& pixelRect) const;

	void TransporterPadLocs(const AuId& archId, const AuScSelection& partIds, const AuPointArray& padLocs);

	void Undo(const AuId& architectId, const QDateTime& now);

	void UnlockDesign();

	void UnlockParts();

	// Returns a pointer to the current open undo/redo transaction.
	// CAVEAT: Must call StartUndo to initiate a new transaction.
	inline AuDesignUndoRedoTransaction& UndoTransaction(const AuId& architectId)
	{
		Q_ASSERT(m_UndoRedo.contains(architectId));				// Failed to call StartUndo.
		return m_UndoRedo[architectId].Last();
	}

	void UpdateHullDoors(const AuId& archId, const AuScSelection& partIds);

	void VisibleFaces(AuScFaceList& faces) const;

	// Returns visible faces from a particular group of the design. Used by fences in preview to get their Post and Section faces.
	void VisibleFaces(AuScFaceList& faces, const QString& groupName) const;

	void WeaponEmitLoc(const AuId& archId, const AuScSelection& partIds, const AuPoint& emitLoc);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, 
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);
	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	virtual void RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, DaPerson* sceneAtom);

	// Quickie transient rendering of the edges of 'faces', for cursors drawn by designer commands. See push/pull.
	void RenderEdges(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuScFaceList& faces) const;

	// Quickie transient rendering of 'faces', for cursors drawn by designer commands. See push/pull.
	// 'empireId' is used to select the color theme for theme painted parts.
	void RenderFaces(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuScFaceList& faces,
			quint32 empireId) const;

	void Undo(const AuId& archId);

	void ViewPositionGrid(const AuId& avatarId, const AuDesignPosition& pos);

	void ViewPositionLoad(const AuId& avatarId, quint8 number);

	void ViewPositionOrigin(const AuId& avatarId);

	// Returns ids of view positions that are currently stored in the blueprint.
	QSet<quint8> ViewPositions() const;

	void ViewPositionSave(const AuId& avatarId, quint8 number);

public:
	// The design id loaded/saved last.
	// Not the live design id of the ship or building being edited.
	virtual quint32 DbDesignId() const;

	virtual QString DesignArchitect() const
	{
		m_EnvLock.lockForRead();
		QString v(m_Architect);
		m_EnvLock.unlock();
		return v;
	}

	virtual QString DesignName() const;

	virtual short int GridOnBits(const AuId& idArchitect) const;

	virtual void InitDesignPlane(const AuId& idArchitect);

	int LoadBdDesign(quint32 designId, const AuId& idLoader, bool viewOnly);

	int LoadItDesign(quint32 designId, const AuId& idLoader, bool viewOnly);

	int LoadScDesign(quint32 designId, const AuId& idLoader, bool viewOnly);

	bool LoadedFromFile(const AuDesign& design);

	// Establishs owner of designer and moves the person to a good starting location.
	// idArchitect is a person atom id present in this designer.
	// 'design' provides the initial design properties.
	void NewDesign(const AuId& idArchitect, const QString& architect, const AuDesign& design);

	// 'idArchitect' is the id of the avatar who is saving the file. It will be passed to AuDesign::ClearArchitectForFileSave.
	// 'thumb' is optional, can be 0.
	bool SaveToFile(const AuId& idArchitect, const QString& fileName, const QImage* thumb) const;

	// Sets design properties except parts and textures.
	void SetProperties(const AuDesign& design);

	virtual bool IsGravityOn(const AuId& idArchitect) const;

	virtual bool IsRenderSolid(const AuId& idArchitect) const;

	void ResetPositions();

	void SetDesignPlane(const AuId& idArchitect, const AuPoint &fOrigin, const AuPoint &vX, const AuPoint &vY);

	virtual double GetDesignPlaneGridSpacing(const AuId& idArchitect) const;

	virtual AuMatrix4x4 GetDesignPlaneTransform(const AuId& idArchitect) const;

public:
	static DaDesigner* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

protected:
	void AnimateEffectTextures();

	// Returns a pointer to an array of effect textures that contains EFF_Count entries, one for each effect.
	const GLuint* EffectTextures() const;

	int LoadDesign(AuDesign* d, const AuId& idLoader, bool viewOnly);

	void RenderDesign(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			DaPerson* sceneAtom, bool transparentPass);

	void RenderDesignVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const QColor themeColors[AuScFace::THEME_Count], bool transparentPass, bool ownTextures);

	void RenderDesignVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const QColor themeColors[AuScFace::THEME_Count], bool transparentPass, bool ownTextures, bool isBack);

	void RenderDesignPaths(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			DaPerson* sceneAtom);

	void RenderDesignVoids(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			DaPerson* sceneAtom);

	void SetPartChangeTimes(const AuScParts& parts);

	void SetPartChangeTimesNoLock(const AuScParts& parts);
};

#endif // DaDesigner_H
