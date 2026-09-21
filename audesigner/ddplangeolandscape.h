//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplangeolandscape.h,v 1.11 2022/04/15 21:03:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdPlanGeoLandscape_H
#define DdPlanGeoLandscape_H

#include "auelapsedtimer.h"
#include "dbcommodity.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>

class AuGLEffects;
class DaBd;
class DaWorldLL;

class DdPlanGeoLandscape : public DdCommand  
{
private:
	bool								m_CtrlDown;				// true while Ctrl key down
	QCursor								m_Cursor;
	bool								m_CursorCtrlDown;
	AuElapsedTimer						m_LastTime;
	int									m_LandscapeType;		// 0=clearing, 1=defoliate, 2=irrigate
	bool								m_ShiftDown;			// true while Shift key down
	QString								m_sReason;
	bool								m_StartEstablished;		// true once the start location has been established.
	AuPoint								m_StartLoc;				// Starting location of a road segment.

public:
	DdPlanGeoLandscape(int type, AuDesignerView* view);
	virtual ~DdPlanGeoLandscape();

	// Keyboard monitoring functions.
	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyUp(QKeyEvent* e);

	// Mouse monitoring functions.
	virtual void OnButtonDown(QMouseEvent* e);

	virtual void OnMouseWheel(QWheelEvent*);

	// Cursor rendering functions.
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor. Default sets Cross cursor if
	// pSceneAtom != 0, otherwise sets Arrow cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

protected:
	int TestRoadLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
			DaBd*& exBd, QString& cityName) const;
	void ConstructRoad(const QPoint &pxCursor);
};

#endif // DdPlanGeoLandscape_H
