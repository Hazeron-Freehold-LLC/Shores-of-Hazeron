//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdbuildzone.h,v 1.4 2022/02/19 18:23:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdBdBuildZone_H
#define DdBdBuildZone_H

#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdBdBuildZone : public DdCommand  
{
private:
	AuPointArray						m_Points;
	bool								m_Round;
	int									m_Steps;

public:
	DdBdBuildZone(AuDesignerView* view);
	virtual ~DdBdBuildZone();

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
	bool MakeOutline(const AuPointArray& points);
	virtual void UpdatePrompt() const;
};

#endif // DdBdBuildZone_H
