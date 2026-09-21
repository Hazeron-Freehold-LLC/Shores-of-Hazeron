//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddgrid3pt.h,v 1.6 2022/02/19 18:23:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdGrid3pt_H
#define DdGrid3pt_H

#include "ddcommand.h"

class DdGrid3pt : public DdCommand  
{
private:
	int									m_GetPoint;				// 0=origin, 1=X axis, 2=point in plane
	AuPoint								m_Origin;
	AuPoint								m_XAxis;

public:
	DdGrid3pt(AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual int RunLevel() const
	{
		return 1;
	}

	virtual QString WikiTopic() const;

protected:
	virtual void UpdatePrompt() const;
};

#endif // DdGrid3pt_H
