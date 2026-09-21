//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscpunchface.h,v 1.7 2022/02/19 18:24:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScPunchFace_H
#define DdScPunchFace_H

#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScPunchFace : public DdCommand  
{
private:
	AuPointArray						m_Points;
	int									m_Steps;				// non-zero when round

public:
	DdScPunchFace(AuDesignerView* view);
	virtual ~DdScPunchFace();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual QString WikiTopic() const;

protected:
	bool DoCut();
	bool DoCut(const AuPoint& radPoint, int steps);

	virtual void UpdatePrompt() const;
};

#endif // DdScPunchFace_H
