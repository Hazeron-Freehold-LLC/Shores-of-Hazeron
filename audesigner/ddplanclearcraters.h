//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanclearcraters.h,v 1.5 2022/02/19 18:23:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdPlanClearCraters_H
#define DdPlanClearCraters_H

#include "auid.h"
#include "auglobell.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class AuGLEffects;

class DdPlanClearCraters : public DdCommand  
{
private:
	int					m_ClearCount;
	int					m_ClearIndex;
	QCursor				m_Cursor;
	AuGlobeLLF			m_GeoLL;

public:
	DdPlanClearCraters(AuDesignerView* view);
	virtual ~DdPlanClearCraters();

	// Mouse monitoring functions.
	virtual void OnButtonDown(QMouseEvent* e);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void OnMouseWheel(QWheelEvent*);

	// Cursor rendering functions.
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

private:
	void ClearLand() const;
	void NextIndex();
	void PrevIndex();
};

#endif // DdPlanClearCraters_H
