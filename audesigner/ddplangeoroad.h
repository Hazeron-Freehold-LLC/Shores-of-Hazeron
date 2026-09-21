//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplangeoroad.h,v 1.12 2022/04/15 21:03:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdPlanGeoRoad_H
#define DdPlanGeoRoad_H

#include "auelapsedtimer.h"
#include "dbcommodity.h"
#include "ddcommand.h"
#include "deconstructionprocess.h"
#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>

class AuGLEffects;
class AuGlobeLLF;
class DaBd;
class DaWorldLL;

class DdPlanGeoRoad : public DdCommand  
{
private:
	bool								m_CtrlDown;				// true while Ctrl key down
	QCursor								m_Cursor;
	bool								m_CursorCtrlDown;
	int									m_ElevationAdj;
	quint8								m_GeoType;
	AuElapsedTimer						m_LastTime;
	bool								m_ShiftDown;			// true while Shift key down
	QString								m_sReason;
	bool								m_StartEstablished;		// true once the start location has been established.
	AuPoint								m_StartLoc;				// Starting location of a road segment.

public:
	DdPlanGeoRoad(quint8 geoType, AuDesignerView* view);
	virtual ~DdPlanGeoRoad();

	// Keyboard monitoring functions.
	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyUp(QKeyEvent* e);

	// Mouse monitoring functions.
	virtual void OnButtonDown(QMouseEvent* e);

	virtual void OnMouseWheel(QWheelEvent*);

	// Cursor rendering functions.
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor. Default sets Cross cursor if
	// pSceneAtom != 0, otherwise sets Arrow cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

protected:
	void ChangeRoadConstruction(const QPoint &pxCursor);
	CONSTRUCTIONPROCESSID ConstructionProcessId() const;
	void ConstructRoad(const QPoint &pxCursor);
	void DrawArrow();
	bool FindRoadContaining(DaPerson* person, const QPoint& pxCursor, AuId& worldId, AuGlobeLLF& geoLL);
	int TestRoadLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
			DaBd*& exBd, QString& cityName) const;
};

#endif // DdPlanGeoRoad_H
