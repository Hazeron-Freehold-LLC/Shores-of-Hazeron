//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscvoidprops.h,v 1.7 2022/02/19 18:24:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScVoidProperties_H
#define DdScVoidProperties_H

#include "audef.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScVoidProperties : public DdCommand  
{
private:
	QCursor								m_Cursor;
	GRAVITY								m_GravityType;
	HOLD_ACCESS							m_HoldAccess;
	quint8								m_Mode;
	quint8								m_PersonnelAccess;
	AuPoint								m_UpStart;

public:
	DdScVoidProperties(AuDesignerView* view);
	virtual ~DdScVoidProperties();

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
	bool SetupVoid(GRAVITY gravityType, const AuPoint& gravityN, HOLD_ACCESS holdAccess, quint8 personnelAccess);

	virtual void UpdatePrompt() const;
};

#endif // DdScVoidProperties_H
