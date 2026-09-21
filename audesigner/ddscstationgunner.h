//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscstationgunner.h,v 1.6 2022/02/19 18:24:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScStationGunner_H
#define DdScStationGunner_H

#include "dbcommodity.h"
#include "ddcommand.h"

class DdScStationGunner : public DdCommand  
{
private:
	AuPoint								m_Corner1;				// First corner.
	AuPoint								m_Corner2;				// Second corner.
	int									m_GetPoint;				// 0=corner, 1=opposite corner
	AuPoint								m_GunnerLoc;
	//AuScTurret::GUNNER_POSITION
	quint8								m_GunnerPos;
	COMMODITYID							m_Module;
	AuMatrix4x4							m_ScreenPlane;

public:
	DdScStationGunner(AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	bool MakeIt(const AuPoint& gunnerFacing);
	virtual void UpdatePrompt() const;
};

#endif // DdScStationGunner_H
