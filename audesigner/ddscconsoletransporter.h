//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscconsoletransporter.h,v 1.6 2022/02/19 18:23:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScConsoleTransporter_H
#define DdScConsoleTransporter_H

#include "dbcommodity.h"
#include "ddcommand.h"

class DdScConsoleTransporter : public DdCommand  
{
private:
	AuPoint								m_ConsoleLoc;
	AuMatrix4x4							m_ConsolePlane;
	int									m_GetPoint;
	COMMODITYID							m_Module;
	AuPointArray						m_PadLocs;
	float								m_Rot;
	int									m_WhichEffect;

public:
	DdScConsoleTransporter(AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	bool MakeIt();
	virtual void UpdatePrompt() const;
};

#endif // DdScConsoleTransporter_H
