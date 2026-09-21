//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsiteproperties.h,v 1.5 2022/02/19 18:24:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdSiteProperties_H
#define DdSiteProperties_H

#include "audef.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdSiteProperties : public DdCommand  
{
public:
	DdSiteProperties(AuDesignerView* view);
	virtual ~DdSiteProperties();

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
	bool SetProperties(quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs, quint8 terBlend, quint8 trees);

	virtual void UpdatePrompt() const;
};

#endif // DdSiteProperties_H
