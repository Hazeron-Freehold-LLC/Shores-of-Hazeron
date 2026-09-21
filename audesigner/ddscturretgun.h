//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscturretgun.h,v 1.6 2022/02/19 18:24:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScTurretGun_H
#define DdScTurretGun_H

#include "ddcommand.h"

class DdScTurretGun : public DdCommand  
{
	AuPoint								m_AimLoc;
	int									m_GetPoint;
	AuPoint								m_GunAxis;
	AuPoint								m_GunEmitLoc;
	AuPoint								m_GunPivot;

public:
	DdScTurretGun(AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	bool DoIt(const AuPoint& sightLoc);
	virtual void UpdatePrompt() const;
};

#endif // DdScTurretGun_H
