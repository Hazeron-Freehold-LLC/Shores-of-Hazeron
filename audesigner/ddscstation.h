//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscstation.h,v 1.6 2022/02/19 18:24:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScStation_H
#define DdScStation_H

#include "dbcommodity.h"
#include "ddcommand.h"

class DdScStation : public DdCommand  
{
private:
	AuPoint								m_Corner1;				// First corner.
	AuPoint								m_Corner2;				// Second corner.
	int									m_GetPoint;				// 0=corner, 1=opposite corner
	COMMODITYID							m_Module;
	AuPoint								m_OperatorLoc;
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::StaticType of the part being made.
	//AuScStation::OPERATOR_POSITION
	quint8								m_Position;
	AuMatrix4x4							m_ScreenPlane;

public:
	DdScStation(quint8 partType, AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	bool MakeIt(const AuPoint& opFacing);
	virtual void UpdatePrompt() const;
};

#endif // DdScStation_H
