//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddplanclearglobe.h,v 1.6 2022/02/19 18:23:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdPlanClearGlobe_H
#define DdPlanClearGlobe_H

#include "auid.h"
#include "auglobell.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class AuGLEffects;

class DdPlanClearGlobe : public DdCommand  
{
private:
	AuId				m_BdId;
	int					m_ClearCount;
	int					m_ClearIndex;
	QCursor				m_Cursor;
	AuGlobeLLF			m_DevLL;
	AuGlobeLLF			m_GeoLL;

public:
	DdPlanClearGlobe(AuDesignerView* view);
	virtual ~DdPlanClearGlobe();

	// Mouse monitoring functions.
	virtual void OnButtonDown(QMouseEvent* e);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void OnMouseWheel(QWheelEvent*);

	// Cursor rendering functions.
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

private:
	void ClearLand() const;
	void NextIndex();
	void PrevIndex();
};

#endif // DdPlanClearGlobe_H
