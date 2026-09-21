//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmove.h,v 1.6 2022/02/19 18:24:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMove_H
#define DdScMove_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScMove : public DdCommand  
{
	bool								m_ConstrainX;
	bool								m_ConstrainY;
	bool								m_ConstrainZ;
	AuMatrix4x4							m_DragPlane;			// Plane in which the drag happens.

public:
	DdScMove(const AuPoint& ref, AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	virtual AuMatrix4x4 DesignPlane() const;

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void OnMouseMove(QMouseEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual void Start();

	virtual QString WikiTopic() const;

protected:
	virtual void UpdatePrompt() const;
};

#endif // DdScMove_H
