//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddpreviewfence.h,v 1.6 2022/04/15 21:03:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdPreviewFence_H
#define DdPreviewFence_H

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

class DdPreviewFence : public DdCommand  
{
private:
	QCursor								m_Cursor;
	AuElapsedTimer						m_LastTime;
	bool								m_ShiftDown;			// true while Shift key down
	QString								m_sReason;
	bool								m_StartEstablished;		// true once the start location has been established.
	AuPoint								m_StartLoc;				// Starting location of a road segment.

public:
	DdPreviewFence(AuDesignerView* view);
	virtual ~DdPreviewFence();

	// Keyboard monitoring functions.
	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyUp(QKeyEvent* e);

	// Mouse monitoring functions.
	virtual void OnButtonDown(QMouseEvent* e);

	// Cursor rendering functions.
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor. Default sets Cross cursor if
	// pSceneAtom != 0, otherwise sets Arrow cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* sceneAtom, const QPoint& pxCursor);

protected:
	void ConstructFence(const QPoint& pxCursor);
	int TestFenceLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
			DaBd*& exBd, DaBd** onPlatform, QString& cityName) const;
};

#endif // DdPreviewFence_H
