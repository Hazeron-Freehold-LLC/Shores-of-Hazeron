//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscwalkpath.h,v 1.7 2022/02/19 18:24:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScWalkPath_H
#define DdScWalkPath_H

#include "ddcommand.h"

class DdScWalkPath : public DdCommand  
{
private:
	AuPoint								m_FromLoc;
	quint8								m_Mode;

public:
	DdScWalkPath(AuDesignerView* view);
	virtual ~DdScWalkPath();

	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

	virtual void Start();

	virtual QString WikiTopic() const;

protected:
	bool MakePath(const AuPoint& toLoc);

	virtual void UpdatePrompt() const;
};

#endif // DdScWalkPath_H
