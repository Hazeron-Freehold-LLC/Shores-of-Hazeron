//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscdooraccess.h,v 1.9 2022/02/19 18:23:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScDoorAccess_H
#define DdScDoorAccess_H

#include "audef.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScDoorAccess : public DdCommand  
{
public:
	DdScDoorAccess(AuDesignerView* view);
	virtual ~DdScDoorAccess();

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
	bool SetupDoorAccess(Qt::CheckState isAutomatic, Qt::CheckState initLocked, Qt::CheckState remoteSpacecraft,
			Qt::CheckState remoteVehicle);

	virtual void UpdatePrompt() const;
};

#endif // DdScDoorAccess_H
