//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscpasteradial.h,v 1.3 2022/02/19 18:24:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScPasteRadial_H
#define DdScPasteRadial_H

#include "ddcommand.h"
#include "aumatrix4x4.h"

class AuScParts;

class DdScPasteRadial : public DdCommand  
{
public:
	AuScParts*							m_PastedParts;
	AuMatrix4x4							m_Rot;

public:
	DdScPasteRadial(AuDesignerView* view);
	virtual ~DdScPasteRadial();

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	virtual bool OnKeyDown(QKeyEvent*);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	virtual void UpdatePrompt() const;
};

#endif // DdScPasteRadial_H
