//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscalign.h,v 1.7 2022/02/19 18:23:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScAlign_H
#define DdScAlign_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScAlign : public DdCommand  
{
	quint8								m_Mode;
	AuMatrix4x4							m_StartM;

public:
	DdScAlign(AuDesignerView* view);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, DaPerson* pSceneAtom,
			const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	void DoMove();

	virtual void UpdatePrompt() const;
};

#endif // DdScAlign_H
