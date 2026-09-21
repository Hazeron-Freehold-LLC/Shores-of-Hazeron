//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanbdfence.h,v 1.8 2022/04/15 21:02:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdPlanBdFence_H
#define DdPlanBdFence_H

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

class DdPlanBdFence : public DdCommand  
{
private:
	QCursor								m_Cursor;
	quint32								m_CustomDesignId;
	int									m_FenceStyle;			// AuScFence::STYLE
	AuElapsedTimer						m_LastTime;
	COMMODITYID							m_Material;				// Construction material for custom fences and walls.
	bool								m_ShiftDown;			// true while Shift key down
	QString								m_sReason;
	bool								m_StartEstablished;		// true once the start location has been established.
	AuPoint								m_StartLoc;				// Starting location of a road segment.

public:
	// 'fenceStyle' is an AuScFence::STYLE.
	// 'constructionMaterial' only applies to custom fences.
	DdPlanBdFence(int fenceStyle, quint32 customDesignId, COMMODITYID constructionMaterial, AuDesignerView* view);
	virtual ~DdPlanBdFence();

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

	virtual void SetConstructionProcess(const AuBdReport& report);

	// Set appropriate device cursor. Default sets Cross cursor if
	// pSceneAtom != 0, otherwise sets Arrow cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* sceneAtom, const QPoint& pxCursor);

	virtual void SetMaterial(int constructionMaterial);

protected:
	void ConstructFence(const QPoint& pxCursor);
	CONSTRUCTIONPROCESSID ConstructionProcessId() const;
	COMMODITYID FenceMaterial() const;
	int TestFenceLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
			DaBd*& exBd, DaBd** onPlatform, QString& cityName) const;
};

#endif // DdPlanBdFence_H
