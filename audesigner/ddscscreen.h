//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscscreen.h,v 1.6 2022/02/19 18:24:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScScreen_H
#define DdScScreen_H

#include "ddcommand.h"

class DdScScreen : public DdCommand  
{
private:
	AuPoint								m_Corner;				// First corner.
	int									m_GetPoint;				// 0=corner, 1=opposite corner
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::StaticType of the part being made.

public:
	DdScScreen(quint8 partType, AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	bool MakeIt(const AuPoint& corner);
	virtual void UpdatePrompt() const;
};

#endif // DdScScreen_H
